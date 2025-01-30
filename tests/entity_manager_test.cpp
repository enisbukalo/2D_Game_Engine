#include <gtest/gtest.h>
#include "Component.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Vec2.h"
#include "components/CGravity.h"
#include "components/CName.h"
#include "components/CTransform.h"
#include "test_utils.h"

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
    EXPECT_NEAR(gravity->getForce().x, 0.0f, EPSILON);
    EXPECT_NEAR(gravity->getForce().y, -9.81f, EPSILON);

    // Test system update
    float deltaTime = 1.0f;
    m_manager.update(deltaTime);
}

TEST_F(EntityManagerTest, EntitySerialization)
{
    // Create first entity with Transform and Gravity
    auto entity1    = m_manager.addEntity("physics_object");
    auto transform1 = entity1->addComponent<CTransform>();
    transform1->setPosition(Vec2(100.0f, 200.0f));
    transform1->setScale(Vec2(2.0f, 2.0f));
    transform1->setRotation(45.0f);
    auto gravity1 = entity1->addComponent<CGravity>();
    gravity1->setForce(Vec2(0.0f, -15.0f));

    // Create second entity with Transform and Name
    auto entity2    = m_manager.addEntity("named_object");
    auto transform2 = entity2->addComponent<CTransform>();
    transform2->setPosition(Vec2(-50.0f, 75.0f));
    auto name2 = entity2->addComponent<CName>();
    name2->setName("TestObject");

    // Create third entity with all components
    auto entity3    = m_manager.addEntity("complete_object");
    auto transform3 = entity3->addComponent<CTransform>();
    transform3->setPosition(Vec2(300.0f, -200.0f));
    transform3->setRotation(90.0f);
    auto gravity3 = entity3->addComponent<CGravity>();
    gravity3->setForce(Vec2(5.0f, -9.81f));
    auto name3 = entity3->addComponent<CName>();
    name3->setName("CompleteObject");

    // Process pending entities
    m_manager.update(0.0f);

    // Save to file
    std::string testFile = std::string(SOURCE_DIR) + "/test_entities.json";
    m_manager.saveToFile(testFile);

    // Read and parse the saved JSON file
    std::string json = readFile(testFile);
    JsonValue   root(json);

    // Test entities array
    const auto& entities = root["entities"].getArray();
    ASSERT_EQ(entities.size(), 3);

    // Find physics_object entity
    const auto& physics = entities[0];
    EXPECT_EQ(physics["tag"].getString(), "physics_object");
    const auto& physicsComponents = physics["components"].getArray();

    // Verify Transform component
    const auto& transform = physicsComponents[0]["cTransform"];
    const auto& pos       = transform["position"];
    const auto& scale     = transform["scale"];
    EXPECT_TRUE(approxEqual(pos["x"].getNumber(), 100.0));
    EXPECT_TRUE(approxEqual(pos["y"].getNumber(), 200.0));
    EXPECT_TRUE(approxEqual(scale["x"].getNumber(), 2.0));
    EXPECT_TRUE(approxEqual(scale["y"].getNumber(), 2.0));
    EXPECT_TRUE(approxEqual(transform["rotation"].getNumber(), 45.0));

    // Verify Gravity component
    const auto& gravity = physicsComponents[1]["cGravity"];
    const auto& force   = gravity["force"];
    EXPECT_TRUE(approxEqual(force["x"].getNumber(), 0.0));
    EXPECT_TRUE(approxEqual(force["y"].getNumber(), -15.0));

    // Find named_object entity
    const auto& named = entities[1];
    EXPECT_EQ(named["tag"].getString(), "named_object");
    const auto& namedComponents = named["components"].getArray();

    // Verify Transform component
    const auto& transform2Data = namedComponents[0]["cTransform"];
    const auto& pos2           = transform2Data["position"];
    EXPECT_TRUE(approxEqual(pos2["x"].getNumber(), -50.0));
    EXPECT_TRUE(approxEqual(pos2["y"].getNumber(), 75.0));

    // Verify Name component
    const auto& name = namedComponents[1]["cName"];
    EXPECT_EQ(name["name"].getString(), "TestObject");

    // Find complete_object entity
    const auto& complete = entities[2];
    EXPECT_EQ(complete["tag"].getString(), "complete_object");
    const auto& completeComponents = complete["components"].getArray();

    // Verify Transform component
    const auto& transform3Data = completeComponents[0]["cTransform"];
    const auto& pos3           = transform3Data["position"];
    EXPECT_TRUE(approxEqual(pos3["x"].getNumber(), 300.0));
    EXPECT_TRUE(approxEqual(pos3["y"].getNumber(), -200.0));
    EXPECT_TRUE(approxEqual(transform3Data["rotation"].getNumber(), 90.0));

    // Verify Gravity component
    const auto& gravity3Data = completeComponents[1]["cGravity"];
    const auto& force3       = gravity3Data["force"];
    EXPECT_TRUE(approxEqual(force3["x"].getNumber(), 5.0));
    EXPECT_TRUE(approxEqual(force3["y"].getNumber(), -9.81));

    // Verify Name component
    const auto& name3Data = completeComponents[2]["cName"];
    EXPECT_EQ(name3Data["name"].getString(), "CompleteObject");
}