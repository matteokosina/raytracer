#include "geometry/aabb.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

bool AABB::intersect(const Ray& ray, double t_min, double t_max) const {
    if (std::isnan(t_min) || std::isnan(t_max) || t_min > t_max) return false;
    const double origins[] = {ray.origin.x, ray.origin.y, ray.origin.z};
    const double directions[] = {ray.direction.x, ray.direction.y, ray.direction.z};
    const double lower[] = {min_bounds.x, min_bounds.y, min_bounds.z};
    const double upper[] = {max_bounds.x, max_bounds.y, max_bounds.z};
    const double infinity = std::numeric_limits<double>::infinity();
    for (int axis = 0; axis < 3; ++axis) {
        const double origin = origins[axis];
        const double direction = directions[axis];
        if (!std::isfinite(origin) || !std::isfinite(direction)) return false;
        if (direction == 0.0) {
            if (origin < lower[axis] || origin > upper[axis]) return false;
            continue;
        }
        const double inverse = 1.0 / direction;
        double near = std::isfinite(inverse) ? (lower[axis] - origin) * inverse
                                            : (lower[axis] - origin) / direction;
        double far = std::isfinite(inverse) ? (upper[axis] - origin) * inverse
                                           : (upper[axis] - origin) / direction;
        if (near > far) std::swap(near, far);
        t_min = std::max(t_min, std::nextafter(near, -infinity));
        t_max = std::min(t_max, std::nextafter(far, infinity));
        if (t_min > t_max) return false;
    }
    return true;
}

AABB AABB::from_triangle(const Triangle& tri) {
    return {{std::min({tri.a.x, tri.b.x, tri.c.x}),
             std::min({tri.a.y, tri.b.y, tri.c.y}),
             std::min({tri.a.z, tri.b.z, tri.c.z})},
            {std::max({tri.a.x, tri.b.x, tri.c.x}),
             std::max({tri.a.y, tri.b.y, tri.c.y}),
             std::max({tri.a.z, tri.b.z, tri.c.z})}};
}

AABB AABB::surround(const AABB& box1, const AABB& box2) {
    return {{std::min(box1.min_bounds.x, box2.min_bounds.x),
             std::min(box1.min_bounds.y, box2.min_bounds.y),
             std::min(box1.min_bounds.z, box2.min_bounds.z)},
            {std::max(box1.max_bounds.x, box2.max_bounds.x),
             std::max(box1.max_bounds.y, box2.max_bounds.y),
             std::max(box1.max_bounds.z, box2.max_bounds.z)}};
}
