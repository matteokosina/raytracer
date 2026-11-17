#pragma once

#include "geometry/kdtree.h"

#include <cstddef>
#include <vector>

/** Triangle scene with baseline and optional immutable spatial acceleration. */
class World {
public:
    /** Adds a primitive and invalidates any previously built acceleration tree.
     * @param triangle Finite world-space triangle, copied into scene storage.
     * @post Original indices remain stable; subsequent queries use brute force
     * until build_kdtree() is called again. Do not mutate during rendering.
     */
    void add_triangle(const Triangle& triangle);

    /** Builds/replaces the tree after scene loading, before rendering.
     * @param max_leaf_triangles Maximum triangle count per leaf, at least 1.
     * @param max_depth Maximum subdivision depth in levels, at least 0.
     * @throws std::invalid_argument For invalid construction limits.
     * @post Acceleration is enabled even for an empty scene.
     */
    void build_kdtree(std::size_t max_leaf_triangles = 6, int max_depth = 20);

    /** @return True if a current tree is available. Does not change the scene. */
    [[nodiscard]] bool kdtree_enabled() const { return static_cast<bool>(kdtree_); }

    /** Convenience query using the available traversal; renderer uses direct methods.
     * @param ray Finite world-space ray; direction need not be normalized.
     * @param t_min Lower ray parameter (world units for unit-length direction).
     * @param t_max Upper ray parameter with the same units as t_min.
     * @param hit Receives t, dimensionless u/v and original triangle index on success.
     * @param intersect_fn Non-null triangle method selected before rendering.
     * @param stats Optional atomic counters, incremented once per actual test.
     * @return True for a closest hit in the closed interval [t_min,t_max].
     * @post On a miss hit is unchanged; ties choose the largest triangle index.
     * Scene is unchanged; concurrent queries are safe.
     */
    [[nodiscard]] bool intersect(const Ray& ray, double t_min, double t_max,
                                 Hit& hit, TriangleIntersectFn intersect_fn = &Triangle::intersect_badouel,
                                 RenderStats* stats = nullptr) const;

    /** Direct baseline query with the same parameters, units and result as intersect().
     * @copydetails intersect
     * @post No traversal-mode branch is evaluated; the tree is never used.
     */
    [[nodiscard]] bool intersect_bruteforce(const Ray& ray, double t_min, double t_max,
                                            Hit& hit, TriangleIntersectFn intersect_fn = &Triangle::intersect_badouel,
                                            RenderStats* stats = nullptr) const;

    /** Direct accelerated query with the same parameters and units as intersect().
     * @copydetails intersect
     * @pre build_kdtree() has been called after the last scene mutation.
     * @post No traversal-mode branch is evaluated.
     */
    [[nodiscard]] bool intersect_kdtree(const Ray& ray, double t_min, double t_max,
                                        Hit& hit, TriangleIntersectFn intersect_fn = &Triangle::intersect_badouel,
                                        RenderStats* stats = nullptr) const;

    /** @return Read-only scene triangles in original order; valid until scene mutation. */
    [[nodiscard]] const std::vector<Triangle>& triangles() const { return triangles_; }
    /** @return Component-wise minimum in world units, or positive infinity if empty. */
    [[nodiscard]] Vector3 bounds_min() const;
    /** @return Component-wise maximum in world units, or negative infinity if empty. */
    [[nodiscard]] Vector3 bounds_max() const;
private:
    std::vector<Triangle> triangles_;
    std::unique_ptr<KDTree> kdtree_;
};

/** Scene traversal method bound at compile time by the rendering implementation. */
using WorldIntersectFn = bool (World::*)(const Ray&, double, double, Hit&,
                                        TriangleIntersectFn, RenderStats*) const;
