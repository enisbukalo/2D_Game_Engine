#include <gtest/gtest.h>
#include <filesystem>
#include "CName.h"
#include "CTransform.h"
#include "Component.h"
#include "Entity.h"
#include "SEntity.h"
#include "TestUtils.h"
#include "Vec2.h"
#include "CPhysicsBody2D.h"
#include "CCollider2D.h"
#include "CInputController.h"
#include "Input/KeyCode.h"
#include "SInput.h"

using namespace Components;
using namespace Entity;
using namespace Systems;

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
        SEntity::instance().clear();
        // Initialize SInputManager for components that rely on it (e.g., CInputController)
        SInput::instance().shutdown();
        SInput::instance().initialize(nullptr, false);
    }

    void TearDown() override
    {
        // Shutdown SInputManager after tests
        SInput::instance().shutdown();
    }
};

TEST_F(EntityManagerTest, EntityCreation)
{
    auto&                   manager = SEntity::instance();
    std::shared_ptr<Entity::Entity> entity  = manager.addEntity("test");
    EXPECT_NE(entity, nullptr);
    EXPECT_EQ(entity->getTag(), "test");
    EXPECT_TRUE(entity->isAlive());
}

TEST_F(EntityManagerTest, EntityRemoval)
{
    auto&                   manager = SEntity::instance();
    std::shared_ptr<Entity::Entity> entity  = manager.addEntity("test");

    manager.update(0.0f);  // Process pending entities
    EXPECT_EQ(manager.getEntities().size(), 1);

    manager.removeEntity(entity);
    manager.update(0.0f);  // Process removals
    EXPECT_EQ(manager.getEntities().size(), 0);
}

TEST_F(EntityManagerTest, EntityTagging)
{
    auto& manager = SEntity::instance();
    manager.addEntity("typeA");
    manager.addEntity("typeA");
    manager.addEntity("typeB");

    manager.update(0.0f);  // Process pending entities

    std::vector<std::shared_ptr<Entity::Entity>> typeAEntities = manager.getEntitiesByTag("typeA");
    std::vector<std::shared_ptr<Entity::Entity>> typeBEntities = manager.getEntitiesByTag("typeB");

    EXPECT_EQ(typeAEntities.size(), 2);
    EXPECT_EQ(typeBEntities.size(), 1);
}

TEST_F(EntityManagerTest, EntityComponentQuery)
{
    auto& manager = SEntity::instance();

    std::shared_ptr<Entity::Entity> entity1 = manager.addEntity("test1");
    entity1->addComponent<CTransform>();
    entity1->addComponent<CName>();

    std::shared_ptr<Entity::Entity> entity2 = manager.addEntity("test2");
    entity2->addComponent<CTransform>();
    entity2->addComponent<CName>();

    manager.update(0.0f);  // Process pending entities

    auto entitiesWithTransform = manager.getEntitiesWithComponent<CTransform>();
    auto entitiesWithName      = manager.getEntitiesWithComponent<CName>();

    EXPECT_EQ(entitiesWithTransform.size(), 2);
    EXPECT_EQ(entitiesWithName.size(), 2);
}

TEST_F(EntityManagerTest, EntityUpdateSystem)
{
    auto& manager   = SEntity::instance();
    auto  entity    = manager.addEntity("test");
    auto  transform = entity->addComponent<CTransform>();
    auto  name      = entity->addComponent<CName>();

    EXPECT_NE(name, nullptr);

    // Test system update
    float deltaTime = 1.0f;
    manager.update(deltaTime);
}

