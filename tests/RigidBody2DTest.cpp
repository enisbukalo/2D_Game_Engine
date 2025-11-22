#include <gtest/gtest.h>
#include "CRigidBody2D.h"
#include "Entity.h"
#include "JsonBuilder.h"
#include "JsonValue.h"

// Test helper class
class TestEntity : public Entity
{
public:
    TestEntity(const std::string& tag, uint8_t id) : Entity(tag, id) {}
};

// ============================================================================
// Basic Functionality Tests
// ============================================================================

TEST(RigidBody2DTest, DefaultValues)
{
    CRigidBody2D rb;

    // Mass properties
    EXPECT_FLOAT_EQ(rb.getMass(), 1.0f);
    EXPECT_FLOAT_EQ(rb.getInverseMass(), 1.0f);

    // Material properties
    EXPECT_FLOAT_EQ(rb.getRestitution(), 0.5f);
    EXPECT_FLOAT_EQ(rb.getFriction(), 0.3f);
    EXPECT_FLOAT_EQ(rb.getLinearDrag(), 0.25f);
    EXPECT_FLOAT_EQ(rb.getAngularDrag(), 0.05f);

    // Gravity settings
    EXPECT_TRUE(rb.getUseGravity());
    EXPECT_FLOAT_EQ(rb.getGravityScale(), 1.0f);

    // Body type
    EXPECT_FALSE(rb.isKinematic());

    // Constraints
    EXPECT_FALSE(rb.getFreezePositionX());
    EXPECT_FALSE(rb.getFreezePositionY());
    EXPECT_FALSE(rb.getFreezeRotation());

    // Forces
    EXPECT_EQ(rb.getAccumulatedForce(), Vec2(0.0f, 0.0f));
    EXPECT_EQ(rb.getTotalForce(), Vec2(0.0f, 0.0f));
}

TEST(RigidBody2DTest, MassProperties)
{
    CRigidBody2D rb;

    // Normal mass
    rb.setMass(2.0f);
    EXPECT_FLOAT_EQ(rb.getMass(), 2.0f);
    EXPECT_FLOAT_EQ(rb.getInverseMass(), 0.5f);

    // Zero mass (infinite mass)
    rb.setMass(0.0f);
    EXPECT_FLOAT_EQ(rb.getMass(), 0.0f);
    EXPECT_FLOAT_EQ(rb.getInverseMass(), 0.0f);

    // Negative mass (should clamp to zero)
    rb.setMass(-5.0f);
    EXPECT_FLOAT_EQ(rb.getMass(), 0.0f);
    EXPECT_FLOAT_EQ(rb.getInverseMass(), 0.0f);
}

TEST(RigidBody2DTest, MaterialProperties)
{
    CRigidBody2D rb;

    rb.setRestitution(0.9f);
    EXPECT_FLOAT_EQ(rb.getRestitution(), 0.9f);

    rb.setFriction(0.7f);
    EXPECT_FLOAT_EQ(rb.getFriction(), 0.7f);

    rb.setLinearDrag(0.05f);
    EXPECT_FLOAT_EQ(rb.getLinearDrag(), 0.05f);

    rb.setAngularDrag(0.1f);
    EXPECT_FLOAT_EQ(rb.getAngularDrag(), 0.1f);
}

TEST(RigidBody2DTest, GravitySettings)
{
    CRigidBody2D rb;

    rb.setUseGravity(false);
    EXPECT_FALSE(rb.getUseGravity());

    rb.setGravityScale(2.5f);
    EXPECT_FLOAT_EQ(rb.getGravityScale(), 2.5f);
}

TEST(RigidBody2DTest, KinematicBody)
{
    CRigidBody2D rb;

    rb.setKinematic(true);
    EXPECT_TRUE(rb.isKinematic());

    rb.setKinematic(false);
    EXPECT_FALSE(rb.isKinematic());
}

TEST(RigidBody2DTest, PositionConstraints)
{
    CRigidBody2D rb;

    rb.setFreezePositionX(true);
    EXPECT_TRUE(rb.getFreezePositionX());

    rb.setFreezePositionY(true);
    EXPECT_TRUE(rb.getFreezePositionY());

    rb.setFreezeRotation(true);
    EXPECT_TRUE(rb.getFreezeRotation());
}

