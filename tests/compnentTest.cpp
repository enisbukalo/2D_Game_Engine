#include <gtest/gtest.h>
#include "CGravity.h"
#include "CName.h"
#include "CTransform.h"
#include "Component.h"
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

    void serialize(JsonBuilder& builder) const override
    {
        // Empty implementation for testing
    }

    void deserialize(const JsonValue& value) override
    {
        // Empty implementation for testing
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

    // Test that position and velocity can be set but aren't automatically updated
    Vec2 testPos(1.0f, 2.0f);
    Vec2 testVel(3.0f, 4.0f);
    transform.setPosition(testPos);
    transform.setVelocity(testVel);
    transform.update(0.5f);  // Update should not change position
    EXPECT_EQ(transform.getPosition(), testPos);
    EXPECT_EQ(transform.getVelocity(), testVel);
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

TEST(ComponentTest, TransformSerialization)
{
    // Create and set up original component
    CTransform transform1;
    transform1.setPosition(Vec2(100.0f, 200.0f));
    transform1.setVelocity(Vec2(5.0f, -3.0f));
    transform1.setScale(Vec2(2.0f, 3.0f));
    transform1.setRotation(45.0f);

    // Serialize
    JsonBuilder builder;
    transform1.serialize(builder);
    JsonValue json(builder.toString());

    // Create new component and deserialize
    CTransform transform2;
    transform2.deserialize(json);

    // Verify all values match
    EXPECT_EQ(transform2.getPosition(), transform1.getPosition());
    EXPECT_EQ(transform2.getVelocity(), transform1.getVelocity());
    EXPECT_EQ(transform2.getScale(), transform1.getScale());
    EXPECT_FLOAT_EQ(transform2.getRotation(), transform1.getRotation());
}

TEST(ComponentTest, GravitySerialization)
{
    // Create and set up original component
    CGravity gravity1;
    gravity1.setForce(Vec2(10.0f, -15.0f));

    // Serialize
    JsonBuilder builder;
    gravity1.serialize(builder);
    JsonValue json(builder.toString());

    // Create new component and deserialize
    CGravity gravity2;
    gravity2.deserialize(json);

    // Verify force matches
    EXPECT_EQ(gravity2.getForce(), gravity1.getForce());
}

TEST(ComponentTest, NameSerialization)
{
    // Create and set up original component
    CName name1("TestEntity");

    // Serialize
    JsonBuilder builder;
    name1.serialize(builder);
    JsonValue json(builder.toString());

    // Create new component and deserialize
    CName name2;
    name2.deserialize(json);

    // Verify name matches
    EXPECT_EQ(name2.getName(), name1.getName());
    EXPECT_EQ(name2.getName(), "TestEntity");
}

TEST(ComponentTest, ComponentSerializationRoundTrip)
{
    // Create an entity with all components
    TestEntity entity("test", 1);

    auto transform = entity.addComponent<CTransform>();
    transform->setPosition(Vec2(100.0f, 200.0f));
    transform->setVelocity(Vec2(5.0f, -3.0f));
    transform->setScale(Vec2(2.0f, 3.0f));
    transform->setRotation(45.0f);

    auto gravity = entity.addComponent<CGravity>();
    gravity->setForce(Vec2(10.0f, -15.0f));

    auto name = entity.addComponent<CName>();
    name->setName("CompleteObject");

    // Serialize entity
    JsonBuilder builder;
    builder.beginObject();
    builder.addKey("components");
    builder.beginArray();
    transform->serialize(builder);
    gravity->serialize(builder);
    name->serialize(builder);
    builder.endArray();
    builder.endObject();

    // Parse the JSON
    JsonValue   json(builder.toString());
    const auto& components = json["components"].getArray();

    // Create new components and deserialize
    CTransform transform2;
    CGravity   gravity2;
    CName      name2;
    transform2.deserialize(components[0]);
    gravity2.deserialize(components[1]);
    name2.deserialize(components[2]);

    // Verify all values match
    EXPECT_EQ(transform2.getPosition(), transform->getPosition());
    EXPECT_EQ(transform2.getVelocity(), transform->getVelocity());
    EXPECT_EQ(transform2.getScale(), transform->getScale());
    EXPECT_FLOAT_EQ(transform2.getRotation(), transform->getRotation());
    EXPECT_EQ(gravity2.getForce(), gravity->getForce());
    EXPECT_EQ(name2.getName(), name->getName());
}