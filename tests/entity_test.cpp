#include "Entity.h"
#include <gtest/gtest.h>
#include "Component.h"
#include "Vec2.h"
#include "components/CGravity.h"
#include "components/CName.h"
#include "components/CTransform.h"

// Test helper class that exposes protected constructor
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

    CTransform *transform = entity.addComponent<CTransform>();
    CName      *name      = entity.addComponent<CName>("TestEntity");
    CGravity   *gravity   = entity.addComponent<CGravity>();

    EXPECT_TRUE(entity.hasComponent<CTransform>());
    EXPECT_TRUE(entity.hasComponent<CName>());
    EXPECT_TRUE(entity.hasComponent<CGravity>());

    EXPECT_EQ(name->getName(), "TestEntity");
}

TEST(EntityTest, ComponentUpdate)
{
    TestEntity entity("test", 1);

    CTransform *transform = entity.addComponent<CTransform>();
    transform->setVelocity(Vec2(1.0f, 1.0f));

    entity.update(1.0f);
    EXPECT_FLOAT_EQ(transform->getPosition().x, 1.0f);
    EXPECT_FLOAT_EQ(transform->getPosition().y, 1.0f);
}