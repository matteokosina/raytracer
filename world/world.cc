#include "world/world.h"

#include <algorithm>
#include <limits>

void World::add_triangle(const Triangle& triangle) { triangles_.push_back(triangle); }

bool World::intersect(const Ray& ray, double t_min, double t_max, Hit& hit, RenderStats* stats,
                      TriangleIntersectionAlgorithm algorithm) const {
    bool found = false;
    double closest = t_max;
    for (std::size_t i = 0; i < triangles_.size(); ++i) {
        if (stats) ++stats->triangle_tests;
        double t = 0.0, u = 0.0, v = 0.0;
        if (triangles_[i].intersect(ray, t_min, closest, t, u, v, algorithm)) {
            found = true;
            closest = t;
            hit = {t, u, v, i};
        }
    }
    return found;
}

Vector3 World::bounds_min() const {
    Vector3 result{std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};
    for (const auto& tri : triangles_) for (const auto& p : {tri.a, tri.b, tri.c}) {
        result.x = std::min(result.x,p.x); result.y = std::min(result.y,p.y); result.z = std::min(result.z,p.z);
    }
    return result;
}

Vector3 World::bounds_max() const {
    Vector3 result{-std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()};
    for (const auto& tri : triangles_) for (const auto& p : {tri.a, tri.b, tri.c}) {
        result.x = std::max(result.x,p.x); result.y = std::max(result.y,p.y); result.z = std::max(result.z,p.z);
    }
    return result;
}
