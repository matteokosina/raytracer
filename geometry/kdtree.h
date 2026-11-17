#pragma once

#include "geometry/aabb.h"
#include "geometry/hit.h"
#include "utility/stats.h"

#include <memory>
#include <vector>

/** Median centroid partition; child bounds may overlap across the split plane. */
struct KDNode {
    AABB bbox; ///< Bounds of all descendant triangles in world units.
    std::unique_ptr<KDNode> left; ///< Lower-centroid half, or null in a leaf.
    std::unique_ptr<KDNode> right; ///< Upper-centroid half, or null in a leaf.
    std::vector<std::size_t> triangle_indices; ///< Original triangle indices; leaves only.
    int split_axis = 0; ///< Dimensionless axis index: 0=X, 1=Y, 2=Z.
    double split_value = 0.0; ///< Median centroid coordinate in world units.

    /** @return True if both child pointers are null. Does not modify the node. */
    [[nodiscard]] bool is_leaf() const { return !left && !right; }
};

/** Immutable, move-safe triangle snapshot with a balanced median-split index. */
class KDTree {
public:
    /** Builds once, retaining original triangle ordering for hit identities.
     * @param triangles Finite world-space triangles; empty input is allowed.
     * @param max_leaf_triangles Maximum primitives per leaf (count, at least 1).
     * @param max_depth Maximum subdivision depth (levels, at least 0).
     * @throws std::invalid_argument For invalid leaf size or depth.
     * @post Owns its geometry; caller mutations and moves cannot invalidate it.
     */
    explicit KDTree(const std::vector<Triangle>& triangles,
                    std::size_t max_leaf_triangles = 6, int max_depth = 20);

    /** Finds the closest intersection using box pruning and near-first traversal.
     * @param ray World-space ray with finite components; need not be normalized.
     * @param t_min Lower ray parameter (world units for unit-length direction).
     * @param t_max Upper ray parameter, in the same units as t_min.
     * @param rec Receives t, dimensionless u/v and original triangle index on success.
     * @param intersect_fn Non-null triangle algorithm selected before rendering.
     * @param stats Optional atomic counters, incremented once per actual test.
     * @return True if a triangle intersects the closed interval [t_min,t_max].
     * @post On a miss rec is unchanged; ties choose the largest original index,
     * matching brute force. Geometry is unchanged; concurrent queries are safe.
     */
    [[nodiscard]] bool hit(const Ray& ray, double t_min, double t_max, Hit& rec,
                           TriangleIntersectFn intersect_fn = &Triangle::intersect_badouel,
                           RenderStats* stats = nullptr) const;

    /** @return Read-only root, or null for an empty tree; valid for this tree's lifetime. */
    [[nodiscard]] const KDNode* root() const { return root_.get(); }

private:
    /** Recursively partitions original indices along depth modulo three.
     * @param indices Nonempty original triangle indices, reordered locally.
     * @param depth Current subdivision level (count).
     * @return Owned subtree enclosing all supplied triangles.
     */
    [[nodiscard]] std::unique_ptr<KDNode> build(std::vector<std::size_t> indices, int depth);

    /** Visits a subtree and tightens the current closest-hit interval.
     * @param node Non-null subtree whose bounds are tested first.
     * @param ray World-space query ray.
     * @param t_min Lower accepted ray parameter.
     * @param closest In/out upper parameter; units match ray parameter t.
     * @param rec In/out closest hit with dimensionless barycentric coordinates.
     * @param found In/out flag indicating a hit anywhere in this query.
     * @param intersect_fn Non-null triangle intersection method.
     * @param stats Optional atomic counters for all visited boxes and triangles.
     * @post Misses preserve the current hit; split planes order but never prune children.
     */
    void traverse(const KDNode& node, const Ray& ray, double t_min, double& closest,
                  Hit& rec, bool& found, TriangleIntersectFn intersect_fn, RenderStats* stats) const;

    std::vector<Triangle> triangles_;
    std::vector<Vector3> centroids_;
    std::size_t max_leaf_triangles_;
    int max_depth_;
    std::unique_ptr<KDNode> root_;
};
