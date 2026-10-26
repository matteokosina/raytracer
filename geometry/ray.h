#pragma once

#include "math/vector3.h"

/** A half-line parameterized as origin + t * direction. */
struct Ray {
    Vector3 origin;
    Vector3 direction;

    /** Returns the point at parameter t (world units when direction is unit length). */
    [[nodiscard]] constexpr Vector3 at(double t) const { return origin + direction*t; }
};
