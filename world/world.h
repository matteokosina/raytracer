#pragma once

#include "geometry/triangle.h"
#include "utility/stats.h"

#include <cstddef>
#include <vector>

/** Closest intersection data returned by a scene query. */
struct Hit {
    double t = 0.0;
    double u = 0.0;
    double v = 0.0;
    std::size_t triangle_index = 0;
};

/** Triangle scene with a brute-force closest-hit query. */
class World {
public:
    /** Adds one triangle to the scene. */
    void add_triangle(const Triangle& triangle);
    /** Returns true and fills hit with the closest triangle in the interval. */
    [[nodiscard]] bool intersect(const Ray& ray, double t_min, double t_max,
                                 Hit& hit, RenderStats* stats = nullptr,
                                 TriangleIntersectionAlgorithm algorithm = TriangleIntersectionAlgorithm::Badouel) const;
    /** Provides read-only access to all scene triangles. */
    [[nodiscard]] const std::vector<Triangle>& triangles() const { return triangles_; }
    /** Returns the component-wise minimum scene coordinate. */
    [[nodiscard]] Vector3 bounds_min() const;
    /** Returns the component-wise maximum scene coordinate. */
    [[nodiscard]] Vector3 bounds_max() const;
private:
    std::vector<Triangle> triangles_;
};
