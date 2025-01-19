#include <gtest/gtest.h>
#include "EntityManager.h"

// Define the source directory path
#ifndef SOURCE_DIR
#define SOURCE_DIR "."
#endif

// Test fixture for entity manager tests
class EntityManagerTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}

    EntityManager m_manager;
};

TEST_F(EntityManagerTest, EntityCreation)
{
    std::shared_ptr<Entity> entity = m_manager.addEntity("test");
    EXPECT_NE(entity, nullptr);
    EXPECT_EQ(entity->getTag(), "test");
    EXPECT_TRUE(entity->isAlive());
}

TEST_F(EntityManagerTest, EntityRemoval)
{
    std::shared_ptr<Entity> entity = m_manager.addEntity("test");

    m_manager.update(0.0f);  // Process pending entities
    EXPECT_EQ(m_manager.getEntities().size(), 1);

    m_manager.removeEntity(entity);
    m_manager.update(0.0f);  // Process removals
    EXPECT_EQ(m_manager.getEntities().size(), 0);
}

TEST_F(EntityManagerTest, EntityTagging)
{
    m_manager.addEntity("typeA");
    m_manager.addEntity("typeA");
    m_manager.addEntity("typeB");

    m_manager.update(0.0f);  // Process pending entities

    std::vector<std::shared_ptr<Entity>> typeAEntities = m_manager.getEntitiesByTag("typeA");
    std::vector<std::shared_ptr<Entity>> typeBEntities = m_manager.getEntitiesByTag("typeB");

    EXPECT_EQ(typeAEntities.size(), 2);
    EXPECT_EQ(typeBEntities.size(), 1);
}

TEST_F(EntityManagerTest, EntityComponentQuery)
{
    std::shared_ptr<Entity> entity1 = m_manager.addEntity("test1");
    entity1->addComponent<CTransform>();

    std::shared_ptr<Entity> entity2 = m_manager.addEntity("test2");
    entity2->addComponent<CTransform>();
    entity2->addComponent<CGravity>();

    m_manager.update(0.0f);  // Process pending entities

    auto entitiesWithTransform = m_manager.getEntitiesWithComponent<CTransform>();
    auto entitiesWithGravity   = m_manager.getEntitiesWithComponent<CGravity>();

    EXPECT_EQ(entitiesWithTransform.size(), 2);
    EXPECT_EQ(entitiesWithGravity.size(), 1);
}

TEST_F(EntityManagerTest, EntityUpdateSystem)
{
    auto entity    = m_manager.addEntity("test");
    auto transform = entity->addComponent<CTransform>();
    auto gravity   = entity->addComponent<CGravity>();

    const float EPSILON = 0.0001f;  // Small value for floating point comparison
    EXPECT_NEAR(gravity->force.x, 0.0f, EPSILON);
    EXPECT_NEAR(gravity->force.y, -9.81f, EPSILON);

    // Test system update
    float deltaTime = 1.0f;
    m_manager.update(deltaTime);
}