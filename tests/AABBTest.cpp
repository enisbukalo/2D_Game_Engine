#include <gtest/gtest.h>
#include "physics/Quadtree.h"

class AABBTest : public ::testing::Test
{
};

TEST_F(AABBTest, Construction)
{
    Vec2 position(1.0f, 2.0f);
    Vec2 size(3.0f, 4.0f);
    AABB box(position, size);

    EXPECT_EQ(box.position, position);
    EXPECT_EQ(box.halfSize, size);
}

TEST_F(AABBTest, ContainsPoint)
{
    AABB box(Vec2(0, 0), Vec2(2, 2));

    // Test points inside
    EXPECT_TRUE(box.contains(Vec2(0, 0)));    // Center
    EXPECT_TRUE(box.contains(Vec2(1, 1)));    // Top right quadrant
    EXPECT_TRUE(box.contains(Vec2(-1, 1)));   // Top left quadrant
    EXPECT_TRUE(box.contains(Vec2(-1, -1)));  // Bottom left quadrant
    EXPECT_TRUE(box.contains(Vec2(1, -1)));   // Bottom right quadrant

    // Test points on edges
    EXPECT_TRUE(box.contains(Vec2(2, 0)));   // Right edge
    EXPECT_TRUE(box.contains(Vec2(-2, 0)));  // Left edge
    EXPECT_TRUE(box.contains(Vec2(0, 2)));   // Top edge
    EXPECT_TRUE(box.contains(Vec2(0, -2)));  // Bottom edge

    // Test points outside
    EXPECT_FALSE(box.contains(Vec2(2.1f, 0)));   // Just outside right
    EXPECT_FALSE(box.contains(Vec2(-2.1f, 0)));  // Just outside left
    EXPECT_FALSE(box.contains(Vec2(0, 2.1f)));   // Just outside top
    EXPECT_FALSE(box.contains(Vec2(0, -2.1f)));  // Just outside bottom
    EXPECT_FALSE(box.contains(Vec2(3, 3)));      // Far outside
}

TEST_F(AABBTest, Intersection)
{
    AABB box1(Vec2(0, 0), Vec2(2, 2));

    // Test overlapping boxes
    EXPECT_TRUE(box1.intersects(AABB(Vec2(1, 1), Vec2(2, 2))));  // Partial overlap
    EXPECT_TRUE(box1.intersects(AABB(Vec2(0, 0), Vec2(1, 1))));  // Contained
    EXPECT_TRUE(box1.intersects(AABB(Vec2(0, 0), Vec2(3, 3))));  // Contains
    EXPECT_TRUE(box1.intersects(box1));                          // Self intersection

    // Test touching boxes (should intersect)
    EXPECT_TRUE(box1.intersects(AABB(Vec2(4, 0), Vec2(2, 2))));  // Edge touch
    EXPECT_TRUE(box1.intersects(AABB(Vec2(4, 4), Vec2(2, 2))));  // Corner touch

    // Test non-overlapping boxes
    EXPECT_FALSE(box1.intersects(AABB(Vec2(5, 0), Vec2(1, 1))));  // Separated horizontally
    EXPECT_FALSE(box1.intersects(AABB(Vec2(0, 5), Vec2(1, 1))));  // Separated vertically
    EXPECT_FALSE(box1.intersects(AABB(Vec2(5, 5), Vec2(1, 1))));  // Separated diagonally
}

TEST_F(AABBTest, SymmetricIntersection)
{
    // Test that intersection is symmetric (A intersects B iff B intersects A)
    AABB box1(Vec2(0, 0), Vec2(2, 2));
    AABB box2(Vec2(1, 1), Vec2(2, 2));
    AABB box3(Vec2(5, 5), Vec2(1, 1));

    EXPECT_EQ(box1.intersects(box2), box2.intersects(box1));  // Overlapping case
    EXPECT_EQ(box1.intersects(box3), box3.intersects(box1));  // Non-overlapping case
}

TEST_F(AABBTest, EdgeCases)
{
    // Test boxes with zero size
    AABB zeroBox(Vec2(0, 0), Vec2(0, 0));
    AABB normalBox(Vec2(0, 0), Vec2(1, 1));

    EXPECT_TRUE(zeroBox.contains(Vec2(0, 0)));
    EXPECT_TRUE(normalBox.intersects(zeroBox));
    EXPECT_TRUE(zeroBox.intersects(normalBox));

    // Test boxes with negative size (should be treated as positive)
    AABB negativeBox(Vec2(0, 0), Vec2(-2, -2));
    EXPECT_TRUE(negativeBox.contains(Vec2(1, 1)));
    EXPECT_TRUE(negativeBox.intersects(normalBox));
}