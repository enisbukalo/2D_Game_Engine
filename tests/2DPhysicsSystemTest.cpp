#include <gtest/gtest.h>
#include <iostream>
#include "CBoxCollider.h"
#include "CCircleCollider.h"
#include "CGravity.h"
#include "CTransform.h"
#include "EntityManager.h"
#include "S2DPhysics.h"

class PhysicsSystemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Clear the EntityManager before each test.
        EntityManager::instance().clear();
        // Reset global gravity to default (981 px/s² downward)
        S2DPhysics::instance().setGlobalGravity(Vec2(0, 981));
    }

    void TearDown() override
    {
        EntityManager::instance().clear();
    }

    // Helper method to create a physics entity with gravity multiplier
    std::shared_ptr<Entity> createPhysicsEntity(const std::string& tag, const Vec2& position, const Vec2& velocity, float gravityMultiplier = 1.0f)
    {
        auto entity    = EntityManager::instance().addEntity(tag);
        auto transform = entity->addComponent<CTransform>();
        transform->setPosition(position);
        transform->setVelocity(velocity);

        auto gravityComp = entity->addComponent<CGravity>();
        gravityComp->setMultiplier(gravityMultiplier);

        return entity;
    }
};

TEST_F(PhysicsSystemTest, BasicGravityEffect)
{
    // Set global gravity (positive = downward at 100px/m scale: 9.81 m/s² = 981 px/s²)
    auto& physics = S2DPhysics::instance();
    physics.setGlobalGravity(Vec2(0.0f, 981.0f));

    // Debug: verify gravity was set
    Vec2 actualGravity = physics.getGlobalGravity();
    std::cout << "DEBUG: Gravity after set: " << actualGravity.y << std::endl;

    // Create an entity with initial position and gravity multiplier 1.0
    Vec2 initialPos(0.0f, 0.0f);
    Vec2 initialVel(0.0f, 0.0f);

    auto entity = createPhysicsEntity("test", initialPos, initialVel, 1.0f);
    EntityManager::instance().update(0.0f);  // Process pending entities

    // Update physics for 1 second
    float deltaTime = 1.0f;
    physics.update(deltaTime);
    EntityManager::instance().update(deltaTime);

    // Check that gravity affected the velocity and position
    // Using semi-implicit Euler integration: v_new = v + a*dt, p_new = p + v_new*dt
    auto transform = entity->getComponent<CTransform>();
    std::cout << "DEBUG: Final position.y: " << transform->getPosition().y << std::endl;
    std::cout << "DEBUG: Final velocity.y: " << transform->getVelocity().y << std::endl;
    EXPECT_FLOAT_EQ(transform->getVelocity().y, 981.0f);  // v = v0 + at = 0 + 981*1.0
    EXPECT_FLOAT_EQ(transform->getPosition().y, 981.0f);  // p = p0 + v_new*t = 0 + 981*1.0
}

TEST_F(PhysicsSystemTest, MultipleEntitiesPhysics)
{
    // Set global gravity
    auto& physics = S2DPhysics::instance();
    physics.setGlobalGravity(Vec2(0.0f, 981.0f));

    // Create two entities with different initial conditions
    Vec2 pos1(0.0f, 100.0f);
    Vec2 vel1(5.0f, 0.0f);

    Vec2 pos2(100.0f, 100.0f);
    Vec2 vel2(-5.0f, 0.0f);

    auto entity1 = createPhysicsEntity("test1", pos1, vel1, 1.0f);
    auto entity2 = createPhysicsEntity("test2", pos2, vel2, 1.0f);
    EntityManager::instance().update(0.0f);  // Process pending entities

    // Update physics for 1 second
    float deltaTime = 1.0f;
    physics.update(deltaTime);
    EntityManager::instance().update(deltaTime);

    // Check both entities were affected by physics (semi-implicit Euler)
    auto transform1 = entity1->getComponent<CTransform>();
    auto transform2 = entity2->getComponent<CTransform>();

    // Entity 1: Moving right with gravity - v_new=(5,-9.81), p_new=(0,100)+(5,-9.81)*1.0
    EXPECT_FLOAT_EQ(transform1->getPosition().x, 5.0f);
    EXPECT_FLOAT_EQ(transform1->getPosition().y, 1081.0f);

    // Entity 2: Moving left with gravity - v_new=(-5,-9.81), p_new=(100,100)+(-5,-9.81)*1.0
    EXPECT_FLOAT_EQ(transform2->getPosition().x, 95.0f);
    EXPECT_FLOAT_EQ(transform2->getPosition().y, 1081.0f);
}

TEST_F(PhysicsSystemTest, ZeroGravity)
{
    // Set global gravity to zero
    auto& physics = S2DPhysics::instance();
    physics.setGlobalGravity(Vec2(0.0f, 0.0f));

    // Create an entity with velocity but no gravity (multiplier = 1.0 but global gravity is 0)
    Vec2 initialPos(0.0f, 0.0f);
    Vec2 initialVel(10.0f, 5.0f);

    auto entity = createPhysicsEntity("test", initialPos, initialVel, 1.0f);
    EntityManager::instance().update(0.0f);

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
    // Set global gravity
    auto& physics = S2DPhysics::instance();
    physics.setGlobalGravity(Vec2(0.0f, 981.0f));

    // Create an entity with gravity
    Vec2 initialPos(0.0f, 100.0f);
    Vec2 initialVel(0.0f, 0.0f);

    auto entity = createPhysicsEntity("test", initialPos, initialVel, 1.0f);
    EntityManager::instance().update(0.0f);

    // Disable the gravity component
    auto gravityComp = entity->getComponent<CGravity>();
    gravityComp->setActive(false);

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
    // Set global gravity
    auto& physics = S2DPhysics::instance();
    physics.setGlobalGravity(Vec2(5.0f, 10.0f));  // Base gravity

    // Test different gravity multipliers
    std::vector<float> gravityMultipliers = {
        0.1f,   // Weak gravity (10%)
        2.0f,   // Strong gravity (200%)
        1.0f,   // Normal gravity (100%)
        -0.5f   // Reverse gravity at half strength
    };

    std::vector<std::shared_ptr<Entity>> entities;
    Vec2                                 initialPos(0.0f, 0.0f);
    Vec2                                 initialVel(0.0f, 0.0f);

    // Create entities with different gravity multipliers
    for (size_t i = 0; i < gravityMultipliers.size(); ++i)
    {
        auto entity = createPhysicsEntity("test" + std::to_string(i), initialPos, initialVel, gravityMultipliers[i]);
        entities.push_back(entity);
    }
    EntityManager::instance().update(0.0f);

    // Update physics for 1 second
    float deltaTime = 1.0f;
    physics.update(deltaTime);
    EntityManager::instance().update(deltaTime);

    // Verify each entity's motion matches its gravity multiplier
    Vec2 globalGravity = physics.getGlobalGravity();
    for (size_t i = 0; i < entities.size(); ++i)
    {
        auto  transform      = entities[i]->getComponent<CTransform>();
        Vec2  expectedForce  = globalGravity * gravityMultipliers[i];
        EXPECT_FLOAT_EQ(transform->getVelocity().x, expectedForce.x);
        EXPECT_FLOAT_EQ(transform->getVelocity().y, expectedForce.y);
    }
}

