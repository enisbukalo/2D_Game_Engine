#include <gtest/gtest.h>
#include "../components/Component.h"
#include "../include/Entity.h"

// Test helper class
class TestEntity : public Entity
{
public:
    TestEntity(const std::string &tag, uint8_t id) : Entity(tag, id) {}
};

TEST(ComponentTest, BasicComponentFunctionality)
{
    Component component;
    EXPECT_TRUE(component.isActive());

    component.setActive(false);
    EXPECT_FALSE(component.isActive());
}

TEST(ComponentTest, TransformComponent)
{
    CTransform transform;

    // Test initial values
    EXPECT_FLOAT_EQ(transform.position.x, 0.0f);
    EXPECT_FLOAT_EQ(transform.position.y, 0.0f);
    EXPECT_FLOAT_EQ(transform.scale.x, 1.0f);
    EXPECT_FLOAT_EQ(transform.scale.y, 1.0f);
    EXPECT_FLOAT_EQ(transform.rotation, 0.0f);

    // Test update with velocity
    transform.velocity = Vec2(1.0f, 2.0f);
    transform.update(0.5f);
    EXPECT_FLOAT_EQ(transform.position.x, 0.5f);
    EXPECT_FLOAT_EQ(transform.position.y, 1.0f);
}

TEST(ComponentTest, GravityComponent)
{
    Entity     *entity    = new TestEntity("test", 0);
    CTransform *transform = entity->addComponent<CTransform>();
    CGravity   *gravity   = entity->addComponent<CGravity>();

    // Test gravity effect
    gravity->update(1.0f);
    EXPECT_FLOAT_EQ(transform->velocity.y, -9.81f);
}

TEST(ComponentTest, NameComponent)
{
    CName nameComp("TestEntity");
    EXPECT_EQ(nameComp.name, "TestEntity");

    CName defaultName;
    EXPECT_TRUE(defaultName.name.empty());
}