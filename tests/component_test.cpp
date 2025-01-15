#include "../components/Component.h"
#include <gtest/gtest.h>
#include "../include/Entity.h"

// Test helper classes
class TestEntity : public Entity
{
public:
    TestEntity(const std::string& tag, uint8_t id) : Entity(tag, id) {}
};

class TestComponent : public Component
{
public:
    std::string getType() const override
    {
        return "Test";
    }
};

TEST(ComponentTest, BasicComponentFunctionality)
{
    TestComponent component;
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
    Entity*     entity    = new TestEntity("test", 0);
    CTransform* transform = entity->addComponent<CTransform>();
    CGravity*   gravity   = entity->addComponent<CGravity>();

    // Test gravity effect
    gravity->update(1.0f);
    EXPECT_FLOAT_EQ(transform->velocity.y, -9.81f);

    delete entity;
}

TEST(ComponentTest, NameComponent)
{
    CName nameComp("TestEntity");
    EXPECT_EQ(nameComp.name, "TestEntity");

    CName defaultName;
    EXPECT_TRUE(defaultName.name.empty());
}

TEST(ComponentTest, ComponentSerialization)
{
    // Test Transform serialization
    CTransform transform;
    transform.position = Vec2(1.0f, 2.0f);
    transform.velocity = Vec2(3.0f, 4.0f);
    transform.scale    = Vec2(2.0f, 2.0f);
    transform.rotation = 45.0f;

    json transformJson = transform.serialize();
    EXPECT_EQ(transformJson["type"], "Transform");
    EXPECT_FLOAT_EQ(transformJson["position"]["x"], 1.0f);
    EXPECT_FLOAT_EQ(transformJson["position"]["y"], 2.0f);
    EXPECT_FLOAT_EQ(transformJson["velocity"]["x"], 3.0f);
    EXPECT_FLOAT_EQ(transformJson["velocity"]["y"], 4.0f);
    EXPECT_FLOAT_EQ(transformJson["scale"]["x"], 2.0f);
    EXPECT_FLOAT_EQ(transformJson["scale"]["y"], 2.0f);
    EXPECT_FLOAT_EQ(transformJson["rotation"], 45.0f);

    // Test Name serialization
    CName name("TestName");
    json  nameJson = name.serialize();
    EXPECT_EQ(nameJson["type"], "Name");
    EXPECT_EQ(nameJson["name"], "TestName");

    // Test Gravity serialization
    CGravity gravity;
    gravity.force    = Vec2(1.0f, -15.0f);
    json gravityJson = gravity.serialize();
    EXPECT_EQ(gravityJson["type"], "Gravity");
    EXPECT_FLOAT_EQ(gravityJson["force"]["x"], 1.0f);
    EXPECT_FLOAT_EQ(gravityJson["force"]["y"], -15.0f);
}