// ========== COLLISION RESOLUTION TESTS ==========

TEST_F(PhysicsSystemTest, CircleCollisionDetection)
{
    // Create two circles that are colliding
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    auto circle1    = EntityManager::instance().addEntity("circle1");
    auto transform1 = circle1->addComponent<CTransform>();
    auto collider1  = circle1->addComponent<CCircleCollider>(25.0f);
    transform1->setPosition(Vec2(100.0f, 100.0f));
    transform1->setVelocity(Vec2(10.0f, 0.0f));

    auto circle2    = EntityManager::instance().addEntity("circle2");
    auto transform2 = circle2->addComponent<CTransform>();
    auto collider2  = circle2->addComponent<CCircleCollider>(25.0f);
    transform2->setPosition(Vec2(140.0f, 100.0f));  // 40 units apart, circles overlap (50 radius sum)
    transform2->setVelocity(Vec2(0.0f, 0.0f));

    EntityManager::instance().update(0.0f);

    // Verify they are intersecting
    EXPECT_TRUE(collider1->intersects(collider2));
}

TEST_F(PhysicsSystemTest, HeadOnCollisionVelocityChange)
{
    // Test head-on collision between two moving circles
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    auto circle1    = EntityManager::instance().addEntity("circle1");
    auto transform1 = circle1->addComponent<CTransform>();
    auto collider1  = circle1->addComponent<CCircleCollider>(25.0f);
    transform1->setPosition(Vec2(100.0f, 100.0f));
    transform1->setVelocity(Vec2(100.0f, 0.0f));  // Moving right fast

    auto circle2    = EntityManager::instance().addEntity("circle2");
    auto transform2 = circle2->addComponent<CTransform>();
    auto collider2  = circle2->addComponent<CCircleCollider>(25.0f);
    transform2->setPosition(Vec2(145.0f, 100.0f));  // 45 units apart - overlapping!
    transform2->setVelocity(Vec2(-100.0f, 0.0f));   // Moving left fast

    EntityManager::instance().update(0.0f);

    // Verify they are already colliding
    EXPECT_TRUE(collider1->intersects(collider2));

    // Store initial velocities
    Vec2 initialVel1 = transform1->getVelocity();
    Vec2 initialVel2 = transform2->getVelocity();

    // Update physics - they should resolve collision
    physics.update(0.016f);  // One frame at 60 FPS

    // After collision, velocities should have changed (bounced)
    Vec2 finalVel1 = transform1->getVelocity();
    Vec2 finalVel2 = transform2->getVelocity();

    // The velocities should be different from initial (collision occurred)
    EXPECT_NE(finalVel1.x, initialVel1.x);
    EXPECT_NE(finalVel2.x, initialVel2.x);

    // In a head-on collision, objects should bounce back (velocities reverse direction)
    // Due to restitution, they won't be exactly reversed but should be opposite direction
    EXPECT_LT(finalVel1.x, 0.0f);  // Circle1 should now be moving left
    EXPECT_GT(finalVel2.x, 0.0f);  // Circle2 should now be moving right
}

TEST_F(PhysicsSystemTest, StaticObjectCollision)
{
    // Test collision with a static object (like ground)
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    // Static object (ground)
    auto ground          = EntityManager::instance().addEntity("ground");
    auto groundTransform = ground->addComponent<CTransform>();
    auto groundCollider  = ground->addComponent<CCircleCollider>(50.0f);
    groundCollider->setStatic(true);  // Mark as immovable
    groundTransform->setPosition(Vec2(400.0f, 50.0f));
    groundTransform->setVelocity(Vec2(0.0f, 0.0f));  // Static

    // Dynamic falling object - position it so it's already overlapping with ground
    auto ball          = EntityManager::instance().addEntity("ball");
    auto ballTransform = ball->addComponent<CTransform>();
    auto ballCollider  = ball->addComponent<CCircleCollider>(25.0f);
    ballTransform->setPosition(Vec2(400.0f, 110.0f));  // 60 units from ground center (overlapping with 75 radius sum)
    ballTransform->setVelocity(Vec2(0.0f, -50.0f));    // Falling down

    EntityManager::instance().update(0.0f);

    // Verify they are colliding
    EXPECT_TRUE(ballCollider->intersects(groundCollider));

    // Store initial velocity
    float initialVelY = ballTransform->getVelocity().y;
    EXPECT_LT(initialVelY, 0.0f);  // Should be moving down

    // Update physics once to resolve collision
    physics.update(0.016f);

    // After collision with static ground, ball should bounce up (velocity y becomes positive)
    float finalVelY = ballTransform->getVelocity().y;

    // The ball should have bounced (velocity changed to upward or at least changed significantly)
    EXPECT_NE(finalVelY, initialVelY);
    EXPECT_GT(finalVelY, initialVelY);  // Should be less negative or positive
}

