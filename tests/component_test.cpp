#include "Component.h"
#include <gtest/gtest.h>
#include "CGravity.h"
#include "CName.h"
#include "CTransform.h"
#include "Entity.h"

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
    EXPECT_FLOAT_EQ(transform.getPosition().x, 0.0f);
    EXPECT_FLOAT_EQ(transform.getPosition().y, 0.0f);
    EXPECT_FLOAT_EQ(transform.getScale().x, 1.0f);
    EXPECT_FLOAT_EQ(transform.getScale().y, 1.0f);
    EXPECT_FLOAT_EQ(transform.getRotation(), 0.0f);

    // Test update with velocity
    transform.setVelocity(Vec2(1.0f, 2.0f));
    transform.update(0.5f);
    EXPECT_FLOAT_EQ(transform.getPosition().x, 0.5f);
    EXPECT_FLOAT_EQ(transform.getPosition().y, 1.0f);
}

TEST(ComponentTest, GravityComponent)
{
    TestEntity entity("test", 1);
    auto       transform = entity.addComponent<CTransform>();
    auto       gravity   = entity.addComponent<CGravity>();

    const float EPSILON = 0.0001f;  // Small value for floating point comparison
    EXPECT_NEAR(gravity->getForce().x, 0.0f, EPSILON);
    EXPECT_NEAR(gravity->getForce().y, -9.81f, EPSILON);

    // Test gravity application
    float deltaTime = 1.0f;
    gravity->update(deltaTime);
}

TEST(ComponentTest, NameComponent)
{
    CName nameComp("TestEntity");
    EXPECT_EQ(nameComp.getName(), "TestEntity");

    CName defaultName;
    EXPECT_TRUE(defaultName.getName().empty());
}