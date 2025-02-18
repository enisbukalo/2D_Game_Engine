#include <gtest/gtest.h>
#include "../components/CGravity.h"
#include "../components/CName.h"
#include "../components/CTransform.h"
#include "../include/ComponentFactory.h"

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

TEST(ComponentFactoryTest, CreateBuiltInComponents)
{
    auto& factory = ComponentFactory::instance();

    // Test creating Transform component
    auto transform = factory.createComponent("Transform");
    ASSERT_NE(transform, nullptr);
    EXPECT_EQ(transform->getType(), "Transform");
    delete transform;

    // Test creating Name component
    auto name = factory.createComponent("Name");
    ASSERT_NE(name, nullptr);
    EXPECT_EQ(name->getType(), "Name");
    delete name;

    // Test creating Gravity component
    auto gravity = factory.createComponent("Gravity");
    ASSERT_NE(gravity, nullptr);
    EXPECT_EQ(gravity->getType(), "Gravity");
    delete gravity;
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