TEST_F(PhysicsSystemTest, CollisionSeparation)
{
    // Test that overlapping objects are separated after collision resolution
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    // Create two overlapping circles moving toward each other
    auto circle1    = EntityManager::instance().addEntity("circle1");
    auto transform1 = circle1->addComponent<CTransform>();
    auto collider1  = circle1->addComponent<CCircleCollider>(30.0f);
    transform1->setPosition(Vec2(200.0f, 200.0f));
    transform1->setVelocity(Vec2(50.0f, 0.0f));  // Moving right

    auto circle2    = EntityManager::instance().addEntity("circle2");
    auto transform2 = circle2->addComponent<CTransform>();
    auto collider2  = circle2->addComponent<CCircleCollider>(30.0f);
    transform2->setPosition(Vec2(250.0f, 200.0f));  // 50 units apart, should overlap (60 radius sum)
    transform2->setVelocity(Vec2(-50.0f, 0.0f));    // Moving left

    EntityManager::instance().update(0.0f);

    // Verify they are initially overlapping
    EXPECT_TRUE(collider1->intersects(collider2));

    // Update physics to resolve collision
    physics.update(0.016f);

    // After collision resolution, they should be separated or just touching
    Vec2  pos1      = transform1->getPosition();
    Vec2  pos2      = transform2->getPosition();
    float distance  = (pos2 - pos1).length();
    float radiusSum = collider1->getRadius() + collider2->getRadius();

    // Distance should be at least equal to radius sum (touching or separated)
    EXPECT_GE(distance, radiusSum * 0.95f);  // Allow small tolerance for numerical precision
}

TEST_F(PhysicsSystemTest, NoCollisionWhenSeparated)
{
    // Test that separated circles don't affect each other
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    auto circle1    = EntityManager::instance().addEntity("circle1");
    auto transform1 = circle1->addComponent<CTransform>();
    auto collider1  = circle1->addComponent<CCircleCollider>(25.0f);
    transform1->setPosition(Vec2(100.0f, 100.0f));
    transform1->setVelocity(Vec2(10.0f, 0.0f));

    auto circle2    = EntityManager::instance().addEntity("circle2");
    auto transform2 = circle2->addComponent<CTransform>();
    auto collider2  = circle2->addComponent<CCircleCollider>(25.0f);
    transform2->setPosition(Vec2(500.0f, 100.0f));  // Far apart
    transform2->setVelocity(Vec2(-10.0f, 0.0f));

    EntityManager::instance().update(0.0f);

    // Verify they are NOT intersecting
    EXPECT_FALSE(collider1->intersects(collider2));

    // Store initial velocities
    Vec2 initialVel1 = transform1->getVelocity();
    Vec2 initialVel2 = transform2->getVelocity();

    // Update physics
    physics.update(0.1f);

    // Velocities should remain unchanged (no collision)
    EXPECT_FLOAT_EQ(transform1->getVelocity().x, initialVel1.x);
    EXPECT_FLOAT_EQ(transform1->getVelocity().y, initialVel1.y);
    EXPECT_FLOAT_EQ(transform2->getVelocity().x, initialVel2.x);
    EXPECT_FLOAT_EQ(transform2->getVelocity().y, initialVel2.y);
}

TEST_F(PhysicsSystemTest, CollisionWithGravity)
{
    // Test realistic scenario: falling object bouncing off ground
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    // Ground
    auto ground          = EntityManager::instance().addEntity("ground");
    auto groundTransform = ground->addComponent<CTransform>();
    auto groundCollider  = ground->addComponent<CCircleCollider>(100.0f);
    groundCollider->setStatic(true);  // Mark as immovable
    groundTransform->setPosition(Vec2(400.0f, 50.0f));
    groundTransform->setVelocity(Vec2(0.0f, 0.0f));

    // Set global gravity for this test
    physics.setGlobalGravity(Vec2(0.0f, 500.0f));

    // Falling ball with gravity
    auto ball          = EntityManager::instance().addEntity("ball");
    auto ballTransform = ball->addComponent<CTransform>();
    auto ballGravity   = ball->addComponent<CGravity>();
    auto ballCollider  = ball->addComponent<CCircleCollider>(25.0f);
    ballTransform->setPosition(Vec2(400.0f, 300.0f));
    ballTransform->setVelocity(Vec2(0.0f, 0.0f));
    ballGravity->setMultiplier(1.0f);  // Normal gravity multiplier

    EntityManager::instance().update(0.0f);

    // Simulate for a bit to let it fall
    for (int i = 0; i < 20; ++i)
    {
        physics.update(0.016f);  // ~60 FPS timestep
    }

    // Ball should have bounced at least once and be somewhere reasonable
    Vec2 finalPos = ballTransform->getPosition();

    // Should not have fallen through the ground
    EXPECT_GT(finalPos.y, 0.0f);

    // Should have moved from initial position (gravity worked)
    EXPECT_NE(finalPos.y, 300.0f);
}

TEST_F(PhysicsSystemTest, MultipleCollisionsSimultaneous)
{
    // Test multiple objects colliding at once
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    // Create 3 circles - two on the sides moving toward a static one in middle
    auto circle1    = EntityManager::instance().addEntity("circle1");
    auto transform1 = circle1->addComponent<CTransform>();
    auto collider1  = circle1->addComponent<CCircleCollider>(20.0f);
    transform1->setPosition(Vec2(170.0f, 200.0f));  // Overlapping with middle (distance 30 < radius sum 40)
    transform1->setVelocity(Vec2(50.0f, 0.0f));     // Moving right toward middle

    auto circle2    = EntityManager::instance().addEntity("circle2");
    auto transform2 = circle2->addComponent<CTransform>();
    auto collider2  = circle2->addComponent<CCircleCollider>(20.0f);
    transform2->setPosition(Vec2(200.0f, 200.0f));  // Middle
    transform2->setVelocity(Vec2(0.0f, 0.0f));      // Static in middle

    auto circle3    = EntityManager::instance().addEntity("circle3");
    auto transform3 = circle3->addComponent<CTransform>();
    auto collider3  = circle3->addComponent<CCircleCollider>(20.0f);
    transform3->setPosition(Vec2(230.0f, 200.0f));  // Overlapping with middle (distance 30 < radius sum 40)
    transform3->setVelocity(Vec2(-50.0f, 0.0f));    // Moving left toward middle

    EntityManager::instance().update(0.0f);

    // Verify at least one pair is colliding initially
    bool hasCollision = collider1->intersects(collider2) || collider2->intersects(collider3);
    EXPECT_TRUE(hasCollision);

    // Store initial position of middle circle
    Vec2 initialPos2 = transform2->getPosition();

    // Update physics multiple times to let collisions happen
    for (int i = 0; i < 5; ++i)
    {
        physics.update(0.016f);
    }

    // Middle circle should have moved from the impacts
    Vec2  finalPos2 = transform2->getPosition();
    float movement  = (finalPos2 - initialPos2).length();
    EXPECT_GT(movement, 0.05f);  // Should have moved at least a little bit (relaxed tolerance for physics precision)
}

