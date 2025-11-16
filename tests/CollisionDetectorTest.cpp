#include <gtest/gtest.h>
#include "CBoxCollider.h"
#include "CCircleCollider.h"
#include "CTransform.h"
#include "Entity.h"
#include "physics/CollisionDetector.h"

// Test helper class
class TestEntity : public Entity
{
public:
    TestEntity(const std::string& tag, uint8_t id) : Entity(tag, id) {}
};

class CollisionDetectorTest : public ::testing::Test
{
protected:
    // Helper to create an entity with a circle collider at a specific position
    std::shared_ptr<TestEntity> createCircleEntity(const Vec2& pos, float radius)
    {
        auto entity    = std::make_shared<TestEntity>("circle", nextId++);
        auto transform = entity->addComponent<CTransform>();
        transform->setPosition(pos);
        auto collider = entity->addComponent<CCircleCollider>(radius);
        return entity;
    }

    // Helper to create an entity with a box collider at a specific position
    std::shared_ptr<TestEntity> createBoxEntity(const Vec2& pos, float width, float height)
    {
        auto entity    = std::make_shared<TestEntity>("box", nextId++);
        auto transform = entity->addComponent<CTransform>();
        transform->setPosition(pos);
        auto collider = entity->addComponent<CBoxCollider>(width, height);
        return entity;
    }

private:
    static uint8_t nextId;
};

uint8_t CollisionDetectorTest::nextId = 1;

// ==================== Circle vs Circle Tests ====================

TEST_F(CollisionDetectorTest, CircleVsCircle_Overlapping)
{
    auto circle1 = createCircleEntity(Vec2(0.0f, 0.0f), 10.0f);
    auto circle2 = createCircleEntity(Vec2(10.0f, 0.0f), 10.0f);

    auto collider1 = circle1->getComponent<CCircleCollider>();
    auto collider2 = circle2->getComponent<CCircleCollider>();

    // Circles at distance 10, with radius 10 each = overlapping
    EXPECT_TRUE(CollisionDetector::intersects(collider1, collider2));
}

TEST_F(CollisionDetectorTest, CircleVsCircle_Touching)
{
    auto circle1 = createCircleEntity(Vec2(0.0f, 0.0f), 10.0f);
    auto circle2 = createCircleEntity(Vec2(20.0f, 0.0f), 10.0f);

    auto collider1 = circle1->getComponent<CCircleCollider>();
    auto collider2 = circle2->getComponent<CCircleCollider>();

    // Circles at distance 20, with radius 10 each = exactly touching
    EXPECT_TRUE(CollisionDetector::intersects(collider1, collider2));
}

TEST_F(CollisionDetectorTest, CircleVsCircle_Separated)
{
    auto circle1 = createCircleEntity(Vec2(0.0f, 0.0f), 10.0f);
    auto circle2 = createCircleEntity(Vec2(25.0f, 0.0f), 10.0f);

    auto collider1 = circle1->getComponent<CCircleCollider>();
    auto collider2 = circle2->getComponent<CCircleCollider>();

    // Circles at distance 25, with radius 10 each = separated
    EXPECT_FALSE(CollisionDetector::intersects(collider1, collider2));
}

TEST_F(CollisionDetectorTest, CircleVsCircle_DiagonalOverlap)
{
    auto circle1 = createCircleEntity(Vec2(0.0f, 0.0f), 10.0f);
    auto circle2 = createCircleEntity(Vec2(7.0f, 7.0f), 10.0f);

    auto collider1 = circle1->getComponent<CCircleCollider>();
    auto collider2 = circle2->getComponent<CCircleCollider>();

    // Circles at diagonal distance ~9.9, with radius 10 each = overlapping
    EXPECT_TRUE(CollisionDetector::intersects(collider1, collider2));
}

// ==================== Box vs Box Tests ====================

TEST_F(CollisionDetectorTest, BoxVsBox_Overlapping)
{
    auto box1 = createBoxEntity(Vec2(0.0f, 0.0f), 20.0f, 20.0f);
    auto box2 = createBoxEntity(Vec2(15.0f, 0.0f), 20.0f, 20.0f);

    auto collider1 = box1->getComponent<CBoxCollider>();
    auto collider2 = box2->getComponent<CBoxCollider>();

    // Boxes at distance 15, with half-width 10 each = overlapping
    EXPECT_TRUE(CollisionDetector::intersects(collider1, collider2));
}

