#include <gtest/gtest.h>
#include <cmath>
#include "Vec2.h"

class Vec2Test : public ::testing::Test
{
protected:
    const float EPSILON = 0.0001f;  // For floating point comparisons
};

TEST_F(Vec2Test, Construction)
{
    Vec2 v1;  // Default constructor
    EXPECT_FLOAT_EQ(v1.x, 0.0f);
    EXPECT_FLOAT_EQ(v1.y, 0.0f);

    Vec2 v2(1.5f, -2.3f);  // Parameter constructor
    EXPECT_FLOAT_EQ(v2.x, 1.5f);
    EXPECT_FLOAT_EQ(v2.y, -2.3f);
}

TEST_F(Vec2Test, Length)
{
    Vec2 v(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v.length(), 5.0f);  // 3-4-5 triangle

    Vec2 zero;
    EXPECT_FLOAT_EQ(zero.length(), 0.0f);
}

TEST_F(Vec2Test, LengthSquared)
{
    Vec2 v(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v.lengthSquared(), 25.0f);  // 3^2 + 4^2 = 25

    Vec2 zero;
    EXPECT_FLOAT_EQ(zero.lengthSquared(), 0.0f);

    Vec2 v2(5.0f, 12.0f);
    EXPECT_FLOAT_EQ(v2.lengthSquared(), 169.0f);  // 5^2 + 12^2 = 169
}

TEST_F(Vec2Test, Normalize)
{
    Vec2 v(3.0f, 4.0f);
    v.normalize();
    EXPECT_FLOAT_EQ(v.x, 0.6f);
    EXPECT_FLOAT_EQ(v.y, 0.8f);
    EXPECT_NEAR(v.length(), 1.0f, EPSILON);
}

TEST_F(Vec2Test, Addition)
{
    Vec2 v1(1.0f, 2.0f);
    Vec2 v2(3.0f, -1.0f);

    // Test operator+
    Vec2 sum = v1 + v2;
    EXPECT_FLOAT_EQ(sum.x, 4.0f);
    EXPECT_FLOAT_EQ(sum.y, 1.0f);

    // Test operator+=
    v1 += v2;
    EXPECT_FLOAT_EQ(v1.x, 4.0f);
    EXPECT_FLOAT_EQ(v1.y, 1.0f);

    // Test add method
    Vec2 v3(1.0f, 1.0f);
    v3.add(Vec2(2.0f, 2.0f));
    EXPECT_FLOAT_EQ(v3.x, 3.0f);
    EXPECT_FLOAT_EQ(v3.y, 3.0f);
}

TEST_F(Vec2Test, Subtraction)
{
    Vec2 v1(1.0f, 2.0f);
    Vec2 v2(3.0f, 1.0f);

    // Test operator-
    Vec2 diff = v1 - v2;
    EXPECT_FLOAT_EQ(diff.x, -2.0f);
    EXPECT_FLOAT_EQ(diff.y, 1.0f);

    // Test operator-=
    v1 -= v2;
    EXPECT_FLOAT_EQ(v1.x, -2.0f);
    EXPECT_FLOAT_EQ(v1.y, 1.0f);
}

TEST_F(Vec2Test, ScalarMultiplication)
{
    Vec2 v(2.0f, -3.0f);

    // Test operator*
    Vec2 scaled1 = v * 2.0f;
    EXPECT_FLOAT_EQ(scaled1.x, 4.0f);
    EXPECT_FLOAT_EQ(scaled1.y, -6.0f);

    // Test operator*=
    v *= 2.0f;
    EXPECT_FLOAT_EQ(v.x, 4.0f);
    EXPECT_FLOAT_EQ(v.y, -6.0f);

    // Test scale method
    Vec2 v2(1.0f, 1.0f);
    v2.scale(3.0f);
    EXPECT_FLOAT_EQ(v2.x, 3.0f);
    EXPECT_FLOAT_EQ(v2.y, 3.0f);
}

TEST_F(Vec2Test, ScalarDivision)
{
    Vec2 v(2.0f, -3.0f);

    // Test operator/
    Vec2 divided = v / 2.0f;
    EXPECT_FLOAT_EQ(divided.x, 1.0f);
    EXPECT_FLOAT_EQ(divided.y, -1.5f);

    // Test operator/=
    v /= 2.0f;
    EXPECT_FLOAT_EQ(v.x, 1.0f);
    EXPECT_FLOAT_EQ(v.y, -1.5f);
}

TEST_F(Vec2Test, Rotation)
{
    Vec2        v(1.0f, 0.0f);
    const float PI = 3.14159265358979323846f;

    // Test 90 degree rotation
    v.rotate(PI / 2);
    EXPECT_NEAR(v.x, 0.0f, EPSILON);
    EXPECT_NEAR(v.y, 1.0f, EPSILON);

    // Test 180 degree rotation
    v.rotate(PI);
    EXPECT_NEAR(v.x, 0.0f, EPSILON);
    EXPECT_NEAR(v.y, -1.0f, EPSILON);
}

TEST_F(Vec2Test, Distance)
{
    Vec2 v1(1.0f, 1.0f);
    Vec2 v2(4.0f, 5.0f);

    EXPECT_FLOAT_EQ(v1.distance(v2), 5.0f);  // 3-4-5 triangle
    EXPECT_FLOAT_EQ(v2.distance(v1), 5.0f);  // Should be symmetric

    EXPECT_FLOAT_EQ(v1.distanceSquared(v2), 25.0f);
    EXPECT_FLOAT_EQ(v2.distanceSquared(v1), 25.0f);
}

TEST_F(Vec2Test, DotProduct)
{
    Vec2 v1(1.0f, 2.0f);
    Vec2 v2(3.0f, 4.0f);

    EXPECT_FLOAT_EQ(v1.dot(v2), 11.0f);  // 1*3 + 2*4
    EXPECT_FLOAT_EQ(v2.dot(v1), 11.0f);  // Should be symmetric
}

TEST_F(Vec2Test, Comparison)
{
    Vec2 v1(1.0f, 2.0f);
    Vec2 v2(1.0f, 2.0f);
    Vec2 v3(1.0f, 2.1f);

    EXPECT_TRUE(v1 == v2);
    EXPECT_FALSE(v1 == v3);
    EXPECT_FALSE(v1 != v2);
    EXPECT_TRUE(v1 != v3);

    // Test near-equality with epsilon
    Vec2 v4(1.0f, 2.0f + EPSILON / 2);
    EXPECT_TRUE(v1 == v4);
}