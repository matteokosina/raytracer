#include "geometry/triangle.h"

#include <algorithm>
#include <cmath>

bool Triangle::intersect(const Ray& ray, double t_min, double t_max,
                         double& t_out, double& u_out, double& v_out,
                         TriangleIntersectionAlgorithm algorithm) const {
    const Vector3 e1 = b-a;
    const Vector3 e2 = c-a;
    if (algorithm == TriangleIntersectionAlgorithm::MollerTrumbore) {
        const Vector3 p = ray.direction.cross(e2);
        const double determinant = e1.dot(p);
        if (std::abs(determinant) <= 1e-12) return false;
        const double inverse = 1.0/determinant;
        const Vector3 offset = ray.origin-a;
        const double u = offset.dot(p)*inverse;
        if (u < 0.0 || u > 1.0) return false;
        const Vector3 q = offset.cross(e1);
        const double v = ray.direction.dot(q)*inverse;
        if (v < 0.0 || u+v > 1.0) return false;
        const double t = e2.dot(q)*inverse;
        if (t < t_min || t > t_max) return false;
        t_out = t;
        u_out = u;
        v_out = v;
        return true;
    }

    const Vector3 n = e1.cross(e2);
    const double n2 = n.length_squared();
    if (n2 <= 1e-24) return false;

    const double denominator = n.dot(ray.direction);
    if (std::abs(denominator) <= 1e-12) return false;
    const double t = n.dot(a-ray.origin)/denominator;
    if (t < t_min || t > t_max) return false;

    const Vector3 point = ray.at(t);
    const int dropped_axis = std::abs(n.x) > std::abs(n.y)
        ? (std::abs(n.x) > std::abs(n.z) ? 0 : 2)
        : (std::abs(n.y) > std::abs(n.z) ? 1 : 2);
    auto coordinates = [dropped_axis](const Vector3& p) {
        if (dropped_axis == 0) return std::pair{p.y,p.z};
        if (dropped_axis == 1) return std::pair{p.x,p.z};
        return std::pair{p.x,p.y};
    };
    const auto [ax,ay] = coordinates(a);
    const auto [bx,by] = coordinates(b);
    const auto [cx,cy] = coordinates(c);
    const auto [px,py] = coordinates(point);
    const double area = (bx-ax)*(cy-ay)-(by-ay)*(cx-ax);
    if (std::abs(area) <= 1e-18) return false;
    const double u = ((px-ax)*(cy-ay)-(py-ay)*(cx-ax))/area;
    const double v = ((bx-ax)*(py-ay)-(by-ay)*(px-ax))/area;
    if (u < 0.0 || v < 0.0 || u+v > 1.0) return false;

    t_out = t;
    u_out = u;
    v_out = v;
    return true;
}