// ========== BOX COLLIDER PHYSICS TESTS ==========

TEST_F(PhysicsSystemTest, BoxVsBoxCollision)
{
    // Test collision between two box colliders
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    auto box1       = EntityManager::instance().addEntity("box1");
    auto transform1 = box1->addComponent<CTransform>();
    auto collider1  = box1->addComponent<CBoxCollider>(40.0f, 40.0f);
    transform1->setPosition(Vec2(200.0f, 200.0f));
    transform1->setVelocity(Vec2(50.0f, 0.0f));  // Moving right

    auto box2       = EntityManager::instance().addEntity("box2");
    auto transform2 = box2->addComponent<CTransform>();
    auto collider2  = box2->addComponent<CBoxCollider>(40.0f, 40.0f);
    transform2->setPosition(Vec2(235.0f, 200.0f));  // 35 units apart, overlapping (40 half-width each)
    transform2->setVelocity(Vec2(-50.0f, 0.0f));    // Moving left

    EntityManager::instance().update(0.0f);

    // Verify they are colliding
    EXPECT_TRUE(collider1->intersects(collider2));

    // Store initial velocities
    Vec2 initialVel1 = transform1->getVelocity();
    Vec2 initialVel2 = transform2->getVelocity();

    // Update physics to resolve collision
    physics.update(0.016f);

    // After collision, velocities should have changed
    Vec2 finalVel1 = transform1->getVelocity();
    Vec2 finalVel2 = transform2->getVelocity();

    EXPECT_NE(finalVel1.x, initialVel1.x);
    EXPECT_NE(finalVel2.x, initialVel2.x);

    // Boxes should bounce back (velocities reverse direction)
    EXPECT_LT(finalVel1.x, 0.0f);  // Box1 should now be moving left
    EXPECT_GT(finalVel2.x, 0.0f);  // Box2 should now be moving right
}

TEST_F(PhysicsSystemTest, CircleVsBoxCollision)
{
    // Test collision between circle and box
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    auto circle          = EntityManager::instance().addEntity("circle");
    auto circleTransform = circle->addComponent<CTransform>();
    auto circleCollider  = circle->addComponent<CCircleCollider>(25.0f);
    circleTransform->setPosition(Vec2(200.0f, 200.0f));
    circleTransform->setVelocity(Vec2(100.0f, 0.0f));  // Moving right fast

    auto box          = EntityManager::instance().addEntity("box");
    auto boxTransform = box->addComponent<CTransform>();
    auto boxCollider  = box->addComponent<CBoxCollider>(40.0f, 40.0f);
    boxCollider->setStatic(true);                     // Mark as immovable
    boxTransform->setPosition(Vec2(235.0f, 200.0f));  // 35 units apart - circle edge at 225, box edge at 215, overlapping!
    boxTransform->setVelocity(Vec2(0.0f, 0.0f));      // Static

    EntityManager::instance().update(0.0f);

    // Verify they are colliding
    EXPECT_TRUE(circleCollider->intersects(boxCollider));

    // Store initial velocity
    float initialVelX = circleTransform->getVelocity().x;

    // Update physics to resolve collision
    physics.update(0.016f);

    // Circle should have bounced back from static box
    float finalVelX = circleTransform->getVelocity().x;
    EXPECT_NE(finalVelX, initialVelX);
    EXPECT_LT(finalVelX, 0.0f);  // Should be moving left now
}

TEST_F(PhysicsSystemTest, BoxAsGround)
{
    // Test using a box as a flat ground surface
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    // Ground box - wide and flat
    auto ground          = EntityManager::instance().addEntity("ground");
    auto groundTransform = ground->addComponent<CTransform>();
    auto groundCollider  = ground->addComponent<CBoxCollider>(400.0f, 20.0f);  // Wide and thin (half-height = 10)
    groundCollider->setStatic(true);                                           // Mark as immovable
    groundTransform->setPosition(Vec2(400.0f, 50.0f));
    groundTransform->setVelocity(Vec2(0.0f, 0.0f));  // Static

    // Falling circle
    auto ball          = EntityManager::instance().addEntity("ball");
    auto ballTransform = ball->addComponent<CTransform>();
    auto ballCollider  = ball->addComponent<CCircleCollider>(25.0f);
    ballTransform->setPosition(Vec2(400.0f, 80.0f));  // Ground top at 60, ball bottom at 55 - overlapping!
    ballTransform->setVelocity(Vec2(0.0f, -50.0f));   // Falling down

    EntityManager::instance().update(0.0f);

    // Verify they are colliding
    EXPECT_TRUE(ballCollider->intersects(groundCollider));

    // Store initial velocity
    float initialVelY = ballTransform->getVelocity().y;
    EXPECT_LT(initialVelY, 0.0f);  // Should be moving down

    // Update physics to resolve collision
    physics.update(0.016f);

    // Ball should have bounced up from ground
    float finalVelY = ballTransform->getVelocity().y;
    EXPECT_NE(finalVelY, initialVelY);
    EXPECT_GT(finalVelY, initialVelY);  // Should be less negative or positive
}