TEST_F(CollisionDetectorTest, BoxVsBox_Touching)
{
    auto box1 = createBoxEntity(Vec2(0.0f, 0.0f), 20.0f, 20.0f);
    auto box2 = createBoxEntity(Vec2(20.0f, 0.0f), 20.0f, 20.0f);

    auto collider1 = box1->getComponent<CBoxCollider>();
    auto collider2 = box2->getComponent<CBoxCollider>();

    // Boxes at distance 20, with half-width 10 each = exactly touching
    EXPECT_TRUE(CollisionDetector::intersects(collider1, collider2));
}

TEST_F(CollisionDetectorTest, BoxVsBox_Separated)
{
    auto box1 = createBoxEntity(Vec2(0.0f, 0.0f), 20.0f, 20.0f);
    auto box2 = createBoxEntity(Vec2(25.0f, 0.0f), 20.0f, 20.0f);

    auto collider1 = box1->getComponent<CBoxCollider>();
    auto collider2 = box2->getComponent<CBoxCollider>();

    // Boxes at distance 25, with half-width 10 each = separated
    EXPECT_FALSE(CollisionDetector::intersects(collider1, collider2));
}

TEST_F(CollisionDetectorTest, BoxVsBox_VerticalOverlap)
{
    auto box1 = createBoxEntity(Vec2(0.0f, 0.0f), 20.0f, 20.0f);
    auto box2 = createBoxEntity(Vec2(0.0f, 15.0f), 20.0f, 20.0f);

    auto collider1 = box1->getComponent<CBoxCollider>();
    auto collider2 = box2->getComponent<CBoxCollider>();

    // Boxes at vertical distance 15, with half-height 10 each = overlapping
    EXPECT_TRUE(CollisionDetector::intersects(collider1, collider2));
}

TEST_F(CollisionDetectorTest, BoxVsBox_DifferentSizes)
{
    auto box1 = createBoxEntity(Vec2(0.0f, 0.0f), 20.0f, 20.0f);
    auto box2 = createBoxEntity(Vec2(25.0f, 0.0f), 40.0f, 40.0f);

    auto collider1 = box1->getComponent<CBoxCollider>();
    auto collider2 = box2->getComponent<CBoxCollider>();

    // Distance 25, half-widths 10 + 20 = 30 = overlapping
    EXPECT_TRUE(CollisionDetector::intersects(collider1, collider2));
}

// ==================== Circle vs Box Tests ====================

TEST_F(CollisionDetectorTest, CircleVsBox_Overlapping)
{
    auto circle = createCircleEntity(Vec2(0.0f, 0.0f), 10.0f);
    auto box    = createBoxEntity(Vec2(15.0f, 0.0f), 20.0f, 20.0f);

    auto circleCollider = circle->getComponent<CCircleCollider>();
    auto boxCollider    = box->getComponent<CBoxCollider>();

    // Circle at 0, radius 10, reaches to x=10
    // Box at 15, half-width 10, starts at x=5
    // Overlap from x=5 to x=10
    EXPECT_TRUE(CollisionDetector::intersects(circleCollider, boxCollider));
}

TEST_F(CollisionDetectorTest, CircleVsBox_Separated)
{
    auto circle = createCircleEntity(Vec2(0.0f, 0.0f), 10.0f);
    auto box    = createBoxEntity(Vec2(25.0f, 0.0f), 20.0f, 20.0f);

    auto circleCollider = circle->getComponent<CCircleCollider>();
    auto boxCollider    = box->getComponent<CBoxCollider>();

    // Circle at 0, radius 10, reaches to x=10
    // Box at 25, half-width 10, starts at x=15
    // Gap from x=10 to x=15
    EXPECT_FALSE(CollisionDetector::intersects(circleCollider, boxCollider));
}

TEST_F(CollisionDetectorTest, CircleVsBox_CircleInsideBox)
{
    auto circle = createCircleEntity(Vec2(0.0f, 0.0f), 5.0f);
    auto box    = createBoxEntity(Vec2(0.0f, 0.0f), 20.0f, 20.0f);

    auto circleCollider = circle->getComponent<CCircleCollider>();
    auto boxCollider    = box->getComponent<CBoxCollider>();

    // Small circle (radius 5) inside large box (half-size 10)
    EXPECT_TRUE(CollisionDetector::intersects(circleCollider, boxCollider));
}

