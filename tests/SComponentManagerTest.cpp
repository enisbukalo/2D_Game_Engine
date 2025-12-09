#include <gtest/gtest.h>
#include "systems/SComponentManager.h"
#include "systems/SEntity.h"
#include "components/CTransform.h"
#include "components/CPhysicsBody2D.h"

using namespace ::Systems;
using namespace ::Entity;
using namespace ::Components;

TEST(SComponentManagerTest, RegisterComponentsOnAdd)
{
    auto& manager = SComponentManager::instance();
    auto& entityManager = SEntity::instance();

    // Create an entity and add components
    auto entity = entityManager.addEntity("test_entity");
    auto* transform = entity->addComponent<CTransform>();
    ASSERT_NE(transform, nullptr);

    auto* physics = entity->addComponent<CPhysicsBody2D>();
    ASSERT_NE(physics, nullptr);

    // Active list should contain the transform and physics components
    const auto& actives = manager.getActiveComponents();
    bool foundTransform = false;
    bool foundPhysics = false;
    for (auto c : actives)
    {
        if (c == transform) foundTransform = true;
        if (c == physics) foundPhysics = true;
    }
    EXPECT_TRUE(foundTransform);
    EXPECT_TRUE(foundPhysics);

    // Physics subset should contain CPhysicsBody2D
    const auto& phys = manager.getPhysicsComponents();
    bool foundPhysicsSubset = false;
    for (auto p : phys)
    {
        if (p == physics) foundPhysicsSubset = true;
    }
    EXPECT_TRUE(foundPhysicsSubset);
}

TEST(SComponentManagerTest, ActivationTransitions)
{
    auto& manager = SComponentManager::instance();
    auto& entityManager = SEntity::instance();

    // Create entity and component
    auto entity = entityManager.addEntity("active_test");
    auto* transform = entity->addComponent<CTransform>();
    ASSERT_NE(transform, nullptr);

    // Make sure component is active initially
    const auto& activesBefore = manager.getActiveComponents();
    EXPECT_NE(std::find(activesBefore.begin(), activesBefore.end(), transform), activesBefore.end());

    // Deactivate component
    transform->setActive(false);
    const auto& activesAfterDeactivate = manager.getActiveComponents();
    EXPECT_EQ(std::find(activesAfterDeactivate.begin(), activesAfterDeactivate.end(), transform), activesAfterDeactivate.end());

    // Reactivate
    transform->setActive(true);
    const auto& activesAfterReactivate = manager.getActiveComponents();
    EXPECT_NE(std::find(activesAfterReactivate.begin(), activesAfterReactivate.end(), transform), activesAfterReactivate.end());
}

TEST(SComponentManagerTest, EntityPausePropagatesToComponents)
{
    auto& manager = SComponentManager::instance();
    auto& entityManager = SEntity::instance();

    auto entity = entityManager.addEntity("pause_test");
    auto* t = entity->addComponent<CTransform>();
    auto* p = entity->addComponent<CPhysicsBody2D>();

    ASSERT_TRUE(t->isActive());
    ASSERT_TRUE(p->isActive());

    entity->setActive(false);
    EXPECT_FALSE(t->isActive());
    EXPECT_FALSE(p->isActive());

    // Active components should not include them
    const auto& actives = manager.getActiveComponents();
    EXPECT_EQ(std::find(actives.begin(), actives.end(), t), actives.end());
    EXPECT_EQ(std::find(actives.begin(), actives.end(), p), actives.end());

    // Restore
    entity->setActive(true);
    EXPECT_TRUE(t->isActive());
    EXPECT_TRUE(p->isActive());

    const auto& activesRestored = manager.getActiveComponents();
    EXPECT_NE(std::find(activesRestored.begin(), activesRestored.end(), t), activesRestored.end());
    EXPECT_NE(std::find(activesRestored.begin(), activesRestored.end(), p), activesRestored.end());
}

TEST(SComponentManagerTest, EntityDestroyUnregistersComponents)
{
    auto& manager = SComponentManager::instance();
    auto& entityManager = SEntity::instance();

    auto entity = entityManager.addEntity("destroy_test");
    auto* t = entity->addComponent<CTransform>();
    auto* p = entity->addComponent<CPhysicsBody2D>();

    // Ensure in active list now
    const auto& activesBefore = manager.getActiveComponents();
    EXPECT_NE(std::find(activesBefore.begin(), activesBefore.end(), t), activesBefore.end());
    EXPECT_NE(std::find(activesBefore.begin(), activesBefore.end(), p), activesBefore.end());

    entity->destroy();

    // Components should have been unregistered
    const auto& activesAfter = manager.getActiveComponents();
    EXPECT_EQ(std::find(activesAfter.begin(), activesAfter.end(), t), activesAfter.end());
    EXPECT_EQ(std::find(activesAfter.begin(), activesAfter.end(), p), activesAfter.end());
}
