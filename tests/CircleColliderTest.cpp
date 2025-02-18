#include <gtest/gtest.h>
#include "CCircleCollider.h"
#include "CTransform.h"
#include "Entity.h"

// Test helper class
class TestEntity : public Entity
{
public:
    TestEntity(const std::string& tag, uint8_t id) : Entity(tag, id) {}
};

class CircleColliderTest : public ::testing::Test
{
protected:
    // Helper to create an entity with a circle collider at a specific position
    std::shared_ptr<TestEntity> createColliderEntity(const Vec2& pos, float radius)
    {
        auto entity    = std::make_shared<TestEntity>("test", nextId++);
        auto transform = entity->addComponent<CTransform>();
        transform->setPosition(pos);
        auto collider = entity->addComponent<CCircleCollider>(radius);
        return entity;
    }

private:
    static uint8_t nextId;
};

uint8_t CircleColliderTest::nextId = 1;

TEST_F(CircleColliderTest, NoCollision)
{
    auto entity1 = createColliderEntity(Vec2(0, 0), 1.0f);
    auto entity2 = createColliderEntity(Vec2(3, 0), 1.0f);

    auto collider1 = entity1->getComponent<CCircleCollider>();
    auto collider2 = entity2->getComponent<CCircleCollider>();

    EXPECT_FALSE(collider1->intersects(collider2));
    EXPECT_FALSE(collider2->intersects(collider1));
}

TEST_F(CircleColliderTest, Collision)
{
    auto entity1 = createColliderEntity(Vec2(0, 0), 2.0f);
    auto entity2 = createColliderEntity(Vec2(3, 0), 2.0f);

    auto collider1 = entity1->getComponent<CCircleCollider>();
    auto collider2 = entity2->getComponent<CCircleCollider>();

    EXPECT_TRUE(collider1->intersects(collider2));
    EXPECT_TRUE(collider2->intersects(collider1));
}

TEST_F(CircleColliderTest, EdgeContact)
{
    auto entity1 = createColliderEntity(Vec2(0, 0), 2.0f);
    auto entity2 = createColliderEntity(Vec2(4, 0), 2.0f);

    auto collider1 = entity1->getComponent<CCircleCollider>();
    auto collider2 = entity2->getComponent<CCircleCollider>();

    EXPECT_TRUE(collider1->intersects(collider2));
    EXPECT_TRUE(collider2->intersects(collider1));
}

TEST_F(CircleColliderTest, DifferentRadii)
{
    auto entity1 = createColliderEntity(Vec2(0, 0), 1.0f);
    auto entity2 = createColliderEntity(Vec2(2, 0), 2.0f);

    auto collider1 = entity1->getComponent<CCircleCollider>();
    auto collider2 = entity2->getComponent<CCircleCollider>();

    EXPECT_TRUE(collider1->intersects(collider2));
    EXPECT_TRUE(collider2->intersects(collider1));
}

TEST_F(CircleColliderTest, DiagonalCollision)
{
    auto entity1 = createColliderEntity(Vec2(0, 0), 2.0f);
    auto entity2 = createColliderEntity(Vec2(2, 2), 2.0f);

    auto collider1 = entity1->getComponent<CCircleCollider>();
    auto collider2 = entity2->getComponent<CCircleCollider>();

    EXPECT_TRUE(collider1->intersects(collider2));
    EXPECT_TRUE(collider2->intersects(collider1));
}

TEST_F(CircleColliderTest, TriggerState)
{
    auto entity   = createColliderEntity(Vec2(0, 0), 1.0f);
    auto collider = entity->getComponent<CCircleCollider>();

    EXPECT_FALSE(collider->isTrigger());
    collider->setTrigger(true);
    EXPECT_TRUE(collider->isTrigger());
}

TEST_F(CircleColliderTest, BoundingBox)
{
    Vec2  position(1.0f, 2.0f);
    float radius = 2.0f;

    auto entity   = createColliderEntity(position, radius);
    auto collider = entity->getComponent<CCircleCollider>();

    AABB bounds = collider->getBounds();
    EXPECT_EQ(bounds.position, position);
    EXPECT_EQ(bounds.halfSize, Vec2(radius * 2, radius * 2));
}