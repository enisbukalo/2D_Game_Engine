#include <gtest/gtest.h>
#include <filesystem>
#include "../include/EntityManager.h"

// Define the source directory path
#ifndef SOURCE_DIR
#define SOURCE_DIR "."
#endif

TEST(EntityManagerTest, EntityCreation)
{
    EntityManager manager;

    std::shared_ptr<Entity> entity = manager.addEntity("test");
    EXPECT_NE(entity, nullptr);
    EXPECT_EQ(entity->getTag(), "test");
    EXPECT_TRUE(entity->isAlive());
}

TEST(EntityManagerTest, EntityRemoval)
{
    EntityManager           manager;
    std::shared_ptr<Entity> entity = manager.addEntity("test");

    manager.update(0.0f);  // Process pending entities
    EXPECT_EQ(manager.getEntities().size(), 1);

    manager.removeEntity(entity);
    manager.update(0.0f);  // Process removals
    EXPECT_EQ(manager.getEntities().size(), 0);
}

TEST(EntityManagerTest, EntityTagging)
{
    EntityManager manager;

    manager.addEntity("typeA");
    manager.addEntity("typeA");
    manager.addEntity("typeB");

    manager.update(0.0f);  // Process pending entities

    std::vector<std::shared_ptr<Entity>> typeAEntities = manager.getEntitiesByTag("typeA");
    std::vector<std::shared_ptr<Entity>> typeBEntities = manager.getEntitiesByTag("typeB");

    EXPECT_EQ(typeAEntities.size(), 2);
    EXPECT_EQ(typeBEntities.size(), 1);
}

TEST(EntityManagerTest, EntityComponentQuery)
{
    EntityManager manager;

    std::shared_ptr<Entity> entity1 = manager.addEntity("test1");
    entity1->addComponent<CTransform>();

    std::shared_ptr<Entity> entity2 = manager.addEntity("test2");
    entity2->addComponent<CTransform>();
    entity2->addComponent<CGravity>();

    manager.update(0.0f);  // Process pending entities

    auto entitiesWithTransform = manager.getEntitiesWithComponent<CTransform>();
    auto entitiesWithGravity   = manager.getEntitiesWithComponent<CGravity>();

    EXPECT_EQ(entitiesWithTransform.size(), 2);
    EXPECT_EQ(entitiesWithGravity.size(), 1);
}

TEST(EntityManagerTest, EntityUpdateSystem)
{
    EntityManager manager;

    std::shared_ptr<Entity> entity    = manager.addEntity("test");
    CTransform             *transform = entity->addComponent<CTransform>();
    CGravity               *gravity   = entity->addComponent<CGravity>();

    manager.update(0.0f);  // Process pending entities
    manager.update(1.0f);  // Update components

    EXPECT_FLOAT_EQ(transform->velocity.y, -9.81f);
}

