#include <gtest/gtest.h>
#include "CPhysicsBody2D.h"
#include "Entity.h"
#include "SEntity.h"
#include "SSerialization.h"

using namespace Components;
using namespace Entity;
using namespace Systems;
using namespace Serialization;

class CPhysicsBody2DTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        SEntity::instance().clear();
    }

    void TearDown() override
    {
        SEntity::instance().clear();
    }
};

//=============================================================================
// Default Value Tests
//=============================================================================

TEST_F(CPhysicsBody2DTest, ComponentCreationAndDefaults)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    ASSERT_NE(body, nullptr);
    EXPECT_EQ(body->getType(), "CPhysicsBody2D");
    EXPECT_EQ(body->getBodyType(), BodyType::Dynamic);
    EXPECT_FLOAT_EQ(body->getDensity(), 1.0f);
    EXPECT_FLOAT_EQ(body->getFriction(), 0.3f);
    EXPECT_FLOAT_EQ(body->getRestitution(), 0.15f);
    EXPECT_FALSE(body->isFixedRotation());
    EXPECT_FLOAT_EQ(body->getLinearDamping(), 0.25f);
    EXPECT_FLOAT_EQ(body->getAngularDamping(), 0.10f);
    EXPECT_FLOAT_EQ(body->getGravityScale(), 1.0f);
    EXPECT_FALSE(body->isInitialized());
}

//=============================================================================
// Setter/Getter Tests
//=============================================================================

TEST_F(CPhysicsBody2DTest, BodyTypeCanBeSet)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    body->setBodyType(BodyType::Static);
    EXPECT_EQ(body->getBodyType(), BodyType::Static);

    body->setBodyType(BodyType::Kinematic);
    EXPECT_EQ(body->getBodyType(), BodyType::Kinematic);

    body->setBodyType(BodyType::Dynamic);
    EXPECT_EQ(body->getBodyType(), BodyType::Dynamic);
}

TEST_F(CPhysicsBody2DTest, DensityCanBeSet)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    body->setDensity(2.5f);
    EXPECT_FLOAT_EQ(body->getDensity(), 2.5f);
}

TEST_F(CPhysicsBody2DTest, FrictionCanBeSet)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    body->setFriction(0.8f);
    EXPECT_FLOAT_EQ(body->getFriction(), 0.8f);
}

TEST_F(CPhysicsBody2DTest, RestitutionCanBeSet)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    body->setRestitution(0.9f);
    EXPECT_FLOAT_EQ(body->getRestitution(), 0.9f);
}

TEST_F(CPhysicsBody2DTest, FixedRotationCanBeToggled)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    EXPECT_FALSE(body->isFixedRotation());

    body->setFixedRotation(true);
    EXPECT_TRUE(body->isFixedRotation());

    body->setFixedRotation(false);
    EXPECT_FALSE(body->isFixedRotation());
}

TEST_F(CPhysicsBody2DTest, LinearDampingCanBeSet)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    body->setLinearDamping(0.5f);
    EXPECT_FLOAT_EQ(body->getLinearDamping(), 0.5f);
}

TEST_F(CPhysicsBody2DTest, AngularDampingCanBeSet)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    body->setAngularDamping(0.75f);
    EXPECT_FLOAT_EQ(body->getAngularDamping(), 0.75f);
}

TEST_F(CPhysicsBody2DTest, GravityScaleCanBeSet)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    body->setGravityScale(0.5f);
    EXPECT_FLOAT_EQ(body->getGravityScale(), 0.5f);

    body->setGravityScale(0.0f);
    EXPECT_FLOAT_EQ(body->getGravityScale(), 0.0f);

    body->setGravityScale(-1.0f);
    EXPECT_FLOAT_EQ(body->getGravityScale(), -1.0f);
}

//=============================================================================
// Serialization Tests
//=============================================================================

TEST_F(CPhysicsBody2DTest, SerializationDynamicBody)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    body->setBodyType(BodyType::Dynamic);
    body->setDensity(2.0f);
    body->setFriction(0.5f);
    body->setRestitution(0.3f);
    body->setFixedRotation(true);
    body->setLinearDamping(0.4f);
    body->setAngularDamping(0.2f);
    body->setGravityScale(0.8f);

    Serialization::JsonBuilder builder;
    body->serialize(builder);
    std::string json = builder.toString();

    EXPECT_TRUE(json.find("\"cPhysicsBody2D\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"bodyType\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"Dynamic\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"density\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"friction\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"restitution\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"fixedRotation\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"linearDamping\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"angularDamping\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"gravityScale\"") != std::string::npos);
}

