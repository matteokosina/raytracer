#include "geometry/aabb.h"

#include <gtest/gtest.h>
#include <limits>

TEST(AABBIntersection, HandlesDirectionsAndClippedIntervals) {
    const AABB box{{0,0,0},{1,1,1}};
    EXPECT_TRUE(box.intersect({{-1,0.5,0.5},{1,0,0}},0.0,10.0));
    EXPECT_TRUE(box.intersect({{2,0.5,0.5},{-1,0,0}},0.0,10.0));
    EXPECT_TRUE(box.intersect({{0.5,-1,0.5},{0,1,0}},0.0,10.0));
    EXPECT_TRUE(box.intersect({{0.5,0.5,2},{0,0,-1}},0.0,10.0));
    EXPECT_TRUE(box.intersect({{0.5,0.5,0.5},{1,1,1}},0.0,10.0));
    EXPECT_FALSE(box.intersect({{-1,2,0.5},{1,0,0}},0.0,10.0));
    EXPECT_FALSE(box.intersect({{-1,0.5,0.5},{-1,0,0}},0.0,10.0));
    EXPECT_FALSE(box.intersect({{-1,0.5,0.5},{1,0,0}},0.0,0.5));
    EXPECT_FALSE(box.intersect({{-1,0.5,0.5},{1,0,0}},2.5,10.0));
    EXPECT_FALSE(box.intersect({{-1,0.5,0.5},{1,0,0}},2.0,1.0));
}

TEST(AABBIntersection, IncludesTangenciesAndZeroWidthSlabs) {
    const AABB box{{0,0,0},{1,1,1}};
    EXPECT_TRUE(box.intersect({{-1,0,1},{1,-0.0,0}},1.0,1.0));
    EXPECT_TRUE(box.intersect({{-1,2,0.5},{1,-2,0}},0.0,10.0));
    EXPECT_TRUE(box.intersect({{0,0.5,0.5},{0,0,0}},0.0,1.0));
    EXPECT_FALSE(box.intersect({{-1,0.5,0.5},{0,0,0}},0.0,1.0));
    const AABB flat{{0,0,0},{1,1,0}};
    EXPECT_TRUE(flat.intersect({{0.25,0.25,1},{0,0,-1}},0.0,10.0));
    const AABB point{{1,1,1},{1,1,1}};
    EXPECT_TRUE(point.intersect({{0,0,0},{1,1,1}},0.0,10.0));
}

TEST(AABBIntersection, RetainsTinyNonzeroDirectionIntersections) {
    const AABB box{{0,0,0},{1,1,1}};
    EXPECT_TRUE(box.intersect({{-1,0.5,0.5},{1e-15,0,0}},0.0,2e15));
    EXPECT_FALSE(box.intersect({{-1,0.5,0.5},{1e-15,0,0}},0.0,1.0));
    const double tiny = std::numeric_limits<double>::denorm_min();
    EXPECT_TRUE(box.intersect({{0,0.5,0.5},{tiny,0,0}},0.0,1.0));
    EXPECT_TRUE(box.intersect({{1,0.5,0.5},{-tiny,0,0}},0.0,1.0));
    EXPECT_TRUE(box.intersect({{-tiny,0.5,0.5},{tiny,0,0}},0.0,2.0));
    EXPECT_FALSE(box.intersect({{-tiny,0.5,0.5},{-tiny,0,0}},0.0,2.0));
}

TEST(AABBIntersection, ComputesAndMergesTriangleBounds) {
    const AABB first = AABB::from_triangle({{-2,3,0},{1,-1,0},{0,2,0}});
    EXPECT_DOUBLE_EQ(first.min_bounds.x,-2);
    EXPECT_DOUBLE_EQ(first.min_bounds.y,-1);
    EXPECT_DOUBLE_EQ(first.min_bounds.z,0);
    EXPECT_DOUBLE_EQ(first.max_bounds.x,1);
    EXPECT_DOUBLE_EQ(first.max_bounds.y,3);
    EXPECT_DOUBLE_EQ(first.max_bounds.z,0);
    const AABB combined = AABB::surround(first,{{-3,0,-4},{0,5,2}});
    EXPECT_DOUBLE_EQ(combined.min_bounds.x,-3);
    EXPECT_DOUBLE_EQ(combined.min_bounds.y,-1);
    EXPECT_DOUBLE_EQ(combined.min_bounds.z,-4);
    EXPECT_DOUBLE_EQ(combined.max_bounds.x,1);
    EXPECT_DOUBLE_EQ(combined.max_bounds.y,5);
    EXPECT_DOUBLE_EQ(combined.max_bounds.z,2);
}