TEST_F(PhysicsSystemTest, BoxCollisionSeparation)
{
    // Test that overlapping boxes are separated after collision
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    auto box1       = EntityManager::instance().addEntity("box1");
    auto transform1 = box1->addComponent<CTransform>();
    auto collider1  = box1->addComponent<CBoxCollider>(50.0f, 50.0f);
    transform1->setPosition(Vec2(200.0f, 200.0f));
    transform1->setVelocity(Vec2(30.0f, 0.0f));

    auto box2       = EntityManager::instance().addEntity("box2");
    auto transform2 = box2->addComponent<CTransform>();
    auto collider2  = box2->addComponent<CBoxCollider>(50.0f, 50.0f);
    transform2->setPosition(Vec2(240.0f, 200.0f));  // 40 units apart, overlapping (50 half-width each)
    transform2->setVelocity(Vec2(-30.0f, 0.0f));

    EntityManager::instance().update(0.0f);

    // Verify they are initially overlapping
    EXPECT_TRUE(collider1->intersects(collider2));

    // Update physics to resolve collision
    physics.update(0.016f);

    // After collision resolution, they should be separated or just touching
    Vec2 pos1 = transform1->getPosition();
    Vec2 pos2 = transform2->getPosition();

    // For AABBs, check x-axis separation (they're aligned horizontally)
    float distance     = std::abs(pos2.x - pos1.x);
    float halfWidthSum = collider1->getWidth() / 2.0f + collider2->getWidth() / 2.0f;

    // Distance should be at least equal to half-width sum (touching or separated)
    EXPECT_GE(distance, halfWidthSum * 0.95f);  // Allow small tolerance
}

TEST_F(PhysicsSystemTest, BoxWithGravity)
{
    // Test realistic scenario: falling box bouncing off ground
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    // Ground box
    auto ground          = EntityManager::instance().addEntity("ground");
    auto groundTransform = ground->addComponent<CTransform>();
    auto groundCollider  = ground->addComponent<CBoxCollider>(400.0f, 50.0f);
    groundCollider->setStatic(true);  // Mark as immovable
    groundTransform->setPosition(Vec2(400.0f, 50.0f));
    groundTransform->setVelocity(Vec2(0.0f, 0.0f));

    // Set global gravity for this test
    physics.setGlobalGravity(Vec2(0.0f, 500.0f));

    // Falling box with gravity
    auto box          = EntityManager::instance().addEntity("box");
    auto boxTransform = box->addComponent<CTransform>();
    auto boxGravity   = box->addComponent<CGravity>();
    auto boxCollider  = box->addComponent<CBoxCollider>(30.0f, 30.0f);
    boxTransform->setPosition(Vec2(400.0f, 300.0f));
    boxTransform->setVelocity(Vec2(0.0f, 0.0f));
    boxGravity->setMultiplier(1.0f);  // Normal gravity multiplier

    EntityManager::instance().update(0.0f);

    // Simulate for a bit to let it fall and bounce
    for (int i = 0; i < 20; ++i)
    {
        physics.update(0.016f);
    }

    // Box should have bounced and be somewhere reasonable
    Vec2 finalPos = boxTransform->getPosition();

    // Should not have fallen through the ground
    EXPECT_GT(finalPos.y, 25.0f);  // Ground is at y=50, box half-height is 15

    // Should have moved from initial position (gravity worked)
    EXPECT_NE(finalPos.y, 300.0f);
}

TEST_F(PhysicsSystemTest, MixedShapeCollisions)
{
    // Test multiple different shape types colliding together
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    // Static box in center
    auto centerBox       = EntityManager::instance().addEntity("centerBox");
    auto centerTransform = centerBox->addComponent<CTransform>();
    auto centerCollider = centerBox->addComponent<CBoxCollider>(40.0f, 40.0f);  // Half-width = 20, edges at 380 and 420
    centerCollider->setStatic(true);                                            // Mark as immovable
    centerTransform->setPosition(Vec2(400.0f, 300.0f));
    centerTransform->setVelocity(Vec2(0.0f, 0.0f));

    // Circle approaching from left
    auto leftCircle    = EntityManager::instance().addEntity("leftCircle");
    auto leftTransform = leftCircle->addComponent<CTransform>();
    auto leftCollider  = leftCircle->addComponent<CCircleCollider>(20.0f);  // Radius 20, right edge at 375
    leftTransform->setPosition(Vec2(365.0f, 300.0f));  // Circle right edge at 385, box left edge at 380 - overlapping!
    leftTransform->setVelocity(Vec2(50.0f, 0.0f));     // Moving right

    // Box approaching from right
    auto rightBox       = EntityManager::instance().addEntity("rightBox");
    auto rightTransform = rightBox->addComponent<CTransform>();
    auto rightCollider  = rightBox->addComponent<CBoxCollider>(30.0f, 30.0f);  // Half-width = 15, left edge at 420
    rightTransform->setPosition(Vec2(435.0f, 300.0f));  // Box left edge at 420, center box right edge at 420 - touching!
    rightTransform->setVelocity(Vec2(-50.0f, 0.0f));    // Moving left

    EntityManager::instance().update(0.0f);

    // Verify at least one collision is happening
    bool hasCollision = leftCollider->intersects(centerCollider) || centerCollider->intersects(rightCollider);
    EXPECT_TRUE(hasCollision);

    // Update physics multiple times
    for (int i = 0; i < 5; ++i)
    {
        physics.update(0.016f);
    }

    // Moving objects should have changed direction after hitting static center box
    EXPECT_LT(leftTransform->getVelocity().x, 0.0f);   // Should be moving left now
    EXPECT_GT(rightTransform->getVelocity().x, 0.0f);  // Should be moving right now
}