TEST_F(CPhysicsBody2DTest, SerializationStaticBody)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    body->setBodyType(BodyType::Static);

    Serialization::JsonBuilder builder;
    body->serialize(builder);
    std::string json = builder.toString();

    EXPECT_TRUE(json.find("\"Static\"") != std::string::npos);
}

TEST_F(CPhysicsBody2DTest, SerializationKinematicBody)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    body->setBodyType(BodyType::Kinematic);

    Serialization::JsonBuilder builder;
    body->serialize(builder);
    std::string json = builder.toString();

    EXPECT_TRUE(json.find("\"Kinematic\"") != std::string::npos);
}

TEST_F(CPhysicsBody2DTest, DeserializationDynamicBody)
{
    std::string json = R"({
        "cPhysicsBody2D": {
            "bodyType": "Dynamic",
            "density": 3.0,
            "friction": 0.7,
            "restitution": 0.5,
            "fixedRotation": true,
            "linearDamping": 0.6,
            "angularDamping": 0.3,
            "gravityScale": 0.5
        }
    })";    Serialization::SSerialization::JsonValue value(json);

    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();
    body->deserialize(value);

    EXPECT_EQ(body->getBodyType(), BodyType::Dynamic);
    EXPECT_FLOAT_EQ(body->getDensity(), 3.0f);
    EXPECT_FLOAT_EQ(body->getFriction(), 0.7f);
    EXPECT_FLOAT_EQ(body->getRestitution(), 0.5f);
    EXPECT_TRUE(body->isFixedRotation());
    EXPECT_FLOAT_EQ(body->getLinearDamping(), 0.6f);
    EXPECT_FLOAT_EQ(body->getAngularDamping(), 0.3f);
    EXPECT_FLOAT_EQ(body->getGravityScale(), 0.5f);
}

TEST_F(CPhysicsBody2DTest, DeserializationStaticBody)
{
    std::string json = R"({
        "cPhysicsBody2D": {
            "bodyType": "Static",
            "density": 1.0,
            "friction": 0.3,
            "restitution": 0.15,
            "fixedRotation": false,
            "linearDamping": 0.25,
            "angularDamping": 0.1,
            "gravityScale": 1.0
        }
    })";    Serialization::SSerialization::JsonValue value(json);

    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();
    body->deserialize(value);

    EXPECT_EQ(body->getBodyType(), BodyType::Static);
}

TEST_F(CPhysicsBody2DTest, DeserializationKinematicBody)
{
    std::string json = R"({
        "cPhysicsBody2D": {
            "bodyType": "Kinematic",
            "density": 1.0,
            "friction": 0.3,
            "restitution": 0.15,
            "fixedRotation": false,
            "linearDamping": 0.25,
            "angularDamping": 0.1,
            "gravityScale": 1.0
        }
    })";    Serialization::SSerialization::JsonValue value(json);

    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();
    body->deserialize(value);

    EXPECT_EQ(body->getBodyType(), BodyType::Kinematic);
}

TEST_F(CPhysicsBody2DTest, SerializeDeserializeRoundTripDynamic)
{
    auto  entity1 = SEntity::instance().addEntity("test1");
    auto* body1   = entity1->addComponent<CPhysicsBody2D>();

    body1->setBodyType(BodyType::Dynamic);
    body1->setDensity(2.5f);
    body1->setFriction(0.6f);
    body1->setRestitution(0.4f);
    body1->setFixedRotation(true);
    body1->setLinearDamping(0.35f);
    body1->setAngularDamping(0.15f);
    body1->setGravityScale(0.75f);

    Serialization::JsonBuilder builder;
    body1->serialize(builder);
    std::string json = builder.toString();    Serialization::SSerialization::JsonValue value(json);

    auto  entity2 = SEntity::instance().addEntity("test2");
    auto* body2   = entity2->addComponent<CPhysicsBody2D>();
    body2->deserialize(value);

    EXPECT_EQ(body1->getBodyType(), body2->getBodyType());
    EXPECT_FLOAT_EQ(body1->getDensity(), body2->getDensity());
    EXPECT_FLOAT_EQ(body1->getFriction(), body2->getFriction());
    EXPECT_FLOAT_EQ(body1->getRestitution(), body2->getRestitution());
    EXPECT_EQ(body1->isFixedRotation(), body2->isFixedRotation());
    EXPECT_FLOAT_EQ(body1->getLinearDamping(), body2->getLinearDamping());
    EXPECT_FLOAT_EQ(body1->getAngularDamping(), body2->getAngularDamping());
    EXPECT_FLOAT_EQ(body1->getGravityScale(), body2->getGravityScale());
}

