#include <gtest/gtest.h>
#include <filesystem>
#include "CCircleCollider.h"
#include "CGravity.h"
#include "CName.h"
#include "CTransform.h"
#include "Component.h"
#include "Entity.h"
#include "EntityManager.h"
#include "TestUtils.h"
#include "Vec2.h"

// Define the source directory path
#ifndef SOURCE_DIR
#define SOURCE_DIR "."
#endif

// Test fixture for entity manager tests
class EntityManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Clear the EntityManager before each test
        EntityManager::instance().clear();
    }

    void TearDown() override {}
};

TEST_F(EntityManagerTest, EntityCreation)
{
    auto&                   manager = EntityManager::instance();
    std::shared_ptr<Entity> entity  = manager.addEntity("test");
    EXPECT_NE(entity, nullptr);
    EXPECT_EQ(entity->getTag(), "test");
    EXPECT_TRUE(entity->isAlive());
}

TEST_F(EntityManagerTest, EntityRemoval)
{
    auto&                   manager = EntityManager::instance();
    std::shared_ptr<Entity> entity  = manager.addEntity("test");

    manager.update(0.0f);  // Process pending entities
    EXPECT_EQ(manager.getEntities().size(), 1);

    manager.removeEntity(entity);
    manager.update(0.0f);  // Process removals
    EXPECT_EQ(manager.getEntities().size(), 0);
}

TEST_F(EntityManagerTest, EntityTagging)
{
    auto& manager = EntityManager::instance();
    manager.addEntity("typeA");
    manager.addEntity("typeA");
    manager.addEntity("typeB");

    manager.update(0.0f);  // Process pending entities

    std::vector<std::shared_ptr<Entity>> typeAEntities = manager.getEntitiesByTag("typeA");
    std::vector<std::shared_ptr<Entity>> typeBEntities = manager.getEntitiesByTag("typeB");

    EXPECT_EQ(typeAEntities.size(), 2);
    EXPECT_EQ(typeBEntities.size(), 1);
}

TEST_F(EntityManagerTest, EntityComponentQuery)
{
    auto& manager = EntityManager::instance();

    std::shared_ptr<Entity> entity1 = manager.addEntity("test1");
    entity1->addComponent<CTransform>();
    entity1->addComponent<CCircleCollider>(2.0f);

    std::shared_ptr<Entity> entity2 = manager.addEntity("test2");
    entity2->addComponent<CTransform>();
    entity2->addComponent<CGravity>();
    entity2->addComponent<CCircleCollider>(3.0f);

    manager.update(0.0f);  // Process pending entities

    auto entitiesWithTransform = manager.getEntitiesWithComponent<CTransform>();
    auto entitiesWithGravity   = manager.getEntitiesWithComponent<CGravity>();
    auto entitiesWithCollider  = manager.getEntitiesWithComponent<CCircleCollider>();

    EXPECT_EQ(entitiesWithTransform.size(), 2);
    EXPECT_EQ(entitiesWithGravity.size(), 1);
    EXPECT_EQ(entitiesWithCollider.size(), 2);
}

TEST_F(EntityManagerTest, EntityUpdateSystem)
{
    auto& manager   = EntityManager::instance();
    auto  entity    = manager.addEntity("test");
    auto  transform = entity->addComponent<CTransform>();
    auto  gravity   = entity->addComponent<CGravity>();

    const float EPSILON = 0.0001f;  // Small value for floating point comparison
    EXPECT_NEAR(gravity->getForce().x, 0.0f, EPSILON);
    EXPECT_NEAR(gravity->getForce().y, -9.81f, EPSILON);

    // Test system update
    float deltaTime = 1.0f;
    manager.update(deltaTime);
}

