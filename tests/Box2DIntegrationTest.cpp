#include <gtest/gtest.h>
#include "box2d/box2d.h"
#include "CPhysicsBody2D.h"
#include "CTransform.h"
#include "CCollider2D.h"
#include "EntityManager.h"
#include "SBox2DPhysics.h"
#include "Vec2.h"

// Basic test to verify Box2D is properly integrated and headers are accessible
class Box2DIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // No setup needed for basic integration test
    }

    void TearDown() override {
        // No teardown needed
    }
};

TEST_F(Box2DIntegrationTest, Box2DHeadersAccessible) {
    // Test that we can create a basic Box2D world
    b2Vec2 gravity = {0.0f, -10.0f};
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = gravity;

    b2WorldId worldId = b2CreateWorld(&worldDef);
    ASSERT_TRUE(b2World_IsValid(worldId));

    // Verify gravity was set correctly
    b2Vec2 worldGravity = b2World_GetGravity(worldId);
    EXPECT_FLOAT_EQ(worldGravity.x, 0.0f);
    EXPECT_FLOAT_EQ(worldGravity.y, -10.0f);

    // Clean up
    b2DestroyWorld(worldId);
}

TEST_F(Box2DIntegrationTest, CreateDynamicBody) {
    // Create world
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, -10.0f};
    b2WorldId worldId = b2CreateWorld(&worldDef);

    // Create a dynamic body
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = {0.0f, 10.0f};

    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);
    ASSERT_TRUE(b2Body_IsValid(bodyId));

    // Verify body position
    b2Vec2 position = b2Body_GetPosition(bodyId);
    EXPECT_FLOAT_EQ(position.x, 0.0f);
    EXPECT_FLOAT_EQ(position.y, 10.0f);

    // Verify body type
    b2BodyType type = b2Body_GetType(bodyId);
    EXPECT_EQ(type, b2_dynamicBody);

    // Clean up
    b2DestroyWorld(worldId);
}

TEST_F(Box2DIntegrationTest, CreateCircleShape) {
    // Create world
    b2WorldDef worldDef = b2DefaultWorldDef();
    b2WorldId worldId = b2CreateWorld(&worldDef);

    // Create a body
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    // Create a circle shape
    b2Circle circle = {{0.0f, 0.0f}, 0.5f};

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;

    b2ShapeId shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
    ASSERT_TRUE(b2Shape_IsValid(shapeId));

    // Clean up
    b2DestroyWorld(worldId);
}

TEST_F(Box2DIntegrationTest, CreateBoxShape) {
    // Create world
    b2WorldDef worldDef = b2DefaultWorldDef();
    b2WorldId worldId = b2CreateWorld(&worldDef);

    // Create a body
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    // Create a box shape
    b2Polygon box = b2MakeBox(1.0f, 1.0f);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;

    b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &box);
    ASSERT_TRUE(b2Shape_IsValid(shapeId));

    // Clean up
    b2DestroyWorld(worldId);
}

TEST_F(Box2DIntegrationTest, PhysicsSimulation) {
    // Create world with gravity
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, -10.0f};
    b2WorldId worldId = b2CreateWorld(&worldDef);

    // Create a dynamic body with a circle shape
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = {0.0f, 10.0f};
    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    b2Circle circle = {{0.0f, 0.0f}, 0.5f};

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    b2CreateCircleShape(bodyId, &shapeDef, &circle);

    // Get initial position
    b2Vec2 initialPos = b2Body_GetPosition(bodyId);

    // Step the simulation
    float timeStep = 1.0f / 60.0f;
    int subStepCount = 4;

    for (int i = 0; i < 60; i++) {
        b2World_Step(worldId, timeStep, subStepCount);
    }

    // Body should have fallen due to gravity
    b2Vec2 finalPos = b2Body_GetPosition(bodyId);
    EXPECT_LT(finalPos.y, initialPos.y);

    // Clean up
    b2DestroyWorld(worldId);
}

// Verify that fixed rotation keeps angular velocity zero
TEST_F(Box2DIntegrationTest, FixedRotation_ZeroesAngularVelocity)
{
    auto& manager = EntityManager::instance();

    auto entity = manager.addEntity("fixed_body_test");
    ASSERT_NE(entity, nullptr);

    auto transform = entity->addComponent<CTransform>();
    ASSERT_NE(transform, nullptr);
    transform->setPosition(Vec2(0.0f, 0.0f));

    auto physicsBody = entity->addComponent<CPhysicsBody2D>();
    ASSERT_NE(physicsBody, nullptr);
    physicsBody->initialize({0.0f, 0.0f}, BodyType::Dynamic);
    physicsBody->setBodyType(BodyType::Dynamic);
    physicsBody->setDensity(1.0f);
    physicsBody->setFixedRotation(false);

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);
    collider->createCircle(0.5f);

    // Give the body some angular impulse then step the world and confirm non-zero
    physicsBody->applyAngularImpulse(5.0f);
    SBox2DPhysics::instance().update(1.0f / 60.0f);
    float omegaBefore = physicsBody->getAngularVelocity();
    EXPECT_NE(omegaBefore, 0.0f);

    // Enable fixed rotation: it should zero the angular velocity immediately
    physicsBody->setFixedRotation(true);
    float omegaAfterSet = physicsBody->getAngularVelocity();
    EXPECT_EQ(omegaAfterSet, 0.0f);

    // Attempt to apply additional angular impulse (should be ignored), then step world
    physicsBody->applyAngularImpulse(10.0f);
    SBox2DPhysics::instance().update(1.0f / 60.0f);
    float omegaAfterUpdate = physicsBody->getAngularVelocity();
    EXPECT_EQ(omegaAfterUpdate, 0.0f);
}
