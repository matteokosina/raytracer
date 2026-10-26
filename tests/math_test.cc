#include "math/vector3.h"

#include <gtest/gtest.h>

TEST(Vector3, CrossProductUsesRightHandRule) {
    const Vector3 result = Vector3{1,0,0}.cross({0,1,0});
    EXPECT_DOUBLE_EQ(result.x,0.0);
    EXPECT_DOUBLE_EQ(result.y,0.0);
    EXPECT_DOUBLE_EQ(result.z,1.0);
}
