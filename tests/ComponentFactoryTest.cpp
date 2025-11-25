#include <gtest/gtest.h>
#include "CName.h"
#include "CTransform.h"
#include "CPhysicsBody2D.h"
#include "CCollider2D.h"
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

    auto name = factory.createComponent("Name");
    ASSERT_NE(name, nullptr);
    EXPECT_EQ(name->getType(), "Name");

    auto physicsBody = factory.createComponent("PhysicsBody2D");
    ASSERT_NE(physicsBody, nullptr);
    EXPECT_EQ(physicsBody->getType(), "CPhysicsBody2D");

    auto collider = factory.createComponent("Collider2D");
    ASSERT_NE(collider, nullptr);
    EXPECT_EQ(collider->getType(), "CCollider2D");

    // Test invalid component type
    auto invalid = factory.createComponent("InvalidType");
    EXPECT_EQ(invalid, nullptr);

    // Clean up
    delete transform;
    delete name;
    delete physicsBody;
    delete collider;
}

TEST(ComponentFactoryTest, RegisterCustomComponent)
{
    auto& factory = ComponentFactory::instance();

    // Register a custom component creator
    factory.registerComponent<CCollider2D>("CustomCollider");

    // Create the component and verify its properties
    auto component = factory.createComponent("CustomCollider");
    ASSERT_NE(component, nullptr);
    EXPECT_EQ(component->getType(), "CCollider2D");

    auto collider2D = dynamic_cast<CCollider2D*>(component);
    ASSERT_NE(collider2D, nullptr);
    EXPECT_FALSE(collider2D->isSensor());

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

    auto physicsBody = factory.createComponent("PhysicsBody2D");
    ASSERT_NE(physicsBody, nullptr);
    EXPECT_NE(dynamic_cast<CPhysicsBody2D*>(physicsBody), nullptr);
    delete physicsBody;
}