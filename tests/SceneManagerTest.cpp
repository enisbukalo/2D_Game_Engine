#include <gtest/gtest.h>
#include <filesystem>
#include "CCircleCollider.h"
#include "CGravity.h"
#include "CName.h"
#include "CTransform.h"
#include "EntityManager.h"
#include "SceneManager.h"
#include "TestUtils.h"

// Test fixture for scene manager tests
class SceneManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EntityManager::instance().clear();

        // Create test directory if it doesn't exist
        if (!std::filesystem::exists("tests/test_data"))
        {
            std::filesystem::create_directory("tests/test_data");
        }
    }

    void TearDown() override
    {
        // Clean up test files
        const std::string testFile = "tests/test_data/test_scene.json";
        if (std::filesystem::exists(testFile))
        {
            std::filesystem::remove(testFile);
        }
    }

    // Helper method to create a test scene with some entities
    void createTestScene()
    {
        auto& manager = EntityManager::instance();

        // Create an entity with Transform, Gravity and CircleCollider
        auto entity1    = manager.addEntity("physics_object");
        auto transform1 = entity1->addComponent<CTransform>();
        transform1->setPosition(Vec2(100.0f, 200.0f));
        transform1->setScale(Vec2(2.0f, 2.0f));
        auto gravity1 = entity1->addComponent<CGravity>();
        gravity1->setMultiplier(1.5f);
        auto collider1 = entity1->addComponent<CCircleCollider>(3.0f);

        // Create another entity with Transform and Name
        auto entity2    = manager.addEntity("named_object");
        auto transform2 = entity2->addComponent<CTransform>();
        transform2->setPosition(Vec2(-50.0f, 75.0f));
        auto name2 = entity2->addComponent<CName>();
        name2->setName("TestObject");

        // Process pending entities
        manager.update(0.0f);
    }
};

TEST_F(SceneManagerTest, SaveAndLoadScene)
{
    const std::string testFile = "tests/test_data/test_scene.json";

    // Create and save a test scene
    createTestScene();
    auto& sceneManager = SceneManager::instance();
    EXPECT_NO_THROW(sceneManager.saveScene(testFile));
    EXPECT_EQ(sceneManager.getCurrentScenePath(), testFile);

    // Clear the entity manager
    EntityManager::instance().clear();
    EntityManager::instance().update(0.0f);  // Process the clear
    EXPECT_TRUE(EntityManager::instance().getEntities().empty());

    // Load the scene back
    EXPECT_NO_THROW(sceneManager.loadScene(testFile));
    EntityManager::instance().update(0.0f);  // Process loaded entities
    EXPECT_EQ(sceneManager.getCurrentScenePath(), testFile);

    // Verify loaded entities
    const auto& entities = EntityManager::instance().getEntities();
    ASSERT_EQ(entities.size(), 2);

    // Verify physics_object
    auto physicsObjects = EntityManager::instance().getEntitiesByTag("physics_object");
    ASSERT_EQ(physicsObjects.size(), 1);
    auto physicsEntity = physicsObjects[0];

    auto transform1 = physicsEntity->getComponent<CTransform>();
    ASSERT_NE(transform1, nullptr);
    EXPECT_EQ(transform1->getPosition(), Vec2(100.0f, 200.0f));
    EXPECT_EQ(transform1->getScale(), Vec2(2.0f, 2.0f));

    auto gravity1 = physicsEntity->getComponent<CGravity>();
    ASSERT_NE(gravity1, nullptr);
    EXPECT_FLOAT_EQ(gravity1->getMultiplier(), 1.5f);

    auto collider1 = physicsEntity->getComponent<CCircleCollider>();
    ASSERT_NE(collider1, nullptr);
    EXPECT_FLOAT_EQ(collider1->getRadius(), 3.0f);
    EXPECT_FALSE(collider1->isTrigger());

    // Verify named_object
    auto namedObjects = EntityManager::instance().getEntitiesByTag("named_object");
    ASSERT_EQ(namedObjects.size(), 1);
    auto namedEntity = namedObjects[0];

    auto transform2 = namedEntity->getComponent<CTransform>();
    ASSERT_NE(transform2, nullptr);
    EXPECT_EQ(transform2->getPosition(), Vec2(-50.0f, 75.0f));

    auto name2 = namedEntity->getComponent<CName>();
    ASSERT_NE(name2, nullptr);
    EXPECT_EQ(name2->getName(), "TestObject");
}

