#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
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
    // EXPECT_NEAR(transform->velocity.y, -9.81f, EPSILON);
}

TEST_F(EntityManagerTest, EntitySerialization)
{
    // Create an entity with multiple components
    auto entity         = m_manager.addEntity("testEntity");
    auto transform      = entity->addComponent<CTransform>();
    transform->position = Vec2(1.0f, 2.0f);
    transform->velocity = Vec2(3.0f, 4.0f);
    transform->scale    = Vec2(2.0f, 2.0f);
    transform->rotation = 45.0f;

    entity->addComponent<CGravity>();
    entity->addComponent<CName>("TestName");

    m_manager.update(0.0f);  // Process pending entities

    // Save to file
    std::string testFile = std::string(SOURCE_DIR) + "/test_save.json";
    m_manager.saveToFile(testFile);

    // Create a new manager and load the file
    EntityManager newManager;
    newManager.loadFromFile(testFile);

    // Verify loaded entity
    auto loadedEntities = newManager.getEntities();
    EXPECT_EQ(loadedEntities.size(), 1);

    auto loadedEntity    = loadedEntities[0];
    auto loadedTransform = loadedEntity->getComponent<CTransform>();
    EXPECT_NE(loadedTransform, nullptr);
    EXPECT_FLOAT_EQ(loadedTransform->position.x, 1.0f);
    EXPECT_FLOAT_EQ(loadedTransform->position.y, 2.0f);

    // Clean up test file
    std::filesystem::remove(testFile);
}

TEST_F(EntityManagerTest, LoadPredefinedScene)
{
    std::string   testFile = std::string(SOURCE_DIR) + "/test_scene.json";
    std::ofstream file(testFile);
    file << R"({
        "totalEntities": 2,
        "entities": [
            {
                "id": 0,
                "tag": "player",
                "alive": true,
                "components": [
                    {
                        "type": "Transform",
                        "position": {"x": 100.0, "y": 200.0},
                        "velocity": {"x": 0.0, "y": 0.0},
                        "scale": {"x": 1.0, "y": 1.0},
                        "rotation": 0.0
                    }
                ]
            },
            {
                "id": 1,
                "tag": "enemy",
                "alive": true,
                "components": [
                    {
                        "type": "Transform",
                        "position": {"x": 300.0, "y": 400.0},
                        "velocity": {"x": 0.0, "y": 0.0},
                        "scale": {"x": 1.0, "y": 1.0},
                        "rotation": 0.0
                    },
                    {
                        "type": "Name",
                        "name": "Enemy1"
                    }
                ]
            }
        ]
    })";
    file.close();

    m_manager.loadFromFile(testFile);
    auto entities = m_manager.getEntities();
    EXPECT_EQ(entities.size(), 2);

    auto playerEntities = m_manager.getEntitiesByTag("player");
    auto enemyEntities  = m_manager.getEntitiesByTag("enemy");
    EXPECT_EQ(playerEntities.size(), 1);
    EXPECT_EQ(enemyEntities.size(), 1);

    auto enemy     = enemyEntities[0];
    auto enemyName = enemy->getComponent<CName>();
    EXPECT_NE(enemyName, nullptr);
    EXPECT_EQ(enemyName->name, "Enemy1");

    // Clean up test file
    std::filesystem::remove(testFile);
}