#include <gtest/gtest.h>
#include "CCircleCollider.h"
#include "CGravity.h"
#include "CName.h"
#include "CTransform.h"
#include "ComponentFactory.h"

// Test helper component.
class TestComponent : public Component
{
public:
    std::string getType() const override
    {
        return "Test";
    }

    void serialize(JsonBuilder& builder) const override
    {
        // Empty implementation for testing
    }

    void deserialize(const JsonValue& value) override
    {
        // Empty implementation for testing
    }
};

TEST(ComponentFactoryTest, CreateComponents)
{
    auto& factory = ComponentFactory::instance();

    // Test creating each component type
    auto transform = factory.createComponent("Transform");
    ASSERT_NE(transform, nullptr);
    EXPECT_EQ(transform->getType(), "Transform");

    auto gravity = factory.createComponent("Gravity");
    ASSERT_NE(gravity, nullptr);
    EXPECT_EQ(gravity->getType(), "Gravity");

    auto name = factory.createComponent("Name");
    ASSERT_NE(name, nullptr);
    EXPECT_EQ(name->getType(), "Name");

    auto circleCollider = factory.createComponent("CircleCollider");
    ASSERT_NE(circleCollider, nullptr);
    EXPECT_EQ(circleCollider->getType(), "CircleCollider");

    // Test invalid component type
    auto invalid = factory.createComponent("InvalidType");
    EXPECT_EQ(invalid, nullptr);

    // Clean up
    delete transform;
    delete gravity;
    delete name;
    delete circleCollider;
}

TEST(ComponentFactoryTest, RegisterCustomComponent)
{
    auto& factory = ComponentFactory::instance();

    // Register a custom component creator
    factory.registerComponent<CCircleCollider>("CircleCollider");

    // Create the component and verify its properties
    auto component = factory.createComponent("CircleCollider");
    ASSERT_NE(component, nullptr);
    EXPECT_EQ(component->getType(), "CircleCollider");

    auto circleCollider = dynamic_cast<CCircleCollider*>(component);
    ASSERT_NE(circleCollider, nullptr);
    EXPECT_FLOAT_EQ(circleCollider->getRadius(), 1.0f);
    EXPECT_FALSE(circleCollider->isTrigger());

    delete component;
}

TEST(ComponentFactoryTest, CreateNonExistentComponent)
{
    auto& factory   = ComponentFactory::instance();
    auto  component = factory.createComponent("NonExistent");
    EXPECT_EQ(component, nullptr);
}

TEST(ComponentFactoryTest, RegisterAndCreateCustomComponent)
{
    auto& factory = ComponentFactory::instance();

    // Register custom component
    factory.registerComponent<TestComponent>("Test");

    // Create and verify the custom component
    auto component = factory.createComponent("Test");
    ASSERT_NE(component, nullptr);
    EXPECT_EQ(component->getType(), "Test");
    delete component;
}

TEST(ComponentFactoryTest, ComponentTypeCorrectness)
{
    auto& factory = ComponentFactory::instance();

    // Create components and verify their concrete types
    auto transform = factory.createComponent("Transform");
    ASSERT_NE(transform, nullptr);
    EXPECT_NE(dynamic_cast<CTransform*>(transform), nullptr);
    delete transform;

    auto name = factory.createComponent("Name");
    ASSERT_NE(name, nullptr);
    EXPECT_NE(dynamic_cast<CName*>(name), nullptr);
    delete name;

    auto gravity = factory.createComponent("Gravity");
    ASSERT_NE(gravity, nullptr);
    EXPECT_NE(dynamic_cast<CGravity*>(gravity), nullptr);
    delete gravity;
}