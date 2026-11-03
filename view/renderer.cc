#include "view/renderer.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr int kMaxReflectionDepth = 4;
constexpr double kRayOffset = 1e-4;

Vector3 background(const Ray& ray) {
    const double blend = 0.5*(ray.direction.normalized().y+1.0);
    const Vector3 horizon{0.025,0.032,0.048};
    const Vector3 sky{0.16,0.23,0.33};
    return horizon*(1.0-blend)+sky*blend;
}

Vector3 offset_origin(const Vector3& point, const Vector3& geometric_normal,
                      const Vector3& outgoing) {
    const double side = outgoing.dot(geometric_normal) >= 0.0 ? 1.0 : -1.0;
    return point+geometric_normal*(side*kRayOffset);
}

double schlick_fresnel(double cosine, double refractive_index) {
    const double ratio = (refractive_index-1.0)/(refractive_index+1.0);
    const double f0 = ratio*ratio;
    return f0+(1.0-f0)*std::pow(1.0-std::clamp(cosine,0.0,1.0),5.0);
}

Vector3 light_visibility(const World& world, const Vector3& point,
                        const Vector3& geometric_normal, const Vector3& light,
                        TriangleIntersectFn intersect_fn, RenderStats* stats) {
    // Transparente Hindernisse werden nacheinander entlang des Schattenstrahls gesammelt.
    // Auch diese Abfragen nutzen denselben vorgewählten Dreieckstest wie die Kamerastrahlen.
    const Ray shadow_ray{offset_origin(point,geometric_normal,light),light};
    Vector3 visibility{1.0,1.0,1.0};
    double t_min = 1e-6;
    for (int layer=0;layer<16;++layer) {
        Hit blocker;
        if (!world.intersect(shadow_ray,t_min,1e30,blocker,intersect_fn,stats)) break;
        const double opacity = std::clamp(world.triangles()[blocker.triangle_index].material.opacity,0.0,1.0);
        visibility = visibility*(1.0-opacity);
        if (visibility.x<0.01 && visibility.y<0.01 && visibility.z<0.01) return {};
        t_min = blocker.t+kRayOffset;
    }
    return visibility;
}

bool refract(const Vector3& incoming, const Vector3& normal, double eta,
             Vector3& transmitted) {
    const double cosine = std::min(-incoming.dot(normal),1.0);
    const Vector3 perpendicular = eta*(incoming+normal*cosine);
    const double parallel_squared = 1.0-perpendicular.length_squared();
    if (parallel_squared < 0.0) return false;
    transmitted = (perpendicular-normal*std::sqrt(parallel_squared)).normalized();
    return true;
}

Vector3 trace(const World& world, const Ray& ray, int depth, RenderStats* stats,
              TriangleIntersectFn intersect_fn) {
    Hit hit;
    // Ein Strahl ohne Treffer trägt nur die Hintergrundfarbe zum Pixel bei.
    // intersect_fn wird unverändert durch Reflexions- und Brechungsstrahlen weitergereicht.
    if (!world.intersect(ray,1e-6,1e30,hit,intersect_fn,stats)) return background(ray);

    const Triangle& triangle = world.triangles()[hit.triangle_index];
    const Material& material = triangle.material;
    const Vector3 point = ray.at(hit.t);
    const Vector3 geometric_normal = triangle.normal();
    const bool entering = geometric_normal.dot(ray.direction)<0.0;
    const Vector3 interface_normal = entering ? geometric_normal : -geometric_normal;
    Vector3 normal = triangle.shading_normal(hit.u,hit.v);
    if (normal.dot(geometric_normal)<0.0) normal = -normal;
    if (normal.dot(ray.direction)>0.0) normal = -normal;

    const Vector3 light = Vector3{-0.5,0.8,0.6}.normalized();
    const Vector3 visibility = light_visibility(world,point,geometric_normal,light,intersect_fn,stats);
    const Vector3 view = (-ray.direction).normalized();
    const double diffuse = std::max(0.0,normal.dot(light));
    const Vector3 half_vector = (light+view).normalized();
    const double highlight = std::pow(std::max(0.0,normal.dot(half_vector)),
                                      std::max(1.0,material.shininess));
    const Vector3 local_color = material.diffuse*0.12
                              + (material.diffuse*(0.86*diffuse)).hadamard(visibility)
                              + (material.specular*highlight).hadamard(visibility);

    // Fresnel-Werte teilen die Materialfarbe auf lokale Farbe, Reflexion und Brechung auf.
    const double opacity = std::clamp(material.opacity,0.0,1.0);
    const double specular_level = std::clamp(std::max({material.specular.x,
        material.specular.y,material.specular.z}),0.0,0.95);
    const double eta = std::clamp(material.refractive_index,1.0,3.0);
    const double cosine = std::clamp(-ray.direction.normalized().dot(interface_normal),0.0,1.0);
    const double fresnel = schlick_fresnel(cosine,entering ? eta : 1.0/eta);
    double reflection_weight = opacity > 0.999
        ? specular_level
        : specular_level+(1.0-specular_level)*fresnel;
    double transmission_weight = (1.0-opacity)*(1.0-reflection_weight);
    const double local_weight = opacity*(1.0-reflection_weight);

    // Begrenze Reflexion und Brechung, damit die Rekursion endlich bleibt.
    if (depth >= kMaxReflectionDepth ||
        (reflection_weight < 0.001 && transmission_weight < 0.001))
        return local_color*local_weight + background(ray)*
               (reflection_weight+transmission_weight);

    const Vector3 incoming = ray.direction.normalized();
    const Vector3 reflected = (incoming-normal*(2.0*incoming.dot(normal))).normalized();
    const Ray reflection_ray{offset_origin(point,geometric_normal,reflected),reflected};
    // Jeder Folgestrahl fragt dieselbe Szene mit demselben Schnittverfahren ab.
    const Vector3 reflected_color = trace(world,reflection_ray,depth+1,stats,intersect_fn);
    Vector3 color = local_color*local_weight + reflected_color*reflection_weight;

    if (transmission_weight > 0.001) {
        const double relative_eta = entering ? 1.0/eta : eta;
        Vector3 transmitted;
        if (refract(incoming,interface_normal,relative_eta,transmitted)) {
            const Ray transmission_ray{offset_origin(point,geometric_normal,transmitted),transmitted};
            color = color+trace(world,transmission_ray,depth+1,stats,intersect_fn)*transmission_weight;
        } else {
            color = color+reflected_color*transmission_weight;
        }
    }
    return color;
}
}

std::vector<Vector3> Renderer::render(const World& world, const Camera& camera,
                                      int width, int height, RenderStats* stats,
                                      TriangleIntersectFn intersect_fn,
                                      ProgressCallback progress) {
    // Der Algorithmus steht vor dem Eintritt in die Pixel- und Dreiecksschleifen fest.
    std::vector<Vector3> pixels(static_cast<std::size_t>(width)*height);
    int last_percent = -1;
    // Pro Pixel wird ein Strahl durch die Pixelmitte verfolgt.
    for (int y=0;y<height;++y) {
        for (int x=0;x<width;++x) {
            if (stats) ++stats->primary_rays;
            const double u = (x+0.5)/width;
            const double v = 1.0-(y+0.5)/height;
            pixels[static_cast<std::size_t>(y)*width+x] = trace(world,camera.ray(u,v),0,stats,intersect_fn);
        }
        const int percent = (100*(y+1))/height;
        if (progress && percent != last_percent) {
            progress(percent);
            last_percent = percent;
        }
    }
    return pixels;
}