TEST(EntityManagerTest, EntitySerialization)
{
    EntityManager manager;

    // Create an entity with multiple components
    auto entity         = manager.addEntity("testEntity");
    auto transform      = entity->addComponent<CTransform>();
    transform->position = Vec2(1.0f, 2.0f);
    transform->velocity = Vec2(3.0f, 4.0f);
    transform->scale    = Vec2(2.0f, 2.0f);
    transform->rotation = 45.0f;

    auto name      = entity->addComponent<CName>("TestObject");
    auto gravity   = entity->addComponent<CGravity>();
    gravity->force = Vec2(0.0f, -15.0f);

    manager.update(0.0f);  // Process pending entities

    // Save to file
    manager.saveToFile("test_entity.json");

    // Create a new manager and load the file
    EntityManager newManager;
    newManager.loadFromFile("test_entity.json");
    newManager.update(0.0f);  // Process loaded entities

    // Verify loaded entity
    auto loadedEntities = newManager.getEntities();
    ASSERT_EQ(loadedEntities.size(), 1);

    auto loadedEntity = loadedEntities[0];
    EXPECT_EQ(loadedEntity->getTag(), "testEntity");

    auto loadedTransform = loadedEntity->getComponent<CTransform>();
    ASSERT_NE(loadedTransform, nullptr);
    EXPECT_FLOAT_EQ(loadedTransform->position.x, 1.0f);
    EXPECT_FLOAT_EQ(loadedTransform->position.y, 2.0f);
    EXPECT_FLOAT_EQ(loadedTransform->velocity.x, 3.0f);
    EXPECT_FLOAT_EQ(loadedTransform->velocity.y, 4.0f);
    EXPECT_FLOAT_EQ(loadedTransform->scale.x, 2.0f);
    EXPECT_FLOAT_EQ(loadedTransform->scale.y, 2.0f);
    EXPECT_FLOAT_EQ(loadedTransform->rotation, 45.0f);

    auto loadedName = loadedEntity->getComponent<CName>();
    ASSERT_NE(loadedName, nullptr);
    EXPECT_EQ(loadedName->name, "TestObject");

    auto loadedGravity = loadedEntity->getComponent<CGravity>();
    ASSERT_NE(loadedGravity, nullptr);
    EXPECT_FLOAT_EQ(loadedGravity->force.x, 0.0f);
    EXPECT_FLOAT_EQ(loadedGravity->force.y, -15.0f);

    // Clean up test file
    std::remove("test_entity.json");
}

TEST(EntityManagerTest, LoadPredefinedScene)
{
    EntityManager manager;

    // Load the predefined scene
    std::string testFilePath = std::string(SOURCE_DIR) + "/tests/test_data/test_scene.json";
    manager.loadFromFile(testFilePath);
    manager.update(0.0f);  // Process loaded entities

    // Verify total number of entities
    auto entities = manager.getEntities();
    ASSERT_EQ(entities.size(), 3);

    // Test player entity
    auto playerEntities = manager.getEntitiesByTag("player");
    ASSERT_EQ(playerEntities.size(), 1);
    auto player = playerEntities[0];

    auto playerTransform = player->getComponent<CTransform>();
    ASSERT_NE(playerTransform, nullptr);
    EXPECT_FLOAT_EQ(playerTransform->position.x, 100.0f);
    EXPECT_FLOAT_EQ(playerTransform->position.y, 200.0f);
    EXPECT_FLOAT_EQ(playerTransform->velocity.x, 5.0f);
    EXPECT_FLOAT_EQ(playerTransform->velocity.y, 0.0f);

    auto playerName = player->getComponent<CName>();
    ASSERT_NE(playerName, nullptr);
    EXPECT_EQ(playerName->name, "Player1");

    // Test platform entity
    auto platformEntities = manager.getEntitiesByTag("platform");
    ASSERT_EQ(platformEntities.size(), 1);
    auto platform = platformEntities[0];

    auto platformTransform = platform->getComponent<CTransform>();
    ASSERT_NE(platformTransform, nullptr);
    EXPECT_FLOAT_EQ(platformTransform->position.y, -50.0f);
    EXPECT_FLOAT_EQ(platformTransform->scale.x, 10.0f);
    EXPECT_FALSE(platform->hasComponent<CGravity>());

    // Test enemy entity
    auto enemyEntities = manager.getEntitiesByTag("enemy");
    ASSERT_EQ(enemyEntities.size(), 1);
    auto enemy = enemyEntities[0];

    auto enemyTransform = enemy->getComponent<CTransform>();
    ASSERT_NE(enemyTransform, nullptr);
    EXPECT_FLOAT_EQ(enemyTransform->position.x, 300.0f);
    EXPECT_FLOAT_EQ(enemyTransform->velocity.x, -2.0f);
    EXPECT_FLOAT_EQ(enemyTransform->rotation, 180.0f);

    auto enemyGravity = enemy->getComponent<CGravity>();
    ASSERT_NE(enemyGravity, nullptr);
    EXPECT_FLOAT_EQ(enemyGravity->force.y, -9.81f);
}