TEST_F(SceneManagerTest, SaveCurrentScene)
{
    const std::string testFile = "tests/test_data/test_scene.json";

    // Create and save a test scene
    createTestScene();
    auto& sceneManager = SceneManager::instance();

    // First save to establish current scene
    EXPECT_NO_THROW(sceneManager.saveScene(testFile));

    // Modify scene
    auto entity = EntityManager::instance().addEntity("new_object");
    entity->addComponent<CName>()->setName("NewObject");
    EntityManager::instance().update(0.0f);  // Process the new entity

    // Save current scene
    EXPECT_NO_THROW(sceneManager.saveCurrentScene());

    // Clear and reload
    EntityManager::instance().clear();
    EntityManager::instance().update(0.0f);  // Process the clear
    EXPECT_NO_THROW(sceneManager.loadScene(testFile));
    EntityManager::instance().update(0.0f);  // Process loaded entities

    // Verify new entity was saved
    auto newObjects = EntityManager::instance().getEntitiesByTag("new_object");
    ASSERT_EQ(newObjects.size(), 1);
    EXPECT_EQ(newObjects[0]->getComponent<CName>()->getName(), "NewObject");
}

TEST_F(SceneManagerTest, ErrorHandling)
{
    auto& sceneManager = SceneManager::instance();

    // Test loading non-existent file
    EXPECT_THROW(sceneManager.loadScene("nonexistent.json"), std::runtime_error);

    sceneManager.clearScene();

    // Test saving current scene when no scene is loaded
    EXPECT_THROW(sceneManager.saveCurrentScene(), std::runtime_error);

    // Test loading invalid file path
    EXPECT_THROW(sceneManager.loadScene("/invalid/path/scene.json"), std::runtime_error);
}

TEST_F(SceneManagerTest, SceneTransitions)
{
    const std::string testFile1 = "tests/test_data/test_scene1.json";
    const std::string testFile2 = "tests/test_data/test_scene2.json";

    auto& sceneManager = SceneManager::instance();

    // Create and save first scene
    createTestScene();
    EXPECT_NO_THROW(sceneManager.saveScene(testFile1));

    // Create and save second scene
    EntityManager::instance().clear();
    EntityManager::instance().update(0.0f);  // Process the clear
    auto entity = EntityManager::instance().addEntity("unique_object");
    entity->addComponent<CName>()->setName("UniqueObject");
    EntityManager::instance().update(0.0f);  // Process the new entity
    EXPECT_NO_THROW(sceneManager.saveScene(testFile2));

    // Test transitioning between scenes
    EXPECT_NO_THROW(sceneManager.loadScene(testFile1));
    EntityManager::instance().update(0.0f);  // Process loaded entities
    ASSERT_EQ(EntityManager::instance().getEntitiesByTag("physics_object").size(), 1);

    EXPECT_NO_THROW(sceneManager.loadScene(testFile2));
    EntityManager::instance().update(0.0f);  // Process loaded entities
    ASSERT_EQ(EntityManager::instance().getEntitiesByTag("unique_object").size(), 1);

    // Clean up second test file
    if (std::filesystem::exists(testFile1))
        std::filesystem::remove(testFile1);
    if (std::filesystem::exists(testFile2))
        std::filesystem::remove(testFile2);
}

TEST_F(SceneManagerTest, ClearScene)
{
    auto& sceneManager = SceneManager::instance();

    // Create a test scene
    createTestScene();

    // Clear the scene
    sceneManager.clearScene();

    // Verify the scene is cleared
    EXPECT_TRUE(EntityManager::instance().getEntities().empty());
}