TEST_F(EntityManagerTest, EntitySerialization)
{
    auto& manager = EntityManager::instance();

    // Create first entity with Transform, Gravity and CircleCollider
    auto entity1    = manager.addEntity("physics_object");
    auto transform1 = entity1->addComponent<CTransform>();
    transform1->setPosition(Vec2(100.0f, 200.0f));
    transform1->setScale(Vec2(2.0f, 2.0f));
    transform1->setRotation(45.0f);
    auto gravity1 = entity1->addComponent<CGravity>();
    gravity1->setForce(Vec2(0.0f, -15.0f));
    auto collider1 = entity1->addComponent<CCircleCollider>(3.0f);
    collider1->setTrigger(true);

    // Create second entity with Transform and Name
    auto entity2    = manager.addEntity("named_object");
    auto transform2 = entity2->addComponent<CTransform>();
    transform2->setPosition(Vec2(-50.0f, 75.0f));
    auto name2 = entity2->addComponent<CName>();
    name2->setName("TestObject");

    // Create third entity with all components
    auto entity3    = manager.addEntity("complete_object");
    auto transform3 = entity3->addComponent<CTransform>();
    transform3->setPosition(Vec2(300.0f, -200.0f));
    transform3->setRotation(90.0f);
    auto gravity3 = entity3->addComponent<CGravity>();
    gravity3->setForce(Vec2(5.0f, -9.81f));
    auto name3 = entity3->addComponent<CName>();
    name3->setName("CompleteObject");
    auto collider3 = entity3->addComponent<CCircleCollider>(5.0f);

    // Process pending entities
    manager.update(0.0f);

    // Save to file
    std::string testFile = std::string(SOURCE_DIR) + "/test_entities.json";
    manager.saveToFile(testFile);

    // Read and parse the saved JSON file
    std::string json = readFile(testFile);
    JsonValue   root(json);

    // Test entities array
    const auto& entities = root["entities"].getArray();
    ASSERT_EQ(entities.size(), 3);

    // Find physics_object entity and verify its components
    const auto& physics = entities[0];
    EXPECT_EQ(physics["tag"].getString(), "physics_object");
    const auto& physicsComponents = physics["components"].getArray();

    // Find and verify Transform component
    const JsonValue* transformData = nullptr;
    for (const auto& comp : physicsComponents) {
        if (!comp["cTransform"].isNull()) {
            transformData = &comp["cTransform"];
            break;
        }
    }
    ASSERT_NE(transformData, nullptr);
    const auto& pos   = (*transformData)["position"];
    const auto& scale = (*transformData)["scale"];
    EXPECT_TRUE(approxEqual(pos["x"].getNumber(), 100.0));
    EXPECT_TRUE(approxEqual(pos["y"].getNumber(), 200.0));
    EXPECT_TRUE(approxEqual(scale["x"].getNumber(), 2.0));
    EXPECT_TRUE(approxEqual(scale["y"].getNumber(), 2.0));
    EXPECT_TRUE(approxEqual((*transformData)["rotation"].getNumber(), 45.0));

    // Find and verify Gravity component
    const JsonValue* gravityData = nullptr;
    for (const auto& comp : physicsComponents) {
        if (!comp["cGravity"].isNull()) {
            gravityData = &comp["cGravity"];
            break;
        }
    }
    ASSERT_NE(gravityData, nullptr);
    const auto& force = (*gravityData)["force"];
    EXPECT_TRUE(approxEqual(force["x"].getNumber(), 0.0));
    EXPECT_TRUE(approxEqual(force["y"].getNumber(), -15.0));

    // Find and verify CircleCollider component
    const JsonValue* colliderData = nullptr;
    for (const auto& comp : physicsComponents) {
        if (!comp["cCircleCollider"].isNull()) {
            colliderData = &comp["cCircleCollider"];
            break;
        }
    }
    ASSERT_NE(colliderData, nullptr);
    EXPECT_TRUE(approxEqual((*colliderData)["radius"].getNumber(), 3.0f));
    EXPECT_TRUE((*colliderData)["trigger"].getBool());

    // Find named_object entity
    const auto& named = entities[1];
    EXPECT_EQ(named["tag"].getString(), "named_object");
    const auto& namedComponents = named["components"].getArray();

    // Find and verify Transform component
    const JsonValue* transform2Data = nullptr;
    for (const auto& comp : namedComponents) {
        if (!comp["cTransform"].isNull()) {
            transform2Data = &comp["cTransform"];
            break;
        }
    }
    ASSERT_NE(transform2Data, nullptr);
    const auto& pos2 = (*transform2Data)["position"];
    EXPECT_TRUE(approxEqual(pos2["x"].getNumber(), -50.0));
    EXPECT_TRUE(approxEqual(pos2["y"].getNumber(), 75.0));

    // Find and verify Name component
    const JsonValue* name2Data = nullptr;
    for (const auto& comp : namedComponents) {
        if (!comp["cName"].isNull()) {
            name2Data = &comp["cName"];
            break;
        }
    }
    ASSERT_NE(name2Data, nullptr);
    EXPECT_EQ((*name2Data)["name"].getString(), "TestObject");

    // Find complete_object entity and verify its components
    const auto& complete = entities[2];
    EXPECT_EQ(complete["tag"].getString(), "complete_object");
    const auto& completeComponents = complete["components"].getArray();

    // Find and verify Transform component
    const JsonValue* transform3Data = nullptr;
    for (const auto& comp : completeComponents) {
        if (!comp["cTransform"].isNull()) {
            transform3Data = &comp["cTransform"];
            break;
        }
    }
    ASSERT_NE(transform3Data, nullptr);
    const auto& pos3 = (*transform3Data)["position"];
    EXPECT_TRUE(approxEqual(pos3["x"].getNumber(), 300.0));
    EXPECT_TRUE(approxEqual(pos3["y"].getNumber(), -200.0));
    EXPECT_TRUE(approxEqual((*transform3Data)["rotation"].getNumber(), 90.0));

    // Find and verify Gravity component
    const JsonValue* gravity3JsonData = nullptr;
    for (const auto& comp : completeComponents) {
        if (!comp["cGravity"].isNull()) {
            gravity3JsonData = &comp["cGravity"];
            break;
        }
    }
    ASSERT_NE(gravity3JsonData, nullptr);
    const auto& force3 = (*gravity3JsonData)["force"];
    EXPECT_TRUE(approxEqual(force3["x"].getNumber(), 5.0));
    EXPECT_TRUE(approxEqual(force3["y"].getNumber(), -9.81));

    // Find and verify Name component
    const JsonValue* name3JsonData = nullptr;
    for (const auto& comp : completeComponents) {
        if (!comp["cName"].isNull()) {
            name3JsonData = &comp["cName"];
            break;
        }
    }
    ASSERT_NE(name3JsonData, nullptr);
    EXPECT_EQ((*name3JsonData)["name"].getString(), "CompleteObject");

    // Find and verify CircleCollider component
    const JsonValue* collider3JsonData = nullptr;
    for (const auto& comp : completeComponents) {
        if (!comp["cCircleCollider"].isNull()) {
            collider3JsonData = &comp["cCircleCollider"];
            break;
        }
    }
    ASSERT_NE(collider3JsonData, nullptr);
    EXPECT_TRUE(approxEqual((*collider3JsonData)["radius"].getNumber(), 5.0f));
    EXPECT_FALSE((*collider3JsonData)["trigger"].getBool());

    // Clean up
    std::filesystem::remove(testFile);
}

