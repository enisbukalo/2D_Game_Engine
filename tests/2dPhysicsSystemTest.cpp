#include <gtest/gtest.h>
#include "EntityManager.h"
#include "S2DPhysics.h"
#include "components/CGravity.h"
#include "components/CTransform.h"

class PhysicsSystemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Clear the EntityManager before each test
        EntityManager::instance().clear();
    }

    void TearDown() override
    {
        EntityManager::instance().clear();
    }

    // Helper method to create a physics entity
    std::shared_ptr<Entity> createPhysicsEntity(const std::string& tag, const Vec2& position, const Vec2& velocity, const Vec2& gravity)
    {
        auto entity    = EntityManager::instance().addEntity(tag);
        auto transform = entity->addComponent<CTransform>();
        transform->setPosition(position);
        transform->setVelocity(velocity);

        auto gravityComp = entity->addComponent<CGravity>();
        gravityComp->setForce(gravity);

        return entity;
    }
};

TEST_F(PhysicsSystemTest, BasicGravityEffect)
{
    // Create an entity with initial position and gravity
    Vec2 initialPos(0.0f, 0.0f);
    Vec2 initialVel(0.0f, 0.0f);
    Vec2 gravity(0.0f, -9.81f);

    auto entity = createPhysicsEntity("test", initialPos, initialVel, gravity);
    EntityManager::instance().update(0.0f);  // Process pending entities

    // Get the physics system instance
    auto& physics = S2DPhysics::instance();

    // Update physics for 1 second
    float deltaTime = 1.0f;
    physics.update(deltaTime);
    EntityManager::instance().update(deltaTime);

    // Check that gravity affected the velocity
    auto transform = entity->getComponent<CTransform>();
    EXPECT_FLOAT_EQ(transform->getVelocity().y, -9.81f);   // v = v0 + at
    EXPECT_FLOAT_EQ(transform->getPosition().y, -4.905f);  // p = p0 + v0t + (1/2)at^2
}

TEST_F(PhysicsSystemTest, MultipleEntitiesPhysics)
{
    // Create two entities with different initial conditions
    Vec2 pos1(0.0f, 100.0f);
    Vec2 vel1(5.0f, 0.0f);
    Vec2 gravity1(0.0f, -9.81f);

    Vec2 pos2(100.0f, 100.0f);
    Vec2 vel2(-5.0f, 0.0f);
    Vec2 gravity2(0.0f, -9.81f);

    auto entity1 = createPhysicsEntity("test1", pos1, vel1, gravity1);
    auto entity2 = createPhysicsEntity("test2", pos2, vel2, gravity2);
    EntityManager::instance().update(0.0f);  // Process pending entities

    auto& physics = S2DPhysics::instance();

    // Update physics for 1 second
    float deltaTime = 1.0f;
    physics.update(deltaTime);
    EntityManager::instance().update(deltaTime);

    // Check both entities were affected by physics
    auto transform1 = entity1->getComponent<CTransform>();
    auto transform2 = entity2->getComponent<CTransform>();

    // Entity 1: Moving right with gravity
    EXPECT_FLOAT_EQ(transform1->getPosition().x, 5.0f);     // x = x0 + vt
    EXPECT_FLOAT_EQ(transform1->getPosition().y, 95.095f);  // y = y0 + v0t + (1/2)at^2

    // Entity 2: Moving left with gravity
    EXPECT_FLOAT_EQ(transform2->getPosition().x, 95.0f);    // x = x0 + vt
    EXPECT_FLOAT_EQ(transform2->getPosition().y, 95.095f);  // y = y0 + v0t + (1/2)at^2
}

TEST_F(PhysicsSystemTest, ZeroGravity)
{
    // Create an entity with velocity but no gravity
    Vec2 initialPos(0.0f, 0.0f);
    Vec2 initialVel(10.0f, 5.0f);
    Vec2 gravity(0.0f, 0.0f);

    auto entity = createPhysicsEntity("test", initialPos, initialVel, gravity);
    EntityManager::instance().update(0.0f);

    auto& physics = S2DPhysics::instance();

    // Update physics for 1 second
    float deltaTime = 1.0f;
    physics.update(deltaTime);
    EntityManager::instance().update(deltaTime);

    // Check that position changed according to velocity (no acceleration)
    auto transform = entity->getComponent<CTransform>();
    EXPECT_FLOAT_EQ(transform->getPosition().x, 10.0f);  // x = x0 + vt
    EXPECT_FLOAT_EQ(transform->getPosition().y, 5.0f);   // y = y0 + vt
    EXPECT_FLOAT_EQ(transform->getVelocity().x, 10.0f);  // Velocity remains constant
    EXPECT_FLOAT_EQ(transform->getVelocity().y, 5.0f);   // Velocity remains constant
}

TEST_F(PhysicsSystemTest, DisabledGravityComponent)
{
    // Create an entity with gravity
    Vec2 initialPos(0.0f, 100.0f);
    Vec2 initialVel(0.0f, 0.0f);
    Vec2 gravity(0.0f, -9.81f);

    auto entity = createPhysicsEntity("test", initialPos, initialVel, gravity);
    EntityManager::instance().update(0.0f);

    // Disable the gravity component
    auto gravityComp = entity->getComponent<CGravity>();
    gravityComp->setActive(false);

    auto& physics = S2DPhysics::instance();

    // Update physics for 1 second
    float deltaTime = 1.0f;
    physics.update(deltaTime);
    EntityManager::instance().update(deltaTime);

    // Check that gravity had no effect
    auto transform = entity->getComponent<CTransform>();
    EXPECT_FLOAT_EQ(transform->getPosition().y, 100.0f);
    EXPECT_FLOAT_EQ(transform->getVelocity().y, 0.0f);
}

TEST_F(PhysicsSystemTest, CustomGravityValues)
{
    // Test different gravity values
    std::vector<Vec2> gravityValues = {
        Vec2(0.0f, -1.0f),   // Weak gravity
        Vec2(0.0f, -20.0f),  // Strong gravity
        Vec2(5.0f, -5.0f),   // Diagonal gravity
        Vec2(-3.0f, 3.0f)    // Inverse diagonal gravity
    };

    std::vector<std::shared_ptr<Entity>> entities;
    Vec2                                 initialPos(0.0f, 0.0f);
    Vec2                                 initialVel(0.0f, 0.0f);

    // Create entities with different gravity values
    for (size_t i = 0; i < gravityValues.size(); ++i)
    {
        auto entity = createPhysicsEntity("test" + std::to_string(i), initialPos, initialVel, gravityValues[i]);
        entities.push_back(entity);
    }
    EntityManager::instance().update(0.0f);

    auto& physics = S2DPhysics::instance();

    // Update physics for 1 second
    float deltaTime = 1.0f;
    physics.update(deltaTime);
    EntityManager::instance().update(deltaTime);

    // Verify each entity's motion matches its gravity
    for (size_t i = 0; i < entities.size(); ++i)
    {
        auto transform = entities[i]->getComponent<CTransform>();
        EXPECT_FLOAT_EQ(transform->getVelocity().x, gravityValues[i].x);
        EXPECT_FLOAT_EQ(transform->getVelocity().y, gravityValues[i].y);
    }
}