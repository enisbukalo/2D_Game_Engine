#include <gtest/gtest.h>
#include "CTransform.h"
#include "Entity.h"
#include "SEntity.h"
#include "SSerialization.h"

using namespace Components;
using namespace Entity;
using namespace Systems;
using namespace Serialization;

class CTransformTest : public ::testing::Test
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

TEST_F(CTransformTest, ComponentCreationAndDefaults)
{
    auto  entity    = SEntity::instance().addEntity("test");
    auto* transform = entity->addComponent<CTransform>();

    ASSERT_NE(transform, nullptr);
    EXPECT_EQ(transform->getType(), "Transform");

    Vec2 pos = transform->getPosition();
    EXPECT_FLOAT_EQ(pos.x, 0.0f);
    EXPECT_FLOAT_EQ(pos.y, 0.0f);

    Vec2 vel = transform->getVelocity();
    EXPECT_FLOAT_EQ(vel.x, 0.0f);
    EXPECT_FLOAT_EQ(vel.y, 0.0f);

    Vec2 scale = transform->getScale();
    EXPECT_FLOAT_EQ(scale.x, 1.0f);
    EXPECT_FLOAT_EQ(scale.y, 1.0f);

    EXPECT_FLOAT_EQ(transform->getRotation(), 0.0f);
}

//=============================================================================
// Setter/Getter Tests
//=============================================================================

TEST_F(CTransformTest, PositionCanBeSet)
{
    auto  entity    = SEntity::instance().addEntity("test");
    auto* transform = entity->addComponent<CTransform>();

    transform->setPosition(Vec2(10.5f, -20.3f));

    Vec2 pos = transform->getPosition();
    EXPECT_FLOAT_EQ(pos.x, 10.5f);
    EXPECT_FLOAT_EQ(pos.y, -20.3f);
}

TEST_F(CTransformTest, VelocityCanBeSet)
{
    auto  entity    = SEntity::instance().addEntity("test");
    auto* transform = entity->addComponent<CTransform>();

    transform->setVelocity(Vec2(5.0f, -3.5f));

    Vec2 vel = transform->getVelocity();
    EXPECT_FLOAT_EQ(vel.x, 5.0f);
    EXPECT_FLOAT_EQ(vel.y, -3.5f);
}

TEST_F(CTransformTest, ScaleCanBeSet)
{
    auto  entity    = SEntity::instance().addEntity("test");
    auto* transform = entity->addComponent<CTransform>();

    transform->setScale(Vec2(2.0f, 0.5f));

    Vec2 scale = transform->getScale();
    EXPECT_FLOAT_EQ(scale.x, 2.0f);
    EXPECT_FLOAT_EQ(scale.y, 0.5f);
}

TEST_F(CTransformTest, RotationCanBeSet)
{
    auto  entity    = SEntity::instance().addEntity("test");
    auto* transform = entity->addComponent<CTransform>();

    transform->setRotation(3.14159f);

    EXPECT_FLOAT_EQ(transform->getRotation(), 3.14159f);
}

TEST_F(CTransformTest, NegativeRotation)
{
    auto  entity    = SEntity::instance().addEntity("test");
    auto* transform = entity->addComponent<CTransform>();

    transform->setRotation(-1.5708f);

    EXPECT_FLOAT_EQ(transform->getRotation(), -1.5708f);
}

//=============================================================================
// Serialization Tests
//=============================================================================

TEST_F(CTransformTest, Serialization)
{
    auto  entity    = SEntity::instance().addEntity("test");
    auto* transform = entity->addComponent<CTransform>();

    transform->setPosition(Vec2(100.0f, 200.0f));
    transform->setVelocity(Vec2(5.0f, -10.0f));
    transform->setScale(Vec2(2.0f, 3.0f));
    transform->setRotation(1.5708f);

    Serialization::JsonBuilder builder;
    transform->serialize(builder);
    std::string json = builder.toString();

    EXPECT_TRUE(json.find("\"cTransform\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"position\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"velocity\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"scale\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"rotation\"") != std::string::npos);
}

TEST_F(CTransformTest, Deserialization)
{
    std::string json = R"({
        "cTransform": {
            "position": { "x": 50.5, "y": 75.25 },
            "velocity": { "x": 1.5, "y": -2.5 },
            "scale": { "x": 1.5, "y": 2.0 },
            "rotation": 0.785
        }
    })";    Serialization::SSerialization::JsonValue value(json);

    auto  entity    = SEntity::instance().addEntity("test");
    auto* transform = entity->addComponent<CTransform>();
    transform->deserialize(value);

    Vec2 pos = transform->getPosition();
    EXPECT_FLOAT_EQ(pos.x, 50.5f);
    EXPECT_FLOAT_EQ(pos.y, 75.25f);

    Vec2 vel = transform->getVelocity();
    EXPECT_FLOAT_EQ(vel.x, 1.5f);
    EXPECT_FLOAT_EQ(vel.y, -2.5f);

    Vec2 scale = transform->getScale();
    EXPECT_FLOAT_EQ(scale.x, 1.5f);
    EXPECT_FLOAT_EQ(scale.y, 2.0f);

    EXPECT_FLOAT_EQ(transform->getRotation(), 0.785f);
}