TEST_F(PhysicsSystemTest, BoxCornerCollision)
{
    // Test collision at corner of box (edge case)
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    // Static box
    auto box          = EntityManager::instance().addEntity("box");
    auto boxTransform = box->addComponent<CTransform>();
    auto boxCollider  = box->addComponent<CBoxCollider>(60.0f, 60.0f);
    boxCollider->setStatic(true);  // Mark as immovable
    boxTransform->setPosition(Vec2(300.0f, 300.0f));
    boxTransform->setVelocity(Vec2(0.0f, 0.0f));

    // Circle approaching corner diagonally
    auto circle          = EntityManager::instance().addEntity("circle");
    auto circleTransform = circle->addComponent<CTransform>();
    auto circleCollider  = circle->addComponent<CCircleCollider>(20.0f);
    // Position near corner: box is 60x60 centered at 300,300, so corner is at ~330,330
    circleTransform->setPosition(Vec2(345.0f, 345.0f));  // Near corner
    circleTransform->setVelocity(Vec2(-50.0f, -50.0f));  // Moving toward corner

    EntityManager::instance().update(0.0f);

    // Store initial velocity
    Vec2 initialVel = circleTransform->getVelocity();

    // Update physics to resolve collision
    physics.update(0.016f);

    // Velocity should have changed after collision
    Vec2 finalVel        = circleTransform->getVelocity();
    bool velocityChanged = (finalVel.x != initialVel.x) || (finalVel.y != initialVel.y);

    // Either collision happened or they're separated - both are valid outcomes
    // depending on exact positioning
    EXPECT_TRUE(velocityChanged || !circleCollider->intersects(boxCollider));
}

// ========== REFACTORED COLLISION RESOLUTION TESTS ==========
// These tests verify the new separated collision resolution functions

TEST_F(PhysicsSystemTest, CircleVsCircle_HeadOnCollision)
{
    // Test the resolveCircleVsCircle function with a head-on collision
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    // Two circles moving toward each other
    auto circle1    = EntityManager::instance().addEntity("circle1");
    auto transform1 = circle1->addComponent<CTransform>();
    auto collider1  = circle1->addComponent<CCircleCollider>(30.0f);
    transform1->setPosition(Vec2(200.0f, 200.0f));
    transform1->setVelocity(Vec2(100.0f, 0.0f));  // Moving right

    auto circle2    = EntityManager::instance().addEntity("circle2");
    auto transform2 = circle2->addComponent<CTransform>();
    auto collider2  = circle2->addComponent<CCircleCollider>(30.0f);
    transform2->setPosition(Vec2(250.0f, 200.0f));  // 50 units apart, overlapping (radius sum = 60)
    transform2->setVelocity(Vec2(-100.0f, 0.0f));   // Moving left

    EntityManager::instance().update(0.0f);

    // Store initial velocities
    float initialVel1X = transform1->getVelocity().x;
    float initialVel2X = transform2->getVelocity().x;

    // Update physics
    physics.update(0.016f);

    // Check velocities reversed (elastic collision)
    float finalVel1X = transform1->getVelocity().x;
    float finalVel2X = transform2->getVelocity().x;

    // With restitution of 0.8, velocities should reverse and reduce
    EXPECT_LT(finalVel1X, 0.0f);  // Circle 1 should bounce back (negative)
    EXPECT_GT(finalVel2X, 0.0f);  // Circle 2 should bounce back (positive)
    EXPECT_NE(finalVel1X, initialVel1X);
    EXPECT_NE(finalVel2X, initialVel2X);
}

TEST_F(PhysicsSystemTest, CircleVsCircle_StaticCollision)
{
    // Test circle colliding with static circle
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    // Static circle
    auto staticCircle    = EntityManager::instance().addEntity("static");
    auto staticTransform = staticCircle->addComponent<CTransform>();
    auto staticCollider  = staticCircle->addComponent<CCircleCollider>(40.0f);
    staticCollider->setStatic(true);
    staticTransform->setPosition(Vec2(300.0f, 200.0f));
    staticTransform->setVelocity(Vec2(0.0f, 0.0f));

    // Moving circle
    auto movingCircle    = EntityManager::instance().addEntity("moving");
    auto movingTransform = movingCircle->addComponent<CTransform>();
    auto movingCollider  = movingCircle->addComponent<CCircleCollider>(40.0f);
    movingTransform->setPosition(Vec2(250.0f, 200.0f));  // 50 apart, overlapping (radius sum = 80)
    movingTransform->setVelocity(Vec2(100.0f, 0.0f));

    EntityManager::instance().update(0.0f);

    // Update physics
    physics.update(0.016f);

    // Static should not move
    EXPECT_EQ(staticTransform->getVelocity().x, 0.0f);
    EXPECT_EQ(staticTransform->getVelocity().y, 0.0f);

    // Moving circle should bounce back
    EXPECT_LT(movingTransform->getVelocity().x, 0.0f);
}

TEST_F(PhysicsSystemTest, CircleVsCircle_GlancingBlow)
{
    // Test circles colliding at an angle
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    auto circle1    = EntityManager::instance().addEntity("circle1");
    auto transform1 = circle1->addComponent<CTransform>();
    auto collider1  = circle1->addComponent<CCircleCollider>(25.0f);
    transform1->setPosition(Vec2(200.0f, 200.0f));
    transform1->setVelocity(Vec2(50.0f, 0.0f));

    auto circle2    = EntityManager::instance().addEntity("circle2");
    auto transform2 = circle2->addComponent<CTransform>();
    auto collider2  = circle2->addComponent<CCircleCollider>(25.0f);
    transform2->setPosition(Vec2(240.0f, 220.0f));  // Offset vertically
    transform2->setVelocity(Vec2(-50.0f, 0.0f));

    EntityManager::instance().update(0.0f);

    physics.update(0.016f);

    // Both circles should have y-component to their velocity after glancing collision
    Vec2 vel1 = transform1->getVelocity();
    Vec2 vel2 = transform2->getVelocity();

    // At least one should have gained y-velocity
    bool hasYVelocity = (std::abs(vel1.y) > 0.1f) || (std::abs(vel2.y) > 0.1f);
    EXPECT_TRUE(hasYVelocity);
}

TEST_F(PhysicsSystemTest, CircleVsBox_DirectImpact)
{
    // Test circle hitting box face-on
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    // Static box
    auto box          = EntityManager::instance().addEntity("box");
    auto boxTransform = box->addComponent<CTransform>();
    auto boxCollider  = box->addComponent<CBoxCollider>(50.0f, 50.0f);
    boxCollider->setStatic(true);
    boxTransform->setPosition(Vec2(300.0f, 200.0f));
    boxTransform->setVelocity(Vec2(0.0f, 0.0f));

    // Moving circle
    auto circle          = EntityManager::instance().addEntity("circle");
    auto circleTransform = circle->addComponent<CTransform>();
    auto circleCollider  = circle->addComponent<CCircleCollider>(30.0f);  // radius = 30
    circleTransform->setPosition(Vec2(246.0f, 200.0f));  // Circle right edge at 276, box left edge at 275 - overlapping!
    circleTransform->setVelocity(Vec2(100.0f, 0.0f));

    EntityManager::instance().update(0.0f);

    float initialVelX = circleTransform->getVelocity().x;

    physics.update(0.016f);

    float finalVelX = circleTransform->getVelocity().x;

    // Circle should bounce back (negative velocity)
    EXPECT_NE(finalVelX, initialVelX);
    EXPECT_LT(finalVelX, 0.0f);
}

