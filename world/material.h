#pragma once

#include "math/vector3.h"

/** Basic diffuse, specular, and opacity parameters loaded from an MTL file. */
struct Material {
    Vector3 diffuse{0.8,0.8,0.8}; ///< Diffuse reflectance in linear RGB [0,1].
    Vector3 specular{0.0,0.0,0.0}; ///< Specular reflectance in linear RGB [0,1].
    double shininess = 16.0; ///< Blinn-Phong exponent; larger values make tighter highlights.
    double opacity = 1.0; ///< Surface opacity where 1 is opaque and 0 is transparent.
    double refractive_index = 1.5; ///< Index of refraction used for dielectric transmission.
};