TEST_F(CTransformTest, SerializeDeserializeRoundTrip)
{
    auto  entity1    = SEntity::instance().addEntity("test1");
    auto* transform1 = entity1->addComponent<CTransform>();

    transform1->setPosition(Vec2(123.456f, -789.012f));
    transform1->setVelocity(Vec2(10.0f, -5.0f));
    transform1->setScale(Vec2(0.5f, 1.5f));
    transform1->setRotation(2.35619f);

    Serialization::JsonBuilder builder;
    transform1->serialize(builder);
    std::string json = builder.toString();    Serialization::SSerialization::JsonValue value(json);

    auto  entity2    = SEntity::instance().addEntity("test2");
    auto* transform2 = entity2->addComponent<CTransform>();
    transform2->deserialize(value);

    Vec2 pos1 = transform1->getPosition();
    Vec2 pos2 = transform2->getPosition();
    EXPECT_FLOAT_EQ(pos1.x, pos2.x);
    EXPECT_FLOAT_EQ(pos1.y, pos2.y);

    Vec2 vel1 = transform1->getVelocity();
    Vec2 vel2 = transform2->getVelocity();
    EXPECT_FLOAT_EQ(vel1.x, vel2.x);
    EXPECT_FLOAT_EQ(vel1.y, vel2.y);

    Vec2 scale1 = transform1->getScale();
    Vec2 scale2 = transform2->getScale();
    EXPECT_FLOAT_EQ(scale1.x, scale2.x);
    EXPECT_FLOAT_EQ(scale1.y, scale2.y);

    EXPECT_FLOAT_EQ(transform1->getRotation(), transform2->getRotation());
}

TEST_F(CTransformTest, ZeroValuesRoundTrip)
{
    auto  entity1    = SEntity::instance().addEntity("test1");
    auto* transform1 = entity1->addComponent<CTransform>();

    transform1->setPosition(Vec2(0.0f, 0.0f));
    transform1->setVelocity(Vec2(0.0f, 0.0f));
    transform1->setScale(Vec2(0.0f, 0.0f));
    transform1->setRotation(0.0f);

    Serialization::JsonBuilder builder;
    transform1->serialize(builder);
    std::string json = builder.toString();    Serialization::SSerialization::JsonValue value(json);

    auto  entity2    = SEntity::instance().addEntity("test2");
    auto* transform2 = entity2->addComponent<CTransform>();
    transform2->deserialize(value);

    Vec2 pos = transform2->getPosition();
    EXPECT_FLOAT_EQ(pos.x, 0.0f);
    EXPECT_FLOAT_EQ(pos.y, 0.0f);

    Vec2 vel = transform2->getVelocity();
    EXPECT_FLOAT_EQ(vel.x, 0.0f);
    EXPECT_FLOAT_EQ(vel.y, 0.0f);

    Vec2 scale = transform2->getScale();
    EXPECT_FLOAT_EQ(scale.x, 0.0f);
    EXPECT_FLOAT_EQ(scale.y, 0.0f);

    EXPECT_FLOAT_EQ(transform2->getRotation(), 0.0f);
}

TEST_F(CTransformTest, NegativeValuesRoundTrip)
{
    auto  entity1    = SEntity::instance().addEntity("test1");
    auto* transform1 = entity1->addComponent<CTransform>();

    transform1->setPosition(Vec2(-100.0f, -200.0f));
    transform1->setVelocity(Vec2(-50.0f, -25.0f));
    transform1->setScale(Vec2(-1.0f, -2.0f));
    transform1->setRotation(-3.14159f);

    Serialization::JsonBuilder builder;
    transform1->serialize(builder);
    std::string json = builder.toString();    Serialization::SSerialization::JsonValue value(json);

    auto  entity2    = SEntity::instance().addEntity("test2");
    auto* transform2 = entity2->addComponent<CTransform>();
    transform2->deserialize(value);

    Vec2 pos = transform2->getPosition();
    EXPECT_FLOAT_EQ(pos.x, -100.0f);
    EXPECT_FLOAT_EQ(pos.y, -200.0f);

    Vec2 vel = transform2->getVelocity();
    EXPECT_FLOAT_EQ(vel.x, -50.0f);
    EXPECT_FLOAT_EQ(vel.y, -25.0f);

    Vec2 scale = transform2->getScale();
    EXPECT_FLOAT_EQ(scale.x, -1.0f);
    EXPECT_FLOAT_EQ(scale.y, -2.0f);

    EXPECT_FLOAT_EQ(transform2->getRotation(), -3.14159f);
}

TEST_F(CTransformTest, LargeValuesRoundTrip)
{
    auto  entity1    = SEntity::instance().addEntity("test1");
    auto* transform1 = entity1->addComponent<CTransform>();

    transform1->setPosition(Vec2(1000000.0f, 2000000.0f));
    transform1->setVelocity(Vec2(50000.0f, 100000.0f));
    transform1->setScale(Vec2(100.0f, 200.0f));
    transform1->setRotation(6.28318f);

    Serialization::JsonBuilder builder;
    transform1->serialize(builder);
    std::string json = builder.toString();    Serialization::SSerialization::JsonValue value(json);

    auto  entity2    = SEntity::instance().addEntity("test2");
    auto* transform2 = entity2->addComponent<CTransform>();
    transform2->deserialize(value);

    Vec2 pos = transform2->getPosition();
    EXPECT_FLOAT_EQ(pos.x, 1000000.0f);
    EXPECT_FLOAT_EQ(pos.y, 2000000.0f);

    Vec2 vel = transform2->getVelocity();
    EXPECT_FLOAT_EQ(vel.x, 50000.0f);
    EXPECT_FLOAT_EQ(vel.y, 100000.0f);

    Vec2 scale = transform2->getScale();
    EXPECT_FLOAT_EQ(scale.x, 100.0f);
    EXPECT_FLOAT_EQ(scale.y, 200.0f);

    EXPECT_FLOAT_EQ(transform2->getRotation(), 6.28318f);
}

