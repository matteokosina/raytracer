#include "world/world.h"

#include <algorithm>
#include <limits>

void World::add_triangle(const Triangle& triangle) {
    triangles_.push_back(triangle);
    kdtree_.reset();
}

void World::build_kdtree(std::size_t max_leaf_triangles, int max_depth) {
    kdtree_ = std::make_unique<KDTree>(triangles_, max_leaf_triangles, max_depth);
}

bool World::intersect(const Ray& ray, double t_min, double t_max, Hit& hit,
                      TriangleIntersectFn intersect_fn, RenderStats* stats) const {
    return kdtree_ ? intersect_kdtree(ray, t_min, t_max, hit, intersect_fn, stats)
                   : intersect_bruteforce(ray, t_min, t_max, hit, intersect_fn, stats);
}

bool World::intersect_kdtree(const Ray& ray, double t_min, double t_max, Hit& hit,
                             TriangleIntersectFn intersect_fn, RenderStats* stats) const {
    return kdtree_->hit(ray, t_min, t_max, hit, intersect_fn, stats);
}

bool World::intersect_bruteforce(const Ray& ray, double t_min, double t_max, Hit& hit,
                                 TriangleIntersectFn intersect_fn, RenderStats* stats) const {
    bool found = false;
    double closest = t_max;
    // Brute Force: Der obere Grenzwert wird mit jedem Treffer enger; der Methodenzeiger
    // bleibt für alle Dreiecke gleich und vermeidet die Algorithmuswahl in dieser Schleife.
    for (std::size_t i = 0; i < triangles_.size(); ++i) {
        if (stats) stats->triangle_tests.fetch_add(1, std::memory_order_relaxed);
        double t = 0.0, u = 0.0, v = 0.0;
        if ((triangles_[i].*intersect_fn)(ray, t_min, closest, t, u, v)) {
            found = true;
            closest = t;
            hit = {t, u, v, i};
        }
    }
    return found;
}

Vector3 World::bounds_min() const {
    Vector3 result{std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};
    for (const auto& tri : triangles_) for (const auto& p : {tri.a, tri.b, tri.c}) {
        result.x = std::min(result.x,p.x); result.y = std::min(result.y,p.y); result.z = std::min(result.z,p.z);
    }
    return result;
}

Vector3 World::bounds_max() const {
    Vector3 result{-std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()};
    for (const auto& tri : triangles_) for (const auto& p : {tri.a, tri.b, tri.c}) {
        result.x = std::max(result.x,p.x); result.y = std::max(result.y,p.y); result.z = std::max(result.z,p.z);
    }
    return result;
}
