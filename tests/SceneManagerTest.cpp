#include <gtest/gtest.h>
#include <filesystem>
#include "CName.h"
#include "CTransform.h"
#include "CPhysicsBody2D.h"
#include "CCollider2D.h"
#include "SEntity.h"
#include "SScene.h"
#include "TestUtils.h"

// Test fixture for scene manager tests
class SceneManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        SEntity::instance().clear();

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
        auto& manager = SEntity::instance();

        // Create an entity with Transform, PhysicsBody2D and Collider2D
        auto entity1    = manager.addEntity("physics_object");
        auto transform1 = entity1->addComponent<CTransform>();
        transform1->setPosition(Vec2(100.0f, 200.0f));
        transform1->setScale(Vec2(2.0f, 2.0f));
        auto physicsBody1 = entity1->addComponent<CPhysicsBody2D>();
        physicsBody1->initialize({100.0f, 200.0f});
        auto collider1 = entity1->addComponent<CCollider2D>();
        collider1->createCircle(3.0f);

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
    auto& sceneManager = SScene::instance();
    EXPECT_NO_THROW(sceneManager.saveScene(testFile));
    EXPECT_EQ(sceneManager.getCurrentScenePath(), testFile);

    // Clear the entity manager
    SEntity::instance().clear();
    SEntity::instance().update(0.0f);  // Process the clear
    EXPECT_TRUE(SEntity::instance().getEntities().empty());

    // Load the scene back
    EXPECT_NO_THROW(sceneManager.loadScene(testFile));
    SEntity::instance().update(0.0f);  // Process loaded entities
    EXPECT_EQ(sceneManager.getCurrentScenePath(), testFile);

    // Verify loaded entities
    const auto& entities = SEntity::instance().getEntities();
    ASSERT_EQ(entities.size(), 2);

    // Verify physics_object
    auto physicsObjects = SEntity::instance().getEntitiesByTag("physics_object");
    ASSERT_EQ(physicsObjects.size(), 1);
    auto physicsEntity = physicsObjects[0];

    auto transform1 = physicsEntity->getComponent<CTransform>();
    ASSERT_NE(transform1, nullptr);
    EXPECT_EQ(transform1->getPosition(), Vec2(100.0f, 200.0f));
    EXPECT_EQ(transform1->getScale(), Vec2(2.0f, 2.0f));

    auto physicsBody1 = physicsEntity->getComponent<CPhysicsBody2D>();
    ASSERT_NE(physicsBody1, nullptr);

    auto collider1 = physicsEntity->getComponent<CCollider2D>();
    ASSERT_NE(collider1, nullptr);
    EXPECT_FLOAT_EQ(collider1->getCircleRadius(), 3.0f);
    EXPECT_FALSE(collider1->isSensor());

    // Verify named_object
    auto namedObjects = SEntity::instance().getEntitiesByTag("named_object");
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
    auto& sceneManager = SScene::instance();

    // First save to establish current scene
    EXPECT_NO_THROW(sceneManager.saveScene(testFile));

    // Modify scene
    auto entity = SEntity::instance().addEntity("new_object");
    entity->addComponent<CName>()->setName("NewObject");
    SEntity::instance().update(0.0f);  // Process the new entity

    // Save current scene
    EXPECT_NO_THROW(sceneManager.saveCurrentScene());

    // Clear and reload
    SEntity::instance().clear();
    SEntity::instance().update(0.0f);  // Process the clear
    EXPECT_NO_THROW(sceneManager.loadScene(testFile));
    SEntity::instance().update(0.0f);  // Process loaded entities

    // Verify new entity was saved
    auto newObjects = SEntity::instance().getEntitiesByTag("new_object");
    ASSERT_EQ(newObjects.size(), 1);
    EXPECT_EQ(newObjects[0]->getComponent<CName>()->getName(), "NewObject");
}

TEST_F(SceneManagerTest, ErrorHandling)
{
    auto& sceneManager = SScene::instance();

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

    auto& sceneManager = SScene::instance();

    // Create and save first scene
    createTestScene();
    EXPECT_NO_THROW(sceneManager.saveScene(testFile1));

    // Create and save second scene
    SEntity::instance().clear();
    SEntity::instance().update(0.0f);  // Process the clear
    auto entity = SEntity::instance().addEntity("unique_object");
    entity->addComponent<CName>()->setName("UniqueObject");
    SEntity::instance().update(0.0f);  // Process the new entity
    EXPECT_NO_THROW(sceneManager.saveScene(testFile2));

    // Test transitioning between scenes
    EXPECT_NO_THROW(sceneManager.loadScene(testFile1));
    SEntity::instance().update(0.0f);  // Process loaded entities
    ASSERT_EQ(SEntity::instance().getEntitiesByTag("physics_object").size(), 1);

    EXPECT_NO_THROW(sceneManager.loadScene(testFile2));
    SEntity::instance().update(0.0f);  // Process loaded entities
    ASSERT_EQ(SEntity::instance().getEntitiesByTag("unique_object").size(), 1);

    // Clean up second test file
    if (std::filesystem::exists(testFile1))
        std::filesystem::remove(testFile1);
    if (std::filesystem::exists(testFile2))
        std::filesystem::remove(testFile2);
}

TEST_F(SceneManagerTest, ClearScene)
{
    auto& sceneManager = SScene::instance();

    // Create a test scene
    createTestScene();

    // Clear the scene
    sceneManager.clearScene();

    // Verify the scene is cleared
    EXPECT_TRUE(SEntity::instance().getEntities().empty());
}
