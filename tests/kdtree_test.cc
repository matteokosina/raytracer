#include "geometry/kdtree.h"
#include "io/obj_loader.h"
#include "view/renderer.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>

namespace {
World test_mesh() {
    World world;
    for (int z = 0; z < 3; ++z) {
        for (int y = -4; y < 4; ++y) {
            for (int x = -4; x < 4; ++x) {
                const Vector3 a{static_cast<double>(x),static_cast<double>(y),static_cast<double>(z)};
                const Vector3 b = a + Vector3{1,0,0};
                const Vector3 c = a + Vector3{0,1,0};
                const Vector3 d = a + Vector3{1,1,0};
                world.add_triangle({a,b,c});
                world.add_triangle({b,d,c});
            }
        }
    }
    return world;
}

void compare_hits(const World& world, const Ray& ray, double t_min, double t_max,
                  TriangleIntersectFn method, RenderStats* brute_stats = nullptr,
                  RenderStats* tree_stats = nullptr) {
    Hit brute{123,0.2,0.3,456}, tree = brute;
    const bool expected = world.intersect_bruteforce(ray,t_min,t_max,brute,method,brute_stats);
    const bool actual = world.intersect(ray,t_min,t_max,tree,method,tree_stats);
    ASSERT_EQ(actual,expected);
    EXPECT_EQ(tree.triangle_index,brute.triangle_index);
    EXPECT_EQ(tree.t,brute.t);
    EXPECT_EQ(tree.u,brute.u);
    EXPECT_EQ(tree.v,brute.v);
}

void verify_node(const KDNode& node, int depth, std::size_t leaf_limit, int depth_limit,
                 std::vector<std::size_t>& indices) {
    EXPECT_EQ(node.split_axis,depth % 3);
    if (node.is_leaf()) {
        EXPECT_FALSE(node.triangle_indices.empty());
        EXPECT_TRUE(node.triangle_indices.size() <= leaf_limit || depth >= depth_limit);
        indices.insert(indices.end(),node.triangle_indices.begin(),node.triangle_indices.end());
        return;
    }
    EXPECT_TRUE(node.triangle_indices.empty());
    ASSERT_NE(node.left,nullptr);
    ASSERT_NE(node.right,nullptr);
    for (const auto* child : {node.left.get(),node.right.get()}) {
        EXPECT_GE(child->bbox.min_bounds.x,node.bbox.min_bounds.x);
        EXPECT_GE(child->bbox.min_bounds.y,node.bbox.min_bounds.y);
        EXPECT_GE(child->bbox.min_bounds.z,node.bbox.min_bounds.z);
        EXPECT_LE(child->bbox.max_bounds.x,node.bbox.max_bounds.x);
        EXPECT_LE(child->bbox.max_bounds.y,node.bbox.max_bounds.y);
        EXPECT_LE(child->bbox.max_bounds.z,node.bbox.max_bounds.z);
        verify_node(*child,depth+1,leaf_limit,depth_limit,indices);
    }
}
}

TEST(KDTreeConstruction, PartitionsAllIndicesAndCyclesAxes) {
    const World world = test_mesh();
    const KDTree tree(world.triangles(),4,20);
    ASSERT_NE(tree.root(),nullptr);
    std::vector<std::size_t> indices;
    verify_node(*tree.root(),0,4,20,indices);
    std::sort(indices.begin(),indices.end());
    ASSERT_EQ(indices.size(),world.triangles().size());
    for (std::size_t i = 0; i < indices.size(); ++i) EXPECT_EQ(indices[i],i);
}

TEST(KDTreeConstruction, HandlesEmptySingletonDepthLimitAndRepeatedCentroids) {
    const KDTree empty({});
    EXPECT_EQ(empty.root(),nullptr);
    Hit hit{9,0.2,0.3,8};
    RenderStats stats;
    EXPECT_FALSE(empty.hit({{0,0,1},{0,0,-1}},0,10,hit,&Triangle::intersect_badouel,&stats));
    EXPECT_EQ(hit.t,9);
    EXPECT_EQ(stats.aabb_tests.load(),0u);
    const Triangle triangle{{0,0,0},{1,0,0},{0,1,0}};
    const KDTree singleton({triangle});
    ASSERT_NE(singleton.root(),nullptr);
    EXPECT_TRUE(singleton.root()->is_leaf());
    const std::vector<Triangle> duplicates(32,triangle);
    const KDTree limited(duplicates,1,0);
    EXPECT_TRUE(limited.root()->is_leaf());
    EXPECT_EQ(limited.root()->triangle_indices.size(),32u);
    const KDTree shallow(duplicates,1,2);
    std::vector<std::size_t> indices;
    verify_node(*shallow.root(),0,1,2,indices);
    EXPECT_EQ(indices.size(),32u);
    const KDTree repeated(duplicates,1,20);
    EXPECT_TRUE(repeated.hit({{0.25,0.25,1},{0,0,-1}},0,10,hit));
    EXPECT_EQ(hit.triangle_index,31u);
    EXPECT_THROW(KDTree(duplicates,0,20),std::invalid_argument);
    EXPECT_THROW(KDTree(duplicates,1,-1),std::invalid_argument);
}

