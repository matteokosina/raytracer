#include "geometry/kdtree.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace {
double coordinate(const Vector3& point, int axis) {
    return axis == 0 ? point.x : (axis == 1 ? point.y : point.z);
}
}

KDTree::KDTree(const std::vector<Triangle>& triangles, std::size_t max_leaf_triangles,
               int max_depth)
    : triangles_(triangles), max_leaf_triangles_(max_leaf_triangles), max_depth_(max_depth) {
    if (max_leaf_triangles == 0 || max_depth < 0)
        throw std::invalid_argument("KDTree requires a positive leaf size and nonnegative depth");
    centroids_.reserve(triangles_.size());
    for (const auto& tri : triangles_) centroids_.push_back(tri.a / 3.0 + tri.b / 3.0 + tri.c / 3.0);
    if (!triangles_.empty()) {
        std::vector<std::size_t> indices(triangles_.size());
        std::iota(indices.begin(), indices.end(), 0);
        root_ = build(std::move(indices), 0);
    }
    centroids_.clear();
    centroids_.shrink_to_fit();
}

std::unique_ptr<KDNode> KDTree::build(std::vector<std::size_t> indices, int depth) {
    auto node = std::make_unique<KDNode>();
    node->bbox = AABB::from_triangle(triangles_[indices.front()]);
    for (std::size_t i = 1; i < indices.size(); ++i)
        node->bbox = AABB::surround(node->bbox, AABB::from_triangle(triangles_[indices[i]]));
    node->split_axis = depth % 3;
    if (indices.size() <= max_leaf_triangles_ || depth >= max_depth_) {
        node->triangle_indices = std::move(indices);
        return node;
    }
    const auto median = indices.begin() + indices.size() / 2;
    const int axis = node->split_axis;
    std::nth_element(indices.begin(), median, indices.end(), [&](std::size_t a, std::size_t b) {
        const double ca = coordinate(centroids_[a], axis);
        const double cb = coordinate(centroids_[b], axis);
        return ca < cb || (ca == cb && a < b);
    });
    node->split_value = coordinate(centroids_[*median], axis);
    node->left = build(std::vector<std::size_t>(indices.begin(), median), depth + 1);
    node->right = build(std::vector<std::size_t>(median, indices.end()), depth + 1);
    return node;
}

bool KDTree::hit(const Ray& ray, double t_min, double t_max, Hit& rec,
                 TriangleIntersectFn intersect_fn, RenderStats* stats) const {
    bool found = false;
    if (root_) traverse(*root_, ray, t_min, t_max, rec, found, intersect_fn, stats);
    return found;
}

void KDTree::traverse(const KDNode& node, const Ray& ray, double t_min, double& closest,
                      Hit& rec, bool& found, TriangleIntersectFn intersect_fn,
                      RenderStats* stats) const {
    if (stats) stats->aabb_tests.fetch_add(1, std::memory_order_relaxed);
    if (!node.bbox.intersect(ray, t_min, closest)) return;
    if (node.is_leaf()) {
        for (const auto index : node.triangle_indices) {
            if (stats) stats->triangle_tests.fetch_add(1, std::memory_order_relaxed);
            double t = 0.0, u = 0.0, v = 0.0;
            if ((triangles_[index].*intersect_fn)(ray, t_min, closest, t, u, v)
                && (!found || t < closest || (t == closest && index > rec.triangle_index))) {
                found = true;
                closest = t;
                rec = {t, u, v, index};
            }
        }
        return;
    }
    const double origin = coordinate(ray.origin, node.split_axis);
    const double direction = coordinate(ray.direction, node.split_axis);
    const bool left_first = origin < node.split_value
        || (origin == node.split_value && direction <= 0.0);
    const KDNode& first = left_first ? *node.left : *node.right;
    const KDNode& second = left_first ? *node.right : *node.left;
    traverse(first, ray, t_min, closest, rec, found, intersect_fn, stats);
    traverse(second, ray, t_min, closest, rec, found, intersect_fn, stats);
}
