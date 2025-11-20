#include <gtest/gtest.h>
#include "CBoxCollider.h"
#include "CTransform.h"
#include "Entity.h"

// Test helper class
class TestEntity : public Entity
{
public:
    TestEntity(const std::string& tag, uint8_t id) : Entity(tag, id) {}
};

class BoxColliderTest : public ::testing::Test
{
protected:
    // Helper to create an entity with a box collider at a specific position
    std::shared_ptr<TestEntity> createBoxEntity(const Vec2& pos, float width, float height)
    {
        auto entity    = std::make_shared<TestEntity>("testBox", nextId++);
        auto transform = entity->addComponent<CTransform>();
        transform->setPosition(pos);
        auto collider = entity->addComponent<CBoxCollider>(width, height);
        return entity;
    }

private:
    static uint8_t nextId;
};

uint8_t BoxColliderTest::nextId = 1;

// ==================== Construction Tests ====================

TEST_F(BoxColliderTest, ConstructorWithWidthHeight)
{
    auto entity   = std::make_shared<TestEntity>("box", 1);
    auto collider = entity->addComponent<CBoxCollider>(30.0f, 50.0f);

    EXPECT_EQ(collider->getWidth(), 30.0f);
    EXPECT_EQ(collider->getHeight(), 50.0f);
    EXPECT_FALSE(collider->isTrigger());
}

TEST_F(BoxColliderTest, ConstructorWithVec2)
{
    auto entity = std::make_shared<TestEntity>("box", 1);
    Vec2 size(100.0f, 200.0f);
    auto collider = entity->addComponent<CBoxCollider>(size);

    EXPECT_EQ(collider->getWidth(), 100.0f);
    EXPECT_EQ(collider->getHeight(), 200.0f);
    Vec2 retrievedSize = collider->getSize();
    EXPECT_EQ(retrievedSize.x, 100.0f);
    EXPECT_EQ(retrievedSize.y, 200.0f);
}

// ==================== Getters and Setters Tests ====================

TEST_F(BoxColliderTest, GettersAndSetters)
{
    auto entity   = std::make_shared<TestEntity>("box", 1);
    auto collider = entity->addComponent<CBoxCollider>(10.0f, 20.0f);

    EXPECT_EQ(collider->getWidth(), 10.0f);
    EXPECT_EQ(collider->getHeight(), 20.0f);

    collider->setSize(40.0f, 60.0f);
    EXPECT_EQ(collider->getWidth(), 40.0f);
    EXPECT_EQ(collider->getHeight(), 60.0f);

    Vec2 newSize(80.0f, 100.0f);
    collider->setSize(newSize);
    EXPECT_EQ(collider->getWidth(), 80.0f);
    EXPECT_EQ(collider->getHeight(), 100.0f);
}

TEST_F(BoxColliderTest, TriggerFlag)
{
    auto entity   = std::make_shared<TestEntity>("box", 1);
    auto collider = entity->addComponent<CBoxCollider>(10.0f, 10.0f);

    EXPECT_FALSE(collider->isTrigger());

    collider->setTrigger(true);
    EXPECT_TRUE(collider->isTrigger());

    collider->setTrigger(false);
    EXPECT_FALSE(collider->isTrigger());
}

// ==================== AABB Bounds Tests ====================

TEST_F(BoxColliderTest, GetBounds)
{
    auto entity    = std::make_shared<TestEntity>("box", 1);
    auto transform = entity->addComponent<CTransform>();
    transform->setPosition(Vec2(100.0f, 200.0f));
    auto collider = entity->addComponent<CBoxCollider>(40.0f, 60.0f);

    AABB bounds = collider->getBounds();

    EXPECT_EQ(bounds.position.x, 100.0f);
    EXPECT_EQ(bounds.position.y, 200.0f);
    EXPECT_EQ(bounds.halfSize.x, 20.0f);  // Half of width 40.0f
    EXPECT_EQ(bounds.halfSize.y, 30.0f);  // Half of height 60.0f
}

TEST_F(BoxColliderTest, GetBoundsAfterMove)
{
    auto entity    = std::make_shared<TestEntity>("box", 1);
    auto transform = entity->addComponent<CTransform>();
    transform->setPosition(Vec2(0.0f, 0.0f));
    auto collider = entity->addComponent<CBoxCollider>(20.0f, 20.0f);

    AABB bounds1 = collider->getBounds();
    EXPECT_EQ(bounds1.position.x, 0.0f);
    EXPECT_EQ(bounds1.position.y, 0.0f);

    transform->setPosition(Vec2(50.0f, 75.0f));

    AABB bounds2 = collider->getBounds();
    EXPECT_EQ(bounds2.position.x, 50.0f);
    EXPECT_EQ(bounds2.position.y, 75.0f);
}

// ==================== Type Tests ====================

TEST_F(BoxColliderTest, GetType)
{
    auto entity   = std::make_shared<TestEntity>("box", 1);
    auto collider = entity->addComponent<CBoxCollider>(10.0f, 10.0f);

    EXPECT_EQ(collider->getType(), "BoxCollider");
}

// ==================== Edge Cases ====================

TEST_F(BoxColliderTest, ZeroSize)
{
    auto entity   = std::make_shared<TestEntity>("box", 1);
    auto collider = entity->addComponent<CBoxCollider>(0.0f, 0.0f);

    EXPECT_EQ(collider->getWidth(), 0.0f);
    EXPECT_EQ(collider->getHeight(), 0.0f);
}

TEST_F(BoxColliderTest, LargeSize)
{
    auto entity   = std::make_shared<TestEntity>("box", 1);
    auto collider = entity->addComponent<CBoxCollider>(10000.0f, 20000.0f);

    EXPECT_EQ(collider->getWidth(), 10000.0f);
    EXPECT_EQ(collider->getHeight(), 20000.0f);
}

TEST_F(BoxColliderTest, RectangularBox)
{
    auto entity   = std::make_shared<TestEntity>("box", 1);
    auto collider = entity->addComponent<CBoxCollider>(100.0f, 20.0f);

    EXPECT_EQ(collider->getWidth(), 100.0f);
    EXPECT_EQ(collider->getHeight(), 20.0f);
    EXPECT_NE(collider->getWidth(), collider->getHeight());
}

TEST_F(BoxColliderTest, SquareBox)
{
    auto entity   = std::make_shared<TestEntity>("box", 1);
    auto collider = entity->addComponent<CBoxCollider>(50.0f, 50.0f);

    EXPECT_EQ(collider->getWidth(), 50.0f);
    EXPECT_EQ(collider->getHeight(), 50.0f);
    EXPECT_EQ(collider->getWidth(), collider->getHeight());
}