// ============================================================================
// Force Management Tests
// ============================================================================

TEST(RigidBody2DTest, ForceAccumulation)
{
    CRigidBody2D rb;

    // Add forces
    rb.addForce(Vec2(10.0f, 20.0f));
    EXPECT_EQ(rb.getAccumulatedForce(), Vec2(10.0f, 20.0f));

    rb.addForce(Vec2(5.0f, -10.0f));
    EXPECT_EQ(rb.getAccumulatedForce(), Vec2(15.0f, 10.0f));

    // Clear forces (should save to total)
    rb.clearForces();
    EXPECT_EQ(rb.getAccumulatedForce(), Vec2(0.0f, 0.0f));
    EXPECT_EQ(rb.getTotalForce(), Vec2(15.0f, 10.0f));

    // Add more forces
    rb.addForce(Vec2(1.0f, 2.0f));
    EXPECT_EQ(rb.getAccumulatedForce(), Vec2(1.0f, 2.0f));
    EXPECT_EQ(rb.getTotalForce(), Vec2(15.0f, 10.0f)); // Total unchanged
}

TEST(RigidBody2DTest, KinematicIgnoresForces)
{
    CRigidBody2D rb;
    rb.setKinematic(true);

    rb.addForce(Vec2(100.0f, 200.0f));
    EXPECT_EQ(rb.getAccumulatedForce(), Vec2(0.0f, 0.0f));

    rb.addImpulse(Vec2(50.0f, 75.0f));
    EXPECT_EQ(rb.getAccumulatedForce(), Vec2(0.0f, 0.0f));
}

TEST(RigidBody2DTest, ImpulseApplication)
{
    CRigidBody2D rb;

    rb.addImpulse(Vec2(25.0f, 50.0f));
    EXPECT_EQ(rb.getAccumulatedForce(), Vec2(25.0f, 50.0f));
}

// ============================================================================
// Serialization Tests
// ============================================================================

TEST(RigidBody2DTest, BasicSerialization)
{
    // Create and configure original component
    CRigidBody2D rb1;
    rb1.setMass(2.5f);
    rb1.setRestitution(0.8f);
    rb1.setFriction(0.4f);
    rb1.setLinearDrag(0.02f);
    rb1.setAngularDrag(0.06f);
    rb1.setUseGravity(false);
    rb1.setGravityScale(1.5f);
    rb1.setKinematic(true);
    rb1.setFreezePositionX(true);
    rb1.setFreezePositionY(false);
    rb1.setFreezeRotation(true);

    // Serialize
    JsonBuilder builder;
    rb1.serialize(builder);
    std::string jsonStr = builder.toString();

    // Deserialize
    JsonValue json(jsonStr);
    CRigidBody2D rb2;
    rb2.deserialize(json);

    // Verify all values match
    EXPECT_FLOAT_EQ(rb2.getMass(), rb1.getMass());
    EXPECT_FLOAT_EQ(rb2.getInverseMass(), rb1.getInverseMass());
    EXPECT_FLOAT_EQ(rb2.getRestitution(), rb1.getRestitution());
    EXPECT_FLOAT_EQ(rb2.getFriction(), rb1.getFriction());
    EXPECT_FLOAT_EQ(rb2.getLinearDrag(), rb1.getLinearDrag());
    EXPECT_FLOAT_EQ(rb2.getAngularDrag(), rb1.getAngularDrag());
    EXPECT_EQ(rb2.getUseGravity(), rb1.getUseGravity());
    EXPECT_FLOAT_EQ(rb2.getGravityScale(), rb1.getGravityScale());
    EXPECT_EQ(rb2.isKinematic(), rb1.isKinematic());
    EXPECT_EQ(rb2.getFreezePositionX(), rb1.getFreezePositionX());
    EXPECT_EQ(rb2.getFreezePositionY(), rb1.getFreezePositionY());
    EXPECT_EQ(rb2.getFreezeRotation(), rb1.getFreezeRotation());
}

