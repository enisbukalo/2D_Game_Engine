#include <gtest/gtest.h>

#include "CPhysicsBody2D.h"
#include "CTransform.h"
#include "Entity.h"
#include "S2DPhysics.h"
#include "SEntity.h"

using namespace Components;
using namespace Entity;
using namespace Systems;

class S2DPhysicsTest : public ::testing::Test
{
protected:
    void SetUp() override { SEntity::instance().clear(); }

    void TearDown() override { SEntity::instance().clear(); }
};

//=============================================================================
// Registration Tests
//=============================================================================

TEST_F(S2DPhysicsTest, BodyIsAutomaticallyRegisteredOnInitialize)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    // Body should be automatically registered after initialization
    int callCount = 0;
    body->setFixedUpdateCallback([&callCount](float dt) { callCount++; });

    // Run fixed updates to verify body is registered
    S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f);

    EXPECT_EQ(callCount, 1);
}

TEST_F(S2DPhysicsTest, BodyIsAutomaticallyUnregisteredOnDestroy)
{
    auto entity = SEntity::instance().addEntity("test");
    auto body   = entity->addComponent<CPhysicsBody2D>();

    int callCount = 0;
    body->setFixedUpdateCallback([&callCount](float dt) { callCount++; });

    // Run fixed updates - should be called
    S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f);
    EXPECT_EQ(callCount, 1);

    // Destroy entity (and thus the body)
    SEntity::instance().removeEntity(entity);

    // Run fixed updates again - callback should not be called
    S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f);
    EXPECT_EQ(callCount, 1);  // Still 1, not incremented
}

TEST_F(S2DPhysicsTest, MultipleBodyRegistrations)
{
    auto  entity1 = SEntity::instance().addEntity("test1");
    auto* body1   = entity1->addComponent<CPhysicsBody2D>();

    auto  entity2 = SEntity::instance().addEntity("test2");
    auto* body2   = entity2->addComponent<CPhysicsBody2D>();

    auto  entity3 = SEntity::instance().addEntity("test3");
    auto* body3   = entity3->addComponent<CPhysicsBody2D>();

    int callCount1 = 0;
    int callCount2 = 0;
    int callCount3 = 0;

    body1->setFixedUpdateCallback([&callCount1](float dt) { callCount1++; });
    body2->setFixedUpdateCallback([&callCount2](float dt) { callCount2++; });
    body3->setFixedUpdateCallback([&callCount3](float dt) { callCount3++; });

    S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f);

    EXPECT_EQ(callCount1, 1);
    EXPECT_EQ(callCount2, 1);
    EXPECT_EQ(callCount3, 1);
}

TEST_F(S2DPhysicsTest, DuplicateRegistrationPrevented)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    int callCount = 0;
    body->setFixedUpdateCallback([&callCount](float dt) { callCount++; });

    // Try to register the same body multiple times
    S2DPhysics::instance().registerBody(body);
    S2DPhysics::instance().registerBody(body);
    S2DPhysics::instance().registerBody(body);

    // Should only be called once despite multiple registrations
    S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f);

    EXPECT_EQ(callCount, 1);
}

//=============================================================================
// Fixed Update Tests
//=============================================================================

TEST_F(S2DPhysicsTest, RunFixedUpdatesWithNoCallbacks)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    // Body has no callback set
    EXPECT_FALSE(body->hasFixedUpdateCallback());

    // Should not crash when running fixed updates
    EXPECT_NO_THROW(S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f));
}

TEST_F(S2DPhysicsTest, RunFixedUpdatesPassesCorrectDeltaTime)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    float receivedDt = 0.0f;
    body->setFixedUpdateCallback([&receivedDt](float dt) { receivedDt = dt; });

    const float expectedDt = 1.0f / 60.0f;
    S2DPhysics::instance().runFixedUpdates(expectedDt);

    EXPECT_FLOAT_EQ(receivedDt, expectedDt);
}

TEST_F(S2DPhysicsTest, RunFixedUpdatesCalledMultipleTimes)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    int callCount = 0;
    body->setFixedUpdateCallback([&callCount](float dt) { callCount++; });

    S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f);
    EXPECT_EQ(callCount, 1);

    S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f);
    EXPECT_EQ(callCount, 2);

    S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f);
    EXPECT_EQ(callCount, 3);
}

TEST_F(S2DPhysicsTest, RunFixedUpdatesCallsAllRegisteredCallbacks)
{
    const int numBodies = 10;
    int       callCounts[numBodies] = {0};

    for (int i = 0; i < numBodies; ++i)
    {
        auto  entity = SEntity::instance().addEntity("test" + std::to_string(i));
        auto* body   = entity->addComponent<CPhysicsBody2D>();
        body->setFixedUpdateCallback([&callCounts, i](float dt) { callCounts[i]++; });
    }

    S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f);

    for (int i = 0; i < numBodies; ++i)
    {
        EXPECT_EQ(callCounts[i], 1) << "Body " << i << " callback was not called";
    }
}

