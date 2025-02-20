#include <gtest/gtest.h>
#include "CCircleCollider.h"
#include "CGravity.h"
#include "CName.h"
#include "CTransform.h"
#include "Component.h"
#include "Entity.h"
#include "Vec2.h"

// Test helper class that exposes protected constructor.
class TestEntity : public Entity
{
public:
    TestEntity(const std::string &tag, uint8_t id) : Entity(tag, id) {}
};

TEST(EntityTest, BasicEntityFunctionality)
{
    TestEntity entity("test", 1);
    EXPECT_TRUE(entity.isAlive());
    EXPECT_EQ(entity.getTag(), "test");
    EXPECT_EQ(entity.getId(), 1);

    entity.destroy();
    EXPECT_FALSE(entity.isAlive());
}

TEST(EntityTest, ComponentManagement)
{
    TestEntity entity("test", 1);

    // Add component
    CTransform *transform = entity.addComponent<CTransform>();
    EXPECT_TRUE(entity.hasComponent<CTransform>());
    EXPECT_NE(transform, nullptr);

    // Get component
    CTransform *retrievedTransform = entity.getComponent<CTransform>();
    EXPECT_EQ(transform, retrievedTransform);

    // Remove component
    entity.removeComponent<CTransform>();
    EXPECT_FALSE(entity.hasComponent<CTransform>());
    EXPECT_EQ(entity.getComponent<CTransform>(), nullptr);
}

TEST(EntityTest, MultipleComponents)
{
    TestEntity entity("test", 1);

    CTransform      *transform = entity.addComponent<CTransform>();
    CName           *name      = entity.addComponent<CName>("TestEntity");
    CGravity        *gravity   = entity.addComponent<CGravity>();
    CCircleCollider *collider  = entity.addComponent<CCircleCollider>(2.0f);

    EXPECT_TRUE(entity.hasComponent<CTransform>());
    EXPECT_TRUE(entity.hasComponent<CName>());
    EXPECT_TRUE(entity.hasComponent<CGravity>());
    EXPECT_TRUE(entity.hasComponent<CCircleCollider>());

    EXPECT_EQ(name->getName(), "TestEntity");
    EXPECT_FLOAT_EQ(collider->getRadius(), 2.0f);
    EXPECT_FALSE(collider->isTrigger());
}

TEST(EntityTest, ComponentUpdate)
{
    TestEntity entity("test", 1);

    CTransform *transform = entity.addComponent<CTransform>();
    Vec2        initialPos(1.0f, 1.0f);
    Vec2        initialVel(2.0f, 2.0f);
    transform->setPosition(initialPos);
    transform->setVelocity(initialVel);

    // Update should not change position since physics system handles that now
    entity.update(1.0f);
    EXPECT_EQ(transform->getPosition(), initialPos);
    EXPECT_EQ(transform->getVelocity(), initialVel);
}