TEST(RigidBody2DTest, ZeroMassSerialization)
{
    // Test that zero mass (infinite mass) is properly serialized
    CRigidBody2D rb1;
    rb1.setMass(0.0f);

    // Serialize
    JsonBuilder builder;
    rb1.serialize(builder);
    JsonValue json(builder.toString());

    // Deserialize
    CRigidBody2D rb2;
    rb2.deserialize(json);

    // Verify zero mass and zero inverse mass
    EXPECT_FLOAT_EQ(rb2.getMass(), 0.0f);
    EXPECT_FLOAT_EQ(rb2.getInverseMass(), 0.0f);
}

TEST(RigidBody2DTest, SerializationRoundTrip)
{
    // Create component with various settings
    CRigidBody2D rb1;
    rb1.setMass(5.0f);
    rb1.setRestitution(0.95f);
    rb1.setFriction(0.15f);
    rb1.setLinearDrag(0.03f);
    rb1.setAngularDrag(0.07f);
    rb1.setUseGravity(true);
    rb1.setGravityScale(0.5f);
    rb1.setKinematic(false);
    rb1.setFreezePositionX(false);
    rb1.setFreezePositionY(true);
    rb1.setFreezeRotation(false);

    // First serialization
    JsonBuilder builder1;
    rb1.serialize(builder1);
    JsonValue json1(builder1.toString());

    // First deserialization
    CRigidBody2D rb2;
    rb2.deserialize(json1);

    // Second serialization
    JsonBuilder builder2;
    rb2.serialize(builder2);
    JsonValue json2(builder2.toString());

    // Second deserialization
    CRigidBody2D rb3;
    rb3.deserialize(json2);

    // Verify all values survived the round trip
    EXPECT_FLOAT_EQ(rb3.getMass(), rb1.getMass());
    EXPECT_FLOAT_EQ(rb3.getInverseMass(), rb1.getInverseMass());
    EXPECT_FLOAT_EQ(rb3.getRestitution(), rb1.getRestitution());
    EXPECT_FLOAT_EQ(rb3.getFriction(), rb1.getFriction());
    EXPECT_FLOAT_EQ(rb3.getLinearDrag(), rb1.getLinearDrag());
    EXPECT_FLOAT_EQ(rb3.getAngularDrag(), rb1.getAngularDrag());
    EXPECT_EQ(rb3.getUseGravity(), rb1.getUseGravity());
    EXPECT_FLOAT_EQ(rb3.getGravityScale(), rb1.getGravityScale());
    EXPECT_EQ(rb3.isKinematic(), rb1.isKinematic());
    EXPECT_EQ(rb3.getFreezePositionX(), rb1.getFreezePositionX());
    EXPECT_EQ(rb3.getFreezePositionY(), rb1.getFreezePositionY());
    EXPECT_EQ(rb3.getFreezeRotation(), rb1.getFreezeRotation());
}

// ============================================================================
// Component Interface Tests
// ============================================================================

TEST(RigidBody2DTest, ComponentType)
{
    CRigidBody2D rb;
    EXPECT_EQ(rb.getType(), "RigidBody2D");
}

TEST(RigidBody2DTest, ComponentActive)
{
    CRigidBody2D rb;
    EXPECT_TRUE(rb.isActive());

    rb.setActive(false);
    EXPECT_FALSE(rb.isActive());
}

// ============================================================================
// Drag Tests
// ============================================================================

TEST(RigidBody2DTest, DragProperties)
{
    CRigidBody2D rb;

    // Test default drag values
    EXPECT_FLOAT_EQ(rb.getLinearDrag(), 0.25f);
    EXPECT_FLOAT_EQ(rb.getAngularDrag(), 0.05f);

    // Test setting drag values
    rb.setLinearDrag(0.05f);
    EXPECT_FLOAT_EQ(rb.getLinearDrag(), 0.05f);

    rb.setAngularDrag(0.1f);
    EXPECT_FLOAT_EQ(rb.getAngularDrag(), 0.1f);
}
