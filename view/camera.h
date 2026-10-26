#pragma once

#include "geometry/ray.h"

/** Pinhole camera aimed at a target point. */
class Camera {
public:
    /** Builds a camera framing the supplied bounds at the given aspect ratio. */
    Camera(const Vector3& bounds_min, const Vector3& bounds_max, double aspect_ratio);
    /** Creates a primary ray through normalized pixel coordinates [0,1]. */
    [[nodiscard]] Ray ray(double u, double v) const;
private:
    Vector3 origin_;
    Vector3 lower_left_;
    Vector3 horizontal_;
    Vector3 vertical_;
};
