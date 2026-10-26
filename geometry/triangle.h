#pragma once

#include "geometry/ray.h"
#include "world/material.h"

/** Available scalar ray-triangle intersection methods. */
enum class TriangleIntersectionAlgorithm {
    Badouel, ///< Plane intersection followed by dominant-axis 2D projection.
    MollerTrumbore ///< Direct 3D barycentric intersection.
};

/** A triangle primitive stored by value in a scene. */
struct Triangle {
    Vector3 a;
    Vector3 b;
    Vector3 c;
    Material material;
    Vector3 normal_a;
    Vector3 normal_b;
    Vector3 normal_c;
    bool has_vertex_normals = false;

    /** Intersects a ray using the selected method; Badouel is the default.
     * @param ray Ray in world coordinates; direction need not be normalized.
     * @param t_min Lower accepted ray parameter.
     * @param t_max Upper accepted ray parameter.
     * @param t_out Receives the hit parameter when an intersection exists.
     * @param u_out Receives the first barycentric coordinate on success.
     * @param v_out Receives the second barycentric coordinate on success.
     * @param algorithm Intersection method used for this query.
     * @return True when the ray hits the triangle within [t_min, t_max].
     */
    [[nodiscard]] bool intersect(const Ray& ray, double t_min, double t_max,
                                 double& t_out, double& u_out, double& v_out,
                                 TriangleIntersectionAlgorithm algorithm = TriangleIntersectionAlgorithm::Badouel) const;
    /** Returns the unit geometric normal; degenerate triangles return zero. */
    [[nodiscard]] Vector3 normal() const { return (b-a).cross(c-a).normalized(); }
    /** Interpolates OBJ vertex normals with barycentric weights and normalizes the result. */
    [[nodiscard]] Vector3 shading_normal(double u, double v) const {
        if (!has_vertex_normals) return normal();
        return (normal_a*(1.0-u-v)+normal_b*u+normal_c*v).normalized();
    }
};
