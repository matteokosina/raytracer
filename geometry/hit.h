#pragma once

#include <cstddef>

/** Closest intersection data; unchanged when a query misses. */
struct Hit {
    double t = 0.0; ///< Ray parameter (world units for a unit direction).
    double u = 0.0; ///< Dimensionless barycentric weight of vertex b.
    double v = 0.0; ///< Dimensionless barycentric weight of vertex c.
    std::size_t triangle_index = 0; ///< Index in the original scene triangle array.
};