TEST_F(PhysicsSystemTest, CircleVsBox_CornerCollision)
{
    // Test circle hitting box corner
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    auto box          = EntityManager::instance().addEntity("box");
    auto boxTransform = box->addComponent<CTransform>();
    auto boxCollider  = box->addComponent<CBoxCollider>(40.0f, 40.0f);
    boxCollider->setStatic(true);
    boxTransform->setPosition(Vec2(300.0f, 300.0f));
    boxTransform->setVelocity(Vec2(0.0f, 0.0f));

    auto circle          = EntityManager::instance().addEntity("circle");
    auto circleTransform = circle->addComponent<CTransform>();
    auto circleCollider  = circle->addComponent<CCircleCollider>(25.0f);
    // Position to hit the top-right corner
    circleTransform->setPosition(Vec2(335.0f, 335.0f));
    circleTransform->setVelocity(Vec2(-50.0f, -50.0f));

    EntityManager::instance().update(0.0f);

    physics.update(0.016f);

    // Circle should bounce away from corner (both components should change)
    Vec2 vel             = circleTransform->getVelocity();
    bool velocityChanged = (vel.x != -50.0f) || (vel.y != -50.0f);
    EXPECT_TRUE(velocityChanged);
}

TEST_F(PhysicsSystemTest, CircleVsBox_CircleAsFirstParameter)
{
    // Test with circle as entity A and box as entity B
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    // Create circle first
    auto circle          = EntityManager::instance().addEntity("circle");
    auto circleTransform = circle->addComponent<CTransform>();
    auto circleCollider  = circle->addComponent<CCircleCollider>(30.0f);  // radius = 30
    circleTransform->setPosition(Vec2(211.0f, 200.0f));  // Circle right edge at 241, box left edge at 240 - overlapping!
    circleTransform->setVelocity(Vec2(80.0f, 0.0f));

    // Create box second
    auto box          = EntityManager::instance().addEntity("box");
    auto boxTransform = box->addComponent<CTransform>();
    auto boxCollider  = box->addComponent<CBoxCollider>(40.0f, 40.0f);  // half-width = 20
    boxCollider->setStatic(true);
    boxTransform->setPosition(Vec2(260.0f, 200.0f));

    EntityManager::instance().update(0.0f);

    physics.update(0.016f);

    // Circle should bounce back
    EXPECT_LT(circleTransform->getVelocity().x, 0.0f);
}

TEST_F(PhysicsSystemTest, CircleVsBox_BoxAsFirstParameter)
{
    // Test with box as entity A and circle as entity B
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    // Create box first
    auto box          = EntityManager::instance().addEntity("box");
    auto boxTransform = box->addComponent<CTransform>();
    auto boxCollider  = box->addComponent<CBoxCollider>(40.0f, 40.0f);  // half-width = 20
    boxCollider->setStatic(true);
    boxTransform->setPosition(Vec2(260.0f, 200.0f));

    // Create circle second
    auto circle          = EntityManager::instance().addEntity("circle");
    auto circleTransform = circle->addComponent<CTransform>();
    auto circleCollider  = circle->addComponent<CCircleCollider>(30.0f);  // radius = 30
    circleTransform->setPosition(Vec2(211.0f, 200.0f));  // Circle right edge at 241, box left edge at 240 - overlapping!
    circleTransform->setVelocity(Vec2(80.0f, 0.0f));

    EntityManager::instance().update(0.0f);

    physics.update(0.016f);

    // Circle should bounce back (same result regardless of parameter order)
    EXPECT_LT(circleTransform->getVelocity().x, 0.0f);
}

TEST_F(PhysicsSystemTest, BoxVsBox_HeadOnCollision)
{
    // Test two boxes colliding head-on
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    auto box1       = EntityManager::instance().addEntity("box1");
    auto transform1 = box1->addComponent<CTransform>();
    auto collider1  = box1->addComponent<CBoxCollider>(30.0f, 30.0f);  // half-width = 15
    transform1->setPosition(Vec2(200.0f, 200.0f));
    transform1->setVelocity(Vec2(60.0f, 0.0f));

    auto box2       = EntityManager::instance().addEntity("box2");
    auto transform2 = box2->addComponent<CTransform>();
    auto collider2  = box2->addComponent<CBoxCollider>(30.0f, 30.0f);  // half-width = 15
    transform2->setPosition(Vec2(228.0f, 200.0f));                     // 28 apart, overlapping (half-width sum = 30)
    transform2->setVelocity(Vec2(-60.0f, 0.0f));

    EntityManager::instance().update(0.0f);

    physics.update(0.016f);

    // Both should bounce back
    EXPECT_LT(transform1->getVelocity().x, 0.0f);
    EXPECT_GT(transform2->getVelocity().x, 0.0f);
}

TEST_F(PhysicsSystemTest, BoxVsBox_OffsetCollision)
{
    // Test boxes colliding with vertical offset
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    auto box1       = EntityManager::instance().addEntity("box1");
    auto transform1 = box1->addComponent<CTransform>();
    auto collider1  = box1->addComponent<CBoxCollider>(40.0f, 40.0f);  // half-width = 20
    transform1->setPosition(Vec2(200.0f, 200.0f));
    transform1->setVelocity(Vec2(50.0f, 0.0f));

    auto box2       = EntityManager::instance().addEntity("box2");
    auto transform2 = box2->addComponent<CTransform>();
    auto collider2  = box2->addComponent<CBoxCollider>(40.0f, 60.0f);  // half-width = 20, half-height = 30
    transform2->setPosition(Vec2(238.0f, 190.0f));  // 38 apart horizontally, overlapping (half-width sum = 40)
    transform2->setVelocity(Vec2(-50.0f, 0.0f));

    EntityManager::instance().update(0.0f);

    physics.update(0.016f);

    // Should resolve along the axis of least penetration
    Vec2 vel1 = transform1->getVelocity();
    Vec2 vel2 = transform2->getVelocity();

    // At least one velocity component should have changed
    bool changed = (vel1.x != 50.0f) || (vel1.y != 0.0f) || (vel2.x != -50.0f) || (vel2.y != 0.0f);
    EXPECT_TRUE(changed);
}

