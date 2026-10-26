#include "view/camera.h"

#include <algorithm>

Camera::Camera(const Vector3& bounds_min, const Vector3& bounds_max, double aspect_ratio) {
    const Vector3 center = (bounds_min+bounds_max)*0.5;
    const Vector3 extent = bounds_max-bounds_min;
    const double radius = std::max({extent.x,extent.y,extent.z,1e-3})*0.5;
    origin_ = center + Vector3{0.0, radius*1.05, -radius*3.2};
    const Vector3 forward = (center-origin_).normalized();
    const Vector3 right = forward.cross({0.0,1.0,0.0}).normalized();
    const Vector3 up = right.cross(forward).normalized();
    const double half_height = radius*0.62;
    const double half_width = half_height*aspect_ratio;
    horizontal_ = right*(2.0*half_width);
    vertical_ = up*(2.0*half_height);
    lower_left_ = origin_ + forward*(radius*2.5) - horizontal_*0.5 - vertical_*0.56;
}

Ray Camera::ray(double u, double v) const { return {origin_, lower_left_+horizontal_*u+vertical_*v-origin_}; }