TEST_F(EntityManagerTest, SaveAndLoadEntities)
{
    auto& manager = EntityManager::instance();

    // Create first entity with Transform and Gravity
    auto entity1    = manager.addEntity("physics_object");
    auto transform1 = entity1->addComponent<CTransform>();
    transform1->setPosition(Vec2(100.0f, 200.0f));
    transform1->setVelocity(Vec2(10.0f, -5.0f));
    transform1->setScale(Vec2(2.0f, 2.0f));
    transform1->setRotation(45.0f);
    auto gravity1 = entity1->addComponent<CGravity>();
    gravity1->setForce(Vec2(0.0f, -15.0f));

    // Create second entity with Transform and Name
    auto entity2    = manager.addEntity("named_object");
    auto transform2 = entity2->addComponent<CTransform>();
    transform2->setPosition(Vec2(-50.0f, 75.0f));
    auto name2 = entity2->addComponent<CName>();
    name2->setName("TestObject");

    // Process pending entities
    manager.update(0.0f);

    // Save to file
    std::string testFile = "tests/test_data/test_entities.json";
    manager.saveToFile(testFile);

    // Create a new manager and load the file
    EntityManager::instance().clear();  // Clear current state
    manager.loadFromFile(testFile);
    manager.update(0.0f);  // Process loaded entities

    // Verify loaded entities
    const auto& loadedEntities = manager.getEntities();
    ASSERT_EQ(loadedEntities.size(), 2);

    // Find and verify physics_object
    auto physicsObjects = manager.getEntitiesByTag("physics_object");
    ASSERT_EQ(physicsObjects.size(), 1);
    auto loadedPhysics = physicsObjects[0];

    auto loadedTransform1 = loadedPhysics->getComponent<CTransform>();
    ASSERT_NE(loadedTransform1, nullptr);
    EXPECT_EQ(loadedTransform1->getPosition(), Vec2(100.0f, 200.0f));
    EXPECT_EQ(loadedTransform1->getVelocity(), Vec2(10.0f, -5.0f));
    EXPECT_EQ(loadedTransform1->getScale(), Vec2(2.0f, 2.0f));
    EXPECT_FLOAT_EQ(loadedTransform1->getRotation(), 45.0f);

    auto loadedGravity1 = loadedPhysics->getComponent<CGravity>();
    ASSERT_NE(loadedGravity1, nullptr);
    EXPECT_EQ(loadedGravity1->getForce(), Vec2(0.0f, -15.0f));

    // Find and verify named_object
    auto namedObjects = manager.getEntitiesByTag("named_object");
    ASSERT_EQ(namedObjects.size(), 1);
    auto loadedNamed = namedObjects[0];

    auto loadedTransform2 = loadedNamed->getComponent<CTransform>();
    ASSERT_NE(loadedTransform2, nullptr);
    EXPECT_EQ(loadedTransform2->getPosition(), Vec2(-50.0f, 75.0f));

    auto loadedName2 = loadedNamed->getComponent<CName>();
    ASSERT_NE(loadedName2, nullptr);
    EXPECT_EQ(loadedName2->getName(), "TestObject");

    // Clean up
    std::filesystem::remove(testFile);
}