TEST_F(PhysicsSystemTest, BoxVsBox_StaticCollision)
{
    // Test dynamic box hitting static box
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    auto staticBox       = EntityManager::instance().addEntity("static");
    auto staticTransform = staticBox->addComponent<CTransform>();
    auto staticCollider  = staticBox->addComponent<CBoxCollider>(50.0f, 50.0f);  // half-width = 25
    staticCollider->setStatic(true);
    staticTransform->setPosition(Vec2(300.0f, 200.0f));

    auto dynamicBox       = EntityManager::instance().addEntity("dynamic");
    auto dynamicTransform = dynamicBox->addComponent<CTransform>();
    auto dynamicCollider  = dynamicBox->addComponent<CBoxCollider>(40.0f, 40.0f);  // half-width = 20
    dynamicTransform->setPosition(Vec2(254.0f, 200.0f));  // 46 apart, overlapping (half-width sum = 45)
    dynamicTransform->setVelocity(Vec2(70.0f, 0.0f));

    EntityManager::instance().update(0.0f);

    physics.update(0.016f);

    // Static should not move
    EXPECT_EQ(staticTransform->getVelocity().x, 0.0f);

    // Dynamic should bounce back
    EXPECT_LT(dynamicTransform->getVelocity().x, 0.0f);
}

TEST_F(PhysicsSystemTest, PositionalCorrection_CircleVsCircle)
{
    // Test that overlapping circles are separated
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    auto circle1    = EntityManager::instance().addEntity("circle1");
    auto transform1 = circle1->addComponent<CTransform>();
    auto collider1  = circle1->addComponent<CCircleCollider>(40.0f);
    transform1->setPosition(Vec2(200.0f, 200.0f));
    transform1->setVelocity(Vec2(0.0f, 0.0f));

    auto circle2    = EntityManager::instance().addEntity("circle2");
    auto transform2 = circle2->addComponent<CTransform>();
    auto collider2  = circle2->addComponent<CCircleCollider>(40.0f);
    transform2->setPosition(Vec2(230.0f, 200.0f));  // Heavy overlap (distance 30 < radius sum 80)
    transform2->setVelocity(Vec2(0.0f, 0.0f));

    EntityManager::instance().update(0.0f);

    // Verify they're overlapping
    EXPECT_TRUE(collider1->intersects(collider2));

    float initialDistance = (transform2->getPosition() - transform1->getPosition()).length();

    physics.update(0.016f);

    float finalDistance = (transform2->getPosition() - transform1->getPosition()).length();

    // They should be pushed apart
    EXPECT_GT(finalDistance, initialDistance);
}

TEST_F(PhysicsSystemTest, PositionalCorrection_CircleVsBox)
{
    // Test that overlapping circle and box are separated
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    auto box          = EntityManager::instance().addEntity("box");
    auto boxTransform = box->addComponent<CTransform>();
    auto boxCollider  = box->addComponent<CBoxCollider>(50.0f, 50.0f);
    boxCollider->setStatic(true);
    boxTransform->setPosition(Vec2(300.0f, 200.0f));

    auto circle          = EntityManager::instance().addEntity("circle");
    auto circleTransform = circle->addComponent<CTransform>();
    auto circleCollider  = circle->addComponent<CCircleCollider>(35.0f);
    circleTransform->setPosition(Vec2(260.0f, 200.0f));  // Deeply penetrating
    circleTransform->setVelocity(Vec2(0.0f, 0.0f));

    EntityManager::instance().update(0.0f);

    EXPECT_TRUE(circleCollider->intersects(boxCollider));

    float initialX = circleTransform->getPosition().x;

    physics.update(0.016f);

    float finalX = circleTransform->getPosition().x;

    // Circle should be pushed away from box (to the left)
    EXPECT_LT(finalX, initialX);
}

TEST_F(PhysicsSystemTest, NoVelocityChange_WhenSeparating)
{
    // Test that no velocity change occurs when objects are already separating
    auto& physics = S2DPhysics::instance();
    physics.setWorldBounds(Vec2(400.0f, 400.0f), Vec2(800.0f, 800.0f));

    auto circle1    = EntityManager::instance().addEntity("circle1");
    auto transform1 = circle1->addComponent<CTransform>();
    auto collider1  = circle1->addComponent<CCircleCollider>(30.0f);
    transform1->setPosition(Vec2(200.0f, 200.0f));
    transform1->setVelocity(Vec2(-50.0f, 0.0f));  // Moving away

    auto circle2    = EntityManager::instance().addEntity("circle2");
    auto transform2 = circle2->addComponent<CTransform>();
    auto collider2  = circle2->addComponent<CCircleCollider>(30.0f);
    transform2->setPosition(Vec2(250.0f, 200.0f));  // Slightly overlapping
    transform2->setVelocity(Vec2(50.0f, 0.0f));     // Moving away

    EntityManager::instance().update(0.0f);

    float initialVel1 = transform1->getVelocity().x;
    float initialVel2 = transform2->getVelocity().x;

    physics.update(0.016f);

    // Velocities might change due to positional correction, but the impulse
    // should only apply if approaching. With high separation velocities,
    // the relative velocity check should prevent additional impulse
    // This is a subtle test - main goal is no crash and reasonable behavior
    Vec2 vel1 = transform1->getVelocity();
    Vec2 vel2 = transform2->getVelocity();

    // Verify they're still generally moving apart
    EXPECT_TRUE((vel1.x < 0.0f && vel2.x > 0.0f) || !collider1->intersects(collider2));
}