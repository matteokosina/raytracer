#include "geometry/triangle.h"

#include <gtest/gtest.h>
#include <random>

TEST(TriangleIntersection, HitsInsideTriangle) {
    const Triangle triangle{{0,0,0},{1,0,0},{0,1,0}};
    double t=0.0, u=0.0, v=0.0;
    EXPECT_TRUE(triangle.intersect({{0.25,0.25,1},{0,0,-1}},0.0,10.0,t,u,v));
    EXPECT_DOUBLE_EQ(t,1.0);
    EXPECT_NEAR(u,0.25,1e-12);
    EXPECT_NEAR(v,0.25,1e-12);
}

TEST(TriangleIntersection, RejectsOutsideAndParallelRays) {
    const Triangle triangle{{0,0,0},{1,0,0},{0,1,0}};
    double t=0.0, u=0.0, v=0.0;
    EXPECT_FALSE(triangle.intersect({{2,2,1},{0,0,-1}},0.0,10.0,t,u,v));
    EXPECT_FALSE(triangle.intersect({{0.2,0.2,1},{1,0,0}},0.0,10.0,t,u,v));
}

TEST(TriangleIntersection, BadouelAndMollerTrumboreAgreeOnRandomRays) {
    const Triangle triangle{{0,0,0},{1,0,0},{0,1,0}};
    std::mt19937 generator(20261026);
    std::uniform_real_distribution<double> xy(-1.5,1.5);
    std::uniform_real_distribution<double> height(0.1,5.0);
    std::uniform_real_distribution<double> slope(-0.5,0.5);
    for (int i=0;i<20000;++i) {
        const Ray ray{{xy(generator),xy(generator),height(generator)},
                      {slope(generator),slope(generator),-1.0}};
        double badouel_t=0.0, badouel_u=0.0, badouel_v=0.0;
        double moeller_t=0.0, moeller_u=0.0, moeller_v=0.0;
        const bool badouel_hit = triangle.intersect(ray,0.0,100.0,badouel_t,badouel_u,badouel_v,
            TriangleIntersectionAlgorithm::Badouel);
        const bool moeller_hit = triangle.intersect(ray,0.0,100.0,moeller_t,moeller_u,moeller_v,
            TriangleIntersectionAlgorithm::MollerTrumbore);
        ASSERT_EQ(badouel_hit,moeller_hit) << "ray index " << i;
        if (badouel_hit) {
            EXPECT_NEAR(badouel_t,moeller_t,1e-10) << "ray index " << i;
            EXPECT_NEAR(badouel_u,moeller_u,1e-10) << "ray index " << i;
            EXPECT_NEAR(badouel_v,moeller_v,1e-10) << "ray index " << i;
        }
    }
}