TEST(KDTreeTraversal, ExactlyMatchesBruteForceForRandomMeshRays) {
    World world = test_mesh();
    world.build_kdtree();
    std::mt19937 generator(20261117);
    std::uniform_real_distribution<double> position(-6,6);
    std::uniform_real_distribution<double> slope(-0.4,0.4);
    RenderStats brute_stats, tree_stats;
    for (const auto method : {&Triangle::intersect_badouel,&Triangle::intersect_moeller}) {
        for (int i = 0; i < 5000; ++i) {
            SCOPED_TRACE(i);
            const Ray ray{{position(generator),position(generator),i % 2 ? 5.0 : -3.0},
                          {slope(generator),slope(generator),i % 2 ? -1.0 : 1.0}};
            compare_hits(world,ray,i % 3 ? 0.0 : 3.5,i % 5 ? 20.0 : 4.0,
                         method,&brute_stats,&tree_stats);
        }
        for (int i = 0; i < 2000; ++i) {
            SCOPED_TRACE(i);
            const Ray ray{{position(generator),position(generator),position(generator)},
                          {slope(generator),slope(generator),slope(generator)}};
            compare_hits(world,ray,0.0,100.0,method,&brute_stats,&tree_stats);
        }
    }
    EXPECT_GT(tree_stats.aabb_tests.load(),0u);
    EXPECT_EQ(brute_stats.aabb_tests.load(),0u);
    EXPECT_LT(tree_stats.triangle_tests.load(),brute_stats.triangle_tests.load()/4);
}

TEST(KDTreeTraversal, ExactlyMatchesBruteForceForOBJMesh) {
    World world = ObjLoader::load(TEST_SCENE_PATH);
    world.build_kdtree();
    const Camera camera(world.bounds_min(),world.bounds_max(),16.0/9.0);
    std::mt19937 generator(1234);
    std::uniform_real_distribution<double> uv(-0.1,1.1);
    for (const auto method : {&Triangle::intersect_badouel,&Triangle::intersect_moeller}) {
        for (int i = 0; i < 1500; ++i) {
            SCOPED_TRACE(i);
            compare_hits(world,camera.ray(uv(generator),uv(generator)),1e-6,1e30,method);
        }
    }
}

TEST(KDTreeTraversal, HandlesSplitCrossingTrianglesAndEqualDistanceTies) {
    World world;
    world.add_triangle({{-10,-1,2},{2,-1,2},{2,1,2}});
    world.add_triangle({{-2,-1,0},{10,-1,0},{-2,1,0}});
    world.add_triangle({{-10,-1,2},{2,-1,2},{2,1,2}});
    world.build_kdtree(1);
    for (const auto method : {&Triangle::intersect_badouel,&Triangle::intersect_moeller}) {
        compare_hits(world,{{0,0,-1},{0,0,1}},0,10,method);
        compare_hits(world,{{0,0,3},{0,0,-1}},0,10,method);
        compare_hits(world,{{-2,-1,3},{0,0,-1}},0,10,method);
        compare_hits(world,{{0,0,3},{0,0,-1}},1,1,method);
    }
    Hit hit;
    ASSERT_TRUE(world.intersect({{0,0,3},{0,0,-1}},0,10,hit));
    EXPECT_EQ(hit.triangle_index,2u);
}

TEST(KDTreeTraversal, CountsActualTestsAndPrunesRootMisses) {
    World world;
    world.add_triangle({{0,0,0},{1,0,0},{0,1,0}});
    world.build_kdtree();
    RenderStats stats;
    Hit hit;
    EXPECT_FALSE(world.intersect({{2,2,1},{0,0,-1}},0,10,hit,&Triangle::intersect_badouel,&stats));
    EXPECT_EQ(stats.aabb_tests.load(),1u);
    EXPECT_EQ(stats.triangle_tests.load(),0u);
    stats.reset();
    EXPECT_TRUE(world.intersect({{0.25,0.25,1},{0,0,-1}},0,10,hit,&Triangle::intersect_badouel,&stats));
    EXPECT_EQ(stats.aabb_tests.load(),1u);
    EXPECT_EQ(stats.triangle_tests.load(),1u);
}

TEST(WorldAcceleration, SurvivesMovesAndInvalidatesOnSceneMutation) {
    World original = test_mesh();
    original.build_kdtree();
    World moved = std::move(original);
    ASSERT_TRUE(moved.kdtree_enabled());
    compare_hits(moved,{{0.25,0.25,5},{0,0,-1}},0,10,&Triangle::intersect_badouel);
    moved.add_triangle({{0,0,4},{1,0,4},{0,1,4}});
    EXPECT_FALSE(moved.kdtree_enabled());
    Hit hit;
    ASSERT_TRUE(moved.intersect({{0.25,0.25,5},{0,0,-1}},0,10,hit));
    EXPECT_EQ(hit.t,1);
    moved.build_kdtree();
    compare_hits(moved,{{0.25,0.25,5},{0,0,-1}},0,10,&Triangle::intersect_badouel);
}

TEST(RendererAcceleration, ProducesIdenticalPixelsIncludingSecondaryRays) {
    World world = ObjLoader::load(TEST_SCENE_PATH);
    const Camera camera(world.bounds_min(),world.bounds_max(),2.0);
    for (const auto method : {&Triangle::intersect_badouel,&Triangle::intersect_moeller}) {
        RenderStats brute_stats, tree_stats;
        const auto brute = Renderer::render(world,camera,24,12,&brute_stats,method);
        world.build_kdtree();
        const auto tree = Renderer::render(world,camera,24,12,&tree_stats,method);
        ASSERT_EQ(brute.size(),tree.size());
        for (std::size_t i = 0; i < brute.size(); ++i) {
            EXPECT_EQ(brute[i].x,tree[i].x);
            EXPECT_EQ(brute[i].y,tree[i].y);
            EXPECT_EQ(brute[i].z,tree[i].z);
        }
        EXPECT_EQ(tree_stats.primary_rays.load(),288u);
        EXPECT_GT(tree_stats.aabb_tests.load(),0u);
        world = ObjLoader::load(TEST_SCENE_PATH);
    }
}