TEST_F(S2DPhysicsTest, RunFixedUpdatesSkipsBodiesWithoutCallbacks)
{
    auto  entity1 = SEntity::instance().addEntity("test1");
    auto* body1   = entity1->addComponent<CPhysicsBody2D>();

    auto  entity2 = SEntity::instance().addEntity("test2");
    auto* body2   = entity2->addComponent<CPhysicsBody2D>();

    auto  entity3 = SEntity::instance().addEntity("test3");
    auto* body3   = entity3->addComponent<CPhysicsBody2D>();

    int callCount1 = 0;
    int callCount3 = 0;

    // Only set callbacks for body1 and body3, skip body2
    body1->setFixedUpdateCallback([&callCount1](float dt) { callCount1++; });
    body3->setFixedUpdateCallback([&callCount3](float dt) { callCount3++; });

    S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f);

    EXPECT_EQ(callCount1, 1);
    EXPECT_EQ(callCount3, 1);
}

//=============================================================================
// Unregister Tests
//=============================================================================

TEST_F(S2DPhysicsTest, UnregisterBodyStopsCallbacks)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    int callCount = 0;
    body->setFixedUpdateCallback([&callCount](float dt) { callCount++; });

    S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f);
    EXPECT_EQ(callCount, 1);

    // Manually unregister
    S2DPhysics::instance().unregisterBody(body);

    S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f);
    EXPECT_EQ(callCount, 1);  // Should not increment
}

TEST_F(S2DPhysicsTest, UnregisterNonExistentBodyDoesNotCrash)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    // Try to unregister a body that was never registered (or already unregistered)
    EXPECT_NO_THROW(S2DPhysics::instance().unregisterBody(body));
    EXPECT_NO_THROW(S2DPhysics::instance().unregisterBody(body));
}

TEST_F(S2DPhysicsTest, UnregisterOneOfMultipleBodies)
{
    auto  entity1 = SEntity::instance().addEntity("test1");
    auto* body1   = entity1->addComponent<CPhysicsBody2D>();

    auto  entity2 = SEntity::instance().addEntity("test2");
    auto* body2   = entity2->addComponent<CPhysicsBody2D>();

    auto  entity3 = SEntity::instance().addEntity("test3");
    auto* body3   = entity3->addComponent<CPhysicsBody2D>();

    int callCount1 = 0;
    int callCount2 = 0;
    int callCount3 = 0;

    body1->setFixedUpdateCallback([&callCount1](float dt) { callCount1++; });
    body2->setFixedUpdateCallback([&callCount2](float dt) { callCount2++; });
    body3->setFixedUpdateCallback([&callCount3](float dt) { callCount3++; });

    S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f);
    EXPECT_EQ(callCount1, 1);
    EXPECT_EQ(callCount2, 1);
    EXPECT_EQ(callCount3, 1);

    // Unregister body2
    S2DPhysics::instance().unregisterBody(body2);

    S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f);
    EXPECT_EQ(callCount1, 2);
    EXPECT_EQ(callCount2, 1);  // Should not increment
    EXPECT_EQ(callCount3, 2);
}

//=============================================================================
// Integration Tests
//=============================================================================

TEST_F(S2DPhysicsTest, FixedUpdateCallbackCanApplyForces)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();
    body->setBodyType(BodyType::Dynamic);

    // Set a callback that would apply a force (like player input)
    b2Vec2 appliedForce = {100.0f, 0.0f};
    body->setFixedUpdateCallback([body, appliedForce](float dt) {
        body->applyForceToCenter(appliedForce);
    });

    // Verify callback is registered and can be invoked
    EXPECT_TRUE(body->hasFixedUpdateCallback());

    // Run fixed update
    EXPECT_NO_THROW(S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f));
}

TEST_F(S2DPhysicsTest, FixedUpdateCallbackOrderIsConsistent)
{
    const int numBodies = 5;
    std::vector<int> callOrder;

    for (int i = 0; i < numBodies; ++i)
    {
        auto  entity = SEntity::instance().addEntity("test" + std::to_string(i));
        auto* body   = entity->addComponent<CPhysicsBody2D>();
        body->setFixedUpdateCallback([&callOrder, i](float dt) { callOrder.push_back(i); });
    }

    S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f);

    // Verify all callbacks were called
    EXPECT_EQ(callOrder.size(), numBodies);

    // Store the order
    std::vector<int> firstOrder = callOrder;
    callOrder.clear();

    // Run again
    S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f);

    // Order should be the same
    EXPECT_EQ(callOrder, firstOrder);
}

TEST_F(S2DPhysicsTest, FixedUpdateWithVaryingDeltaTimes)
{
    auto  entity = SEntity::instance().addEntity("test");
    auto* body   = entity->addComponent<CPhysicsBody2D>();

    std::vector<float> receivedDts;
    body->setFixedUpdateCallback([&receivedDts](float dt) { receivedDts.push_back(dt); });

    // Call with different delta times
    S2DPhysics::instance().runFixedUpdates(1.0f / 60.0f);
    S2DPhysics::instance().runFixedUpdates(1.0f / 30.0f);
    S2DPhysics::instance().runFixedUpdates(1.0f / 120.0f);

    ASSERT_EQ(receivedDts.size(), 3);
    EXPECT_FLOAT_EQ(receivedDts[0], 1.0f / 60.0f);
    EXPECT_FLOAT_EQ(receivedDts[1], 1.0f / 30.0f);
    EXPECT_FLOAT_EQ(receivedDts[2], 1.0f / 120.0f);
}