TEST_F(CPhysicsBody2DTest, SerializeDeserializeRoundTripStatic)
{
    auto  entity1 = SEntity::instance().addEntity("test1");
    auto* body1   = entity1->addComponent<CPhysicsBody2D>();

    body1->setBodyType(BodyType::Static);
    body1->setDensity(0.0f);
    body1->setFriction(1.0f);
    body1->setRestitution(0.0f);
    body1->setFixedRotation(false);
    body1->setLinearDamping(0.0f);
    body1->setAngularDamping(0.0f);
    body1->setGravityScale(0.0f);

    Serialization::JsonBuilder builder;
    body1->serialize(builder);
    std::string json = builder.toString();    Serialization::SSerialization::JsonValue value(json);

    auto  entity2 = SEntity::instance().addEntity("test2");
    auto* body2   = entity2->addComponent<CPhysicsBody2D>();
    body2->deserialize(value);

    EXPECT_EQ(body1->getBodyType(), body2->getBodyType());
    EXPECT_FLOAT_EQ(body1->getDensity(), body2->getDensity());
    EXPECT_FLOAT_EQ(body1->getFriction(), body2->getFriction());
    EXPECT_FLOAT_EQ(body1->getRestitution(), body2->getRestitution());
    EXPECT_EQ(body1->isFixedRotation(), body2->isFixedRotation());
    EXPECT_FLOAT_EQ(body1->getLinearDamping(), body2->getLinearDamping());
    EXPECT_FLOAT_EQ(body1->getAngularDamping(), body2->getAngularDamping());
    EXPECT_FLOAT_EQ(body1->getGravityScale(), body2->getGravityScale());
}

TEST_F(CPhysicsBody2DTest, SerializeDeserializeRoundTripKinematic)
{
    auto  entity1 = SEntity::instance().addEntity("test1");
    auto* body1   = entity1->addComponent<CPhysicsBody2D>();

    body1->setBodyType(BodyType::Kinematic);
    body1->setDensity(1.0f);
    body1->setFriction(0.5f);
    body1->setRestitution(0.2f);
    body1->setFixedRotation(true);
    body1->setLinearDamping(0.1f);
    body1->setAngularDamping(0.05f);
    body1->setGravityScale(1.0f);

    Serialization::JsonBuilder builder;
    body1->serialize(builder);
    std::string json = builder.toString();    Serialization::SSerialization::JsonValue value(json);

    auto  entity2 = SEntity::instance().addEntity("test2");
    auto* body2   = entity2->addComponent<CPhysicsBody2D>();
    body2->deserialize(value);

    EXPECT_EQ(body1->getBodyType(), body2->getBodyType());
    EXPECT_FLOAT_EQ(body1->getDensity(), body2->getDensity());
    EXPECT_FLOAT_EQ(body1->getFriction(), body2->getFriction());
    EXPECT_FLOAT_EQ(body1->getRestitution(), body2->getRestitution());
    EXPECT_EQ(body1->isFixedRotation(), body2->isFixedRotation());
    EXPECT_FLOAT_EQ(body1->getLinearDamping(), body2->getLinearDamping());
    EXPECT_FLOAT_EQ(body1->getAngularDamping(), body2->getAngularDamping());
    EXPECT_FLOAT_EQ(body1->getGravityScale(), body2->getGravityScale());
}

TEST_F(CPhysicsBody2DTest, ZeroGravityScale)
{
    auto  entity1 = SEntity::instance().addEntity("test1");
    auto* body1   = entity1->addComponent<CPhysicsBody2D>();

    body1->setGravityScale(0.0f);

    Serialization::JsonBuilder builder;
    body1->serialize(builder);
    std::string json = builder.toString();    Serialization::SSerialization::JsonValue value(json);

    auto  entity2 = SEntity::instance().addEntity("test2");
    auto* body2   = entity2->addComponent<CPhysicsBody2D>();
    body2->deserialize(value);

    EXPECT_FLOAT_EQ(body2->getGravityScale(), 0.0f);
}

TEST_F(CPhysicsBody2DTest, NegativeGravityScale)
{
    auto  entity1 = SEntity::instance().addEntity("test1");
    auto* body1   = entity1->addComponent<CPhysicsBody2D>();

    body1->setGravityScale(-2.0f);

    Serialization::JsonBuilder builder;
    body1->serialize(builder);
    std::string json = builder.toString();    Serialization::SSerialization::JsonValue value(json);

    auto  entity2 = SEntity::instance().addEntity("test2");
    auto* body2   = entity2->addComponent<CPhysicsBody2D>();
    body2->deserialize(value);

    EXPECT_FLOAT_EQ(body2->getGravityScale(), -2.0f);
}

