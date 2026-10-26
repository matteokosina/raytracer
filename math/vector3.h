#pragma once

#include <cmath>

/** A three-dimensional vector with double precision components. */
struct Vector3 {
    double x = 0.0; ///< X coordinate in world units.
    double y = 0.0; ///< Y coordinate in world units.
    double z = 0.0; ///< Z coordinate in world units.

    /** Returns the squared Euclidean length in squared world units. */
    [[nodiscard]] constexpr double length_squared() const { return x*x + y*y + z*z; }
    /** Returns the Euclidean length in world units. */
    [[nodiscard]] double length() const { return std::sqrt(length_squared()); }
    /** Returns this vector normalized; a zero vector remains zero. */
    [[nodiscard]] Vector3 normalized() const {
        const double magnitude = length();
        return magnitude > 0.0 ? Vector3{x/magnitude, y/magnitude, z/magnitude} : Vector3{};
    }
    /** Returns the dot product of two vectors. */
    [[nodiscard]] constexpr double dot(const Vector3& rhs) const {
        return x*rhs.x + y*rhs.y + z*rhs.z;
    }
    /** Returns the right-handed cross product of two vectors. */
    [[nodiscard]] constexpr Vector3 cross(const Vector3& rhs) const {
        return {y*rhs.z-z*rhs.y, z*rhs.x-x*rhs.z, x*rhs.y-y*rhs.x};
    }
    /** Adds component-wise to another vector. */
    [[nodiscard]] constexpr Vector3 operator+(const Vector3& rhs) const { return {x+rhs.x,y+rhs.y,z+rhs.z}; }
    /** Subtracts another vector component-wise. */
    [[nodiscard]] constexpr Vector3 operator-(const Vector3& rhs) const { return {x-rhs.x,y-rhs.y,z-rhs.z}; }
    /** Negates all components. */
    [[nodiscard]] constexpr Vector3 operator-() const { return {-x,-y,-z}; }
    /** Scales all components by a dimensionless factor. */
    [[nodiscard]] constexpr Vector3 operator*(double s) const { return {x*s,y*s,z*s}; }
    /** Divides all components by a non-zero scalar. */
    [[nodiscard]] constexpr Vector3 operator/(double s) const { return {x/s,y/s,z/s}; }
    /** Multiplies corresponding components. */
    [[nodiscard]] constexpr Vector3 hadamard(const Vector3& rhs) const { return {x*rhs.x,y*rhs.y,z*rhs.z}; }
};

/** Scales a vector by a scalar from the left. */
[[nodiscard]] constexpr Vector3 operator*(double s, const Vector3& v) { return v*s; }