TEST_F(CollisionDetectorTest, CircleVsBox_CircleAtCorner)
{
    auto circle = createCircleEntity(Vec2(15.0f, 15.0f), 8.0f);
    auto box    = createBoxEntity(Vec2(0.0f, 0.0f), 20.0f, 20.0f);

    auto circleCollider = circle->getComponent<CCircleCollider>();
    auto boxCollider    = box->getComponent<CBoxCollider>();

    // Circle near corner of box at (10, 10)
    // Distance from (15, 15) to (10, 10) = ~7.07
    // Circle radius 8 > 7.07, so overlapping
    EXPECT_TRUE(CollisionDetector::intersects(circleCollider, boxCollider));
}

TEST_F(CollisionDetectorTest, CircleVsBox_CircleAtCornerSeparated)
{
    auto circle = createCircleEntity(Vec2(18.0f, 18.0f), 5.0f);
    auto box    = createBoxEntity(Vec2(0.0f, 0.0f), 20.0f, 20.0f);

    auto circleCollider = circle->getComponent<CCircleCollider>();
    auto boxCollider    = box->getComponent<CBoxCollider>();

    // Circle far from corner of box at (10, 10)
    // Distance from (18, 18) to (10, 10) = ~11.3
    // Circle radius 5 < 11.3, so separated
    EXPECT_FALSE(CollisionDetector::intersects(circleCollider, boxCollider));
}

// ==================== Box vs Circle Tests (reversed order) ====================

TEST_F(CollisionDetectorTest, BoxVsCircle_Overlapping)
{
    auto box    = createBoxEntity(Vec2(0.0f, 0.0f), 20.0f, 20.0f);
    auto circle = createCircleEntity(Vec2(15.0f, 0.0f), 10.0f);

    auto boxCollider    = box->getComponent<CBoxCollider>();
    auto circleCollider = circle->getComponent<CCircleCollider>();

    // Test that order doesn't matter
    EXPECT_TRUE(CollisionDetector::intersects(boxCollider, circleCollider));
}

TEST_F(CollisionDetectorTest, BoxVsCircle_Separated)
{
    auto box    = createBoxEntity(Vec2(0.0f, 0.0f), 20.0f, 20.0f);
    auto circle = createCircleEntity(Vec2(25.0f, 0.0f), 10.0f);

    auto boxCollider    = box->getComponent<CBoxCollider>();
    auto circleCollider = circle->getComponent<CCircleCollider>();

    // Test that order doesn't matter
    EXPECT_FALSE(CollisionDetector::intersects(boxCollider, circleCollider));
}

// ==================== Edge Cases ====================

TEST_F(CollisionDetectorTest, NullColliders)
{
    EXPECT_FALSE(CollisionDetector::intersects(nullptr, nullptr));
}

TEST_F(CollisionDetectorTest, CircleVsCircle_ZeroRadius)
{
    auto circle1 = createCircleEntity(Vec2(0.0f, 0.0f), 0.0f);
    auto circle2 = createCircleEntity(Vec2(0.0f, 0.0f), 0.0f);

    auto collider1 = circle1->getComponent<CCircleCollider>();
    auto collider2 = circle2->getComponent<CCircleCollider>();

    // Two point circles at same location
    EXPECT_TRUE(CollisionDetector::intersects(collider1, collider2));
}

TEST_F(CollisionDetectorTest, BoxVsBox_ZeroSize)
{
    auto box1 = createBoxEntity(Vec2(0.0f, 0.0f), 0.0f, 0.0f);
    auto box2 = createBoxEntity(Vec2(0.0f, 0.0f), 0.0f, 0.0f);

    auto collider1 = box1->getComponent<CBoxCollider>();
    auto collider2 = box2->getComponent<CBoxCollider>();

    // Two point boxes at same location
    EXPECT_TRUE(CollisionDetector::intersects(collider1, collider2));
}

TEST_F(CollisionDetectorTest, CircleVsBox_LargeNumbers)
{
    auto circle = createCircleEntity(Vec2(1000.0f, 1000.0f), 15.0f);
    auto box    = createBoxEntity(Vec2(1010.0f, 1000.0f), 20.0f, 20.0f);

    auto circleCollider = circle->getComponent<CCircleCollider>();
    auto boxCollider    = box->getComponent<CBoxCollider>();

    // Circle at 1000, radius 15, reaches to x=1015
    // Box at 1010, half-width 10, starts at x=1000
    EXPECT_TRUE(CollisionDetector::intersects(circleCollider, boxCollider));
}