TEST_F(EntityManagerTest, EntitySerialization)
{
    auto& manager = SEntity::instance();

    // Create first entity with Transform
    auto entity1    = manager.addEntity("transform_object");
    auto transform1 = entity1->addComponent<CTransform>();
    transform1->setPosition(Vec2(100.0f, 200.0f));
    transform1->setScale(Vec2(2.0f, 2.0f));
    transform1->setRotation(45.0f);

    // Create second entity with Transform and Name
    auto entity2    = manager.addEntity("named_object");
    auto transform2 = entity2->addComponent<CTransform>();
    transform2->setPosition(Vec2(-50.0f, 75.0f));
    auto name2 = entity2->addComponent<CName>();
    name2->setName("TestObject");

    // Create third entity with Transform and Name
    auto entity3    = manager.addEntity("complete_object");
    auto transform3 = entity3->addComponent<CTransform>();
    transform3->setPosition(Vec2(300.0f, -200.0f));
    transform3->setRotation(90.0f);
    auto name3 = entity3->addComponent<CName>();
    name3->setName("CompleteObject");

    // Create fourth entity with C2DPhysicsBody
    auto entity4       = manager.addEntity("physics_object");
    auto transform4    = entity4->addComponent<CTransform>();   
    transform4->setPosition(Vec2(0.0f, 0.0f));
    auto physicsBody4  = entity4->addComponent<CPhysicsBody2D>();
    physicsBody4->initialize({0.0f, 0.0f});
    physicsBody4->setBodyType(BodyType::Dynamic);
    physicsBody4->setDensity(1.0f);
    physicsBody4->setFriction(0.5f);
    physicsBody4->setRestitution(0.2f);
    physicsBody4->setFixedRotation(false);
    physicsBody4->setLinearDamping(0.1f);
    physicsBody4->setAngularDamping(0.1f);
    physicsBody4->setGravityScale(1.0f);
    auto collider4 = entity4->addComponent<CCollider2D>();
    collider4->createCircle(5.0f);

    // Create fifth entity with CInputController and test action binding serialization
    auto entity5       = manager.addEntity("controller_object");
    auto transform5    = entity5->addComponent<CTransform>();
    transform5->setPosition(Vec2(0.0f, 0.0f));
    auto controller5   = entity5->addComponent<CInputController>();
    controller5->init();
    ActionBinding controllerBinding;
    controllerBinding.keys.push_back(KeyCode::Space);
    controllerBinding.trigger = ActionTrigger::Pressed;
    controller5->bindAction("Jump", controllerBinding);

    // Process pending entities
    manager.update(0.0f);

    // Save to file
    std::string testFile = std::string(SOURCE_DIR) + "/test_entities.json";
    manager.saveToFile(testFile);

    // Read and parse the saved JSON file
    std::string json = readFile(testFile);
   Serialization::SSerialization::JsonValue   root(json);

    // Test entities array
    const auto& entities = root["entities"].getArray();
    ASSERT_EQ(entities.size(), 5);

    // Find transform_object entity and verify its components
    const auto& physics = entities[0];
    EXPECT_EQ(physics["tag"].getString(), "transform_object");
    const auto& physicsComponents = physics["components"].getArray();

    // Find and verify Transform component
    const Serialization::SSerialization::JsonValue* transformData = nullptr;
    for (const auto& comp : physicsComponents)
    {
        if (!comp["cTransform"].isNull())
        {
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

    // Find named_object entity
    const auto& named = entities[1];
    EXPECT_EQ(named["tag"].getString(), "named_object");
    const auto& namedComponents = named["components"].getArray();

    // Find and verify Transform component
    const Serialization::SSerialization::JsonValue* transform2Data = nullptr;
    for (const auto& comp : namedComponents)
    {
        if (!comp["cTransform"].isNull())
        {
            transform2Data = &comp["cTransform"];
            break;
        }
    }
    ASSERT_NE(transform2Data, nullptr);
    const auto& pos2 = (*transform2Data)["position"];
    EXPECT_TRUE(approxEqual(pos2["x"].getNumber(), -50.0));
    EXPECT_TRUE(approxEqual(pos2["y"].getNumber(), 75.0));

    // Find and verify Name component
    const Serialization::SSerialization::JsonValue* name2Data = nullptr;
    for (const auto& comp : namedComponents)
    {
        if (!comp["cName"].isNull())
        {
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
    const Serialization::SSerialization::JsonValue* transform3Data = nullptr;
    for (const auto& comp : completeComponents)
    {
        if (!comp["cTransform"].isNull())
        {
            transform3Data = &comp["cTransform"];
            break;
        }
    }
    ASSERT_NE(transform3Data, nullptr);
    const auto& pos3 = (*transform3Data)["position"];
    EXPECT_TRUE(approxEqual(pos3["x"].getNumber(), 300.0));
    EXPECT_TRUE(approxEqual(pos3["y"].getNumber(), -200.0));
    EXPECT_TRUE(approxEqual((*transform3Data)["rotation"].getNumber(), 90.0));

    // Find and verify Name component
    const Serialization::SSerialization::JsonValue* name3JsonData = nullptr;
    for (const auto& comp : completeComponents)
    {
        if (!comp["cName"].isNull())
        {
            name3JsonData = &comp["cName"];
            break;
        }
    }
    ASSERT_NE(name3JsonData, nullptr);
    EXPECT_EQ((*name3JsonData)["name"].getString(), "CompleteObject");

    // Find and verify PhysicsBody2D component
    const auto& physicsObject = entities[3];
    EXPECT_EQ(physicsObject["tag"].getString(), "physics_object");
    const auto& physicsObjectComponents = physicsObject["components"].getArray();
    const Serialization::SSerialization::JsonValue* physicsBody3JsonData = nullptr;
    for (const auto& comp : physicsObjectComponents)
    {
        if (!comp["cPhysicsBody2D"].isNull())
        {
            physicsBody3JsonData = &comp["cPhysicsBody2D"];
            break;
        }
    }
    ASSERT_NE(physicsBody3JsonData, nullptr);
    EXPECT_EQ((*physicsBody3JsonData)["bodyType"].getString(), "Dynamic");
    EXPECT_TRUE(approxEqual((*physicsBody3JsonData)["density"].getNumber(), 1.0f));
    EXPECT_TRUE(approxEqual((*physicsBody3JsonData)["friction"].getNumber(), 0.5f));
    EXPECT_TRUE(approxEqual((*physicsBody3JsonData)["restitution"].getNumber(), 0.2f));
    EXPECT_FALSE((*physicsBody3JsonData)["fixedRotation"].getBool());   
    EXPECT_TRUE(approxEqual((*physicsBody3JsonData)["linearDamping"].getNumber(), 0.1f));
    EXPECT_TRUE(approxEqual((*physicsBody3JsonData)["angularDamping"].getNumber(), 0.1f));
    EXPECT_TRUE(approxEqual((*physicsBody3JsonData)["gravityScale"].getNumber(), 1.0f));

    // Find and verify CircleCollider component from the PhysicsBody2D entity
    const Serialization::SSerialization::JsonValue* collider4JsonData = nullptr;
    for (const auto& comp : physicsObjectComponents)
    {
        if (!comp["cCollider2D"].isNull())
        {
            collider4JsonData = &comp["cCollider2D"];
            break;
        }
    }
    ASSERT_NE(collider4JsonData, nullptr);
    // Check the new fixtures array format
    ASSERT_TRUE((*collider4JsonData)["fixtures"].isArray());
    const auto& fixtures = (*collider4JsonData)["fixtures"].getArray();
    ASSERT_GT(fixtures.size(), 0);
    // Verify first fixture is a circle with radius 5.0
    EXPECT_EQ(fixtures[0]["shapeType"].getString(), "Circle");
    EXPECT_TRUE(approxEqual(fixtures[0]["radius"].getNumber(), 5.0f));

    // Find controller entity and verify it has cInputController
    const Serialization::SSerialization::JsonValue* controllerJsonData = nullptr;
    for (const auto& ent : entities)
    {
        if (ent["tag"].getString() == "controller_object")
        {
            const auto& comps = ent["components"].getArray();
            for (const auto& comp : comps)
            {
                if (!comp["cInputController"].isNull())
                {
                    controllerJsonData = &comp["cInputController"];
                    break;
                }
            }
            break;
        }
    }
    ASSERT_NE(controllerJsonData, nullptr);
    // Check that it contains an action named "Jump"
    const auto& actionsArray = (*controllerJsonData)["actions"].getArray();
    bool foundJump = false;
    for (const auto& actionObj : actionsArray)
    {
        if (actionObj["action"].getString() == "Jump") { foundJump = true; break; }
    }
    EXPECT_TRUE(foundJump);

    // Clean up
    std::filesystem::remove(testFile);
}

TEST_F(EntityManagerTest, SaveAndLoadEntities)
{
    auto& manager = SEntity::instance();

    // Create first entity with Transform and Gravity
    auto entity1    = manager.addEntity("physics_object");
    auto transform1 = entity1->addComponent<CTransform>();
    transform1->setPosition(Vec2(100.0f, 200.0f));
    transform1->setVelocity(Vec2(10.0f, -5.0f));
    transform1->setScale(Vec2(2.0f, 2.0f));
    transform1->setRotation(45.0f);

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
    SEntity::instance().clear();  // Clear current state
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