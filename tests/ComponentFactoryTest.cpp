#include <gtest/gtest.h>
#include "CName.h"
#include "CTransform.h"
#include "CPhysicsBody2D.h"
#include "CCollider2D.h"
#include "CInputController.h"
#include "CRenderable.h"
#include "CTexture.h"
#include "CShader.h"
#include "CMaterial.h"
#include "ComponentFactory.h"

using namespace Components;

// Test helper component.
class TestComponent : public Component
{
public:
    std::string getType() const override
    {
        return "Test";
    }

    void serialize(Serialization::JsonBuilder& builder) const override
    {
        // Empty implementation for testing
    }

    void deserialize(const Serialization::SSerialization::JsonValue& value) override
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

    auto inputController = factory.createComponent("CInputController");
    ASSERT_NE(inputController, nullptr);
    EXPECT_EQ(inputController->getType(), "CInputController");

    auto inputControllerAlias = factory.createComponent("InputController");
    ASSERT_NE(inputControllerAlias, nullptr);
    EXPECT_EQ(inputControllerAlias->getType(), "CInputController");

    // Test rendering components
    auto renderable = factory.createComponent("Renderable");
    ASSERT_NE(renderable, nullptr);
    EXPECT_EQ(renderable->getType(), "Renderable");

    auto texture = factory.createComponent("Texture");
    ASSERT_NE(texture, nullptr);
    EXPECT_EQ(texture->getType(), "Texture");

    auto shader = factory.createComponent("Shader");
    ASSERT_NE(shader, nullptr);
    EXPECT_EQ(shader->getType(), "Shader");

    auto material = factory.createComponent("Material");
    ASSERT_NE(material, nullptr);
    EXPECT_EQ(material->getType(), "Material");

    // Test invalid component type
    auto invalid = factory.createComponent("InvalidType");
    EXPECT_EQ(invalid, nullptr);

    // Clean up
    delete transform;
    delete name;
    delete physicsBody;
    delete collider;
    delete inputController;
    delete inputControllerAlias;
    delete renderable;
    delete texture;
    delete shader;
    delete material;
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

    auto inputController = factory.createComponent("CInputController");
    ASSERT_NE(inputController, nullptr);
    EXPECT_NE(dynamic_cast<CInputController*>(inputController), nullptr);
    delete inputController;

    auto renderable = factory.createComponent("Renderable");
    ASSERT_NE(renderable, nullptr);
    EXPECT_NE(dynamic_cast<CRenderable*>(renderable), nullptr);
    delete renderable;

    auto texture = factory.createComponent("Texture");
    ASSERT_NE(texture, nullptr);
    EXPECT_NE(dynamic_cast<CTexture*>(texture), nullptr);
    delete texture;

    auto shader = factory.createComponent("Shader");
    ASSERT_NE(shader, nullptr);
    EXPECT_NE(dynamic_cast<CShader*>(shader), nullptr);
    delete shader;

    auto material = factory.createComponent("Material");
    ASSERT_NE(material, nullptr);
    EXPECT_NE(dynamic_cast<CMaterial*>(material), nullptr);
    delete material;
}