#include <gtest/gtest.h>
#include "EntityManager.h"
#include "Entity.h"
#include "CTransform.h"
#include "CPhysicsBody2D.h"
#include "SBox2DPhysics.h"
#include <filesystem>
#include <chrono>
#include <memory>
#include <cmath>

// Helper to compare floats with tolerance
bool floatEquals(float a, float b, float epsilon = 0.001f)
{
    return std::fabs(a - b) < epsilon;
}

// Helper to compare Vec2 with tolerance
bool vec2Equals(const Vec2& a, const Vec2& b, float epsilon = 0.001f)
{
    return floatEquals(a.x, b.x, epsilon) && floatEquals(a.y, b.y, epsilon);
}

// Helper to produce a temp file path unique to this test run
static std::string tempFileName(const std::string& baseName)
{
    auto dir = std::filesystem::temp_directory_path();
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    std::string filename = baseName + "_" + std::to_string(ms) + ".json";
    return (dir / filename).string();
}

class HierarchyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EntityManager::instance().clear();
    }

    void TearDown() override
    {
        EntityManager::instance().clear();
    }
};

// Test basic parent-child relationship
TEST_F(HierarchyTest, BasicParentChildRelationship)
{
    auto parent = EntityManager::instance().addEntity("Parent");
    auto child = EntityManager::instance().addEntity("Child");

    // Set parent
    EXPECT_TRUE(child->setParent(parent));

    // Verify relationship
    EXPECT_EQ(child->getParent(), parent);
    auto children = parent->getChildren();
    EXPECT_EQ(children.size(), 1);
    EXPECT_EQ(children[0], child);
}

// Test multiple children
TEST_F(HierarchyTest, MultipleChildren)
{
    auto parent = EntityManager::instance().addEntity("Parent");
    auto child1 = EntityManager::instance().addEntity("Child1");
    auto child2 = EntityManager::instance().addEntity("Child2");
    auto child3 = EntityManager::instance().addEntity("Child3");

    EXPECT_TRUE(child1->setParent(parent));
    EXPECT_TRUE(child2->setParent(parent));
    EXPECT_TRUE(child3->setParent(parent));

    auto children = parent->getChildren();
    EXPECT_EQ(children.size(), 3);
}

// Test cycle prevention
TEST_F(HierarchyTest, CyclePrevention)
{
    auto entity1 = EntityManager::instance().addEntity("Entity1");
    auto entity2 = EntityManager::instance().addEntity("Entity2");
    auto entity3 = EntityManager::instance().addEntity("Entity3");

    // Create chain: entity1 -> entity2 -> entity3
    EXPECT_TRUE(entity2->setParent(entity1));
    EXPECT_TRUE(entity3->setParent(entity2));

    // Try to create cycle: entity1 -> entity3 (but entity3 is already child of entity2)
    EXPECT_FALSE(entity1->setParent(entity3));

    // Verify relationships unchanged
    EXPECT_EQ(entity2->getParent(), entity1);
    EXPECT_EQ(entity3->getParent(), entity2);
    EXPECT_EQ(entity1->getParent(), nullptr);
}

// Test cascade deletion
TEST_F(HierarchyTest, CascadeDeletion)
{
    auto parent = EntityManager::instance().addEntity("Parent");
    auto child1 = EntityManager::instance().addEntity("Child1");
    auto child2 = EntityManager::instance().addEntity("Child2");
    auto grandchild = EntityManager::instance().addEntity("Grandchild");

    child1->setParent(parent);
    child2->setParent(parent);
    grandchild->setParent(child1);

    // Remove parent should cascade delete all children
    EntityManager::instance().removeEntity(parent);
    EntityManager::instance().update(0.0f);

    // All entities should be marked as dead
    EXPECT_FALSE(parent->isAlive());
    EXPECT_FALSE(child1->isAlive());
    EXPECT_FALSE(child2->isAlive());
    EXPECT_FALSE(grandchild->isAlive());
}

// Test local and world transforms
TEST_F(HierarchyTest, LocalAndWorldTransforms)
{
    auto parent = EntityManager::instance().addEntity("Parent");
    auto child = EntityManager::instance().addEntity("Child");

    auto parentTransform = parent->addComponent<CTransform>(Vec2(100, 100), Vec2(2, 2), 0.0f);
    auto childTransform = child->addComponent<CTransform>(Vec2(50, 0), Vec2(1, 1), 0.0f);

    child->setParent(parent);

    // Child's local position
    EXPECT_TRUE(vec2Equals(childTransform->getLocalPosition(), Vec2(50, 0)));

    // Child's world position should be parent's position + (local * parent's scale)
    Vec2 expectedWorldPos = Vec2(100 + 50 * 2, 100 + 0 * 2); // (200, 100)
    EXPECT_TRUE(vec2Equals(childTransform->getWorldPosition(), expectedWorldPos));

    // Child's world scale should be parent's scale * local scale
    Vec2 expectedWorldScale = Vec2(2 * 1, 2 * 1); // (2, 2)
    EXPECT_TRUE(vec2Equals(childTransform->getWorldScale(), expectedWorldScale));
}

// Test world transform with rotation
TEST_F(HierarchyTest, WorldTransformWithRotation)
{
    auto parent = EntityManager::instance().addEntity("Parent");
    auto child = EntityManager::instance().addEntity("Child");

    float parentRotation = 3.14159f / 2.0f; // 90 degrees
    auto parentTransform = parent->addComponent<CTransform>(Vec2(0, 0), Vec2(1, 1), parentRotation);
    auto childTransform = child->addComponent<CTransform>(Vec2(10, 0), Vec2(1, 1), 0.0f);

    child->setParent(parent);

    // Child's world rotation should be parent's rotation + local rotation
    float expectedWorldRotation = parentRotation + 0.0f;
    EXPECT_TRUE(floatEquals(childTransform->getWorldRotation(), expectedWorldRotation));
}

// Test setting world position updates local position
TEST_F(HierarchyTest, SetWorldPositionUpdatesLocal)
{
    auto parent = EntityManager::instance().addEntity("Parent");
    auto child = EntityManager::instance().addEntity("Child");

    auto parentTransform = parent->addComponent<CTransform>(Vec2(100, 100), Vec2(1, 1), 0.0f);
    auto childTransform = child->addComponent<CTransform>(Vec2(50, 50), Vec2(1, 1), 0.0f);

    child->setParent(parent);

    // Set world position to (200, 200)
    childTransform->setWorldPosition(Vec2(200, 200));

    // Local position should be updated to achieve that world position
    Vec2 expectedLocalPos = Vec2(100, 100); // world - parent = 200,200 - 100,100
    EXPECT_TRUE(vec2Equals(childTransform->getLocalPosition(), expectedLocalPos));
}

// Test nested hierarchy with transforms
TEST_F(HierarchyTest, DeepHierarchyTransforms)
{
    auto root = EntityManager::instance().addEntity("Root");
    auto level1 = EntityManager::instance().addEntity("Level1");
    auto level2 = EntityManager::instance().addEntity("Level2");
    auto level3 = EntityManager::instance().addEntity("Level3");

    auto rootTransform = root->addComponent<CTransform>(Vec2(10, 10), Vec2(2, 2), 0.0f);
    auto level1Transform = level1->addComponent<CTransform>(Vec2(5, 5), Vec2(2, 2), 0.0f);
    auto level2Transform = level2->addComponent<CTransform>(Vec2(3, 3), Vec2(2, 2), 0.0f);
    auto level3Transform = level3->addComponent<CTransform>(Vec2(1, 1), Vec2(2, 2), 0.0f);

    level1->setParent(root);
    level2->setParent(level1);
    level3->setParent(level2);

    // World scale should multiply through the hierarchy
    Vec2 expectedWorldScale = Vec2(2 * 2 * 2 * 2, 2 * 2 * 2 * 2); // (16, 16)
    EXPECT_TRUE(vec2Equals(level3Transform->getWorldScale(), expectedWorldScale));
}

// Test serialization of nested entities
TEST_F(HierarchyTest, SerializeDeserializeNestedEntities)
{
    // Create a hierarchy
    auto parent = EntityManager::instance().addEntity("Parent");
    auto child1 = EntityManager::instance().addEntity("Child1");
    auto child2 = EntityManager::instance().addEntity("Child2");

    parent->addComponent<CTransform>(Vec2(100, 100), Vec2(1, 1), 0.0f);
    child1->addComponent<CTransform>(Vec2(10, 10), Vec2(1, 1), 0.0f);
    child2->addComponent<CTransform>(Vec2(20, 20), Vec2(1, 1), 0.0f);

    child1->setParent(parent);
    child2->setParent(parent);

    std::string parentGuid = parent->getGuid();
    std::string child1Guid = child1->getGuid();
    std::string child2Guid = child2->getGuid();

    // Process pending entities before saving
    EntityManager::instance().update(0.0f);

    // Save to a temp file
    std::string filename = tempFileName("test_hierarchy");
    EntityManager::instance().saveToFile(filename);

    // Clear and reload
    EntityManager::instance().clear();
    EntityManager::instance().loadFromFile(filename);
    EntityManager::instance().update(0.0f);  // Process loaded entities

    // Verify entities were loaded
    auto loadedParent = EntityManager::instance().getEntityByGuid(parentGuid);
    auto loadedChild1 = EntityManager::instance().getEntityByGuid(child1Guid);
    auto loadedChild2 = EntityManager::instance().getEntityByGuid(child2Guid);

    ASSERT_NE(loadedParent, nullptr);
    ASSERT_NE(loadedChild1, nullptr);
    ASSERT_NE(loadedChild2, nullptr);

    // Verify parent-child relationships
    EXPECT_EQ(loadedChild1->getParent(), loadedParent);
    EXPECT_EQ(loadedChild2->getParent(), loadedParent);

    auto children = loadedParent->getChildren();
    EXPECT_EQ(children.size(), 2);

    // No need to manually delete test file - it's in temp location
}

// Test physics body on parent entity
TEST_F(HierarchyTest, PhysicsBodyOnParent)
{
    auto parent = EntityManager::instance().addEntity("Parent");
    auto child = EntityManager::instance().addEntity("Child");

    auto parentTransform = parent->addComponent<CTransform>(Vec2(0, 0), Vec2(1, 1), 0.0f);
    auto childTransform = child->addComponent<CTransform>(Vec2(10, 10), Vec2(1, 1), 0.0f);

    auto physics = parent->addComponent<CPhysicsBody2D>();
    physics->initialize(b2Vec2{0, 0}, BodyType::Dynamic);

    child->setParent(parent);

    // Physics root owner should be the parent
    Entity* physicsRoot = CPhysicsBody2D::getPhysicsRootOwner(child.get());
    EXPECT_EQ(physicsRoot, parent.get());
}

// Test physics body on child entity
TEST_F(HierarchyTest, PhysicsBodyOnChild)
{
    auto parent = EntityManager::instance().addEntity("Parent");
    auto child = EntityManager::instance().addEntity("Child");

    auto parentTransform = parent->addComponent<CTransform>(Vec2(0, 0), Vec2(1, 1), 0.0f);
    auto childTransform = child->addComponent<CTransform>(Vec2(10, 10), Vec2(1, 1), 0.0f);

    auto physics = child->addComponent<CPhysicsBody2D>();
    physics->initialize(b2Vec2{10, 10}, BodyType::Dynamic);

    child->setParent(parent);

    // Physics root owner should be the child (highest in hierarchy with physics)
    Entity* physicsRoot = CPhysicsBody2D::getPhysicsRootOwner(child.get());
    EXPECT_EQ(physicsRoot, child.get());
}

// Test physics body on both parent and child - parent should be root
TEST_F(HierarchyTest, PhysicsBodyOnBothParentAndChild)
{
    auto parent = EntityManager::instance().addEntity("Parent");
    auto child = EntityManager::instance().addEntity("Child");

    auto parentTransform = parent->addComponent<CTransform>(Vec2(0, 0), Vec2(1, 1), 0.0f);
    auto childTransform = child->addComponent<CTransform>(Vec2(10, 10), Vec2(1, 1), 0.0f);

    auto parentPhysics = parent->addComponent<CPhysicsBody2D>();
    parentPhysics->initialize(b2Vec2{0, 0}, BodyType::Dynamic);

    auto childPhysics = child->addComponent<CPhysicsBody2D>();
    childPhysics->initialize(b2Vec2{10, 10}, BodyType::Dynamic);

    child->setParent(parent);

    // Physics root owner should be the parent (highest in hierarchy)
    Entity* physicsRoot = CPhysicsBody2D::getPhysicsRootOwner(child.get());
    EXPECT_EQ(physicsRoot, parent.get());
}

// Test serialization with physics bodies in hierarchy
TEST_F(HierarchyTest, SerializeDeserializeNestedEntitiesWithPhysics)
{
    // Create a hierarchy with physics
    auto parent = EntityManager::instance().addEntity("Parent");
    auto child = EntityManager::instance().addEntity("Child");

    auto parentTransform = parent->addComponent<CTransform>(Vec2(100, 100), Vec2(1, 1), 0.0f);
    auto childTransform = child->addComponent<CTransform>(Vec2(10, 10), Vec2(1, 1), 0.0f);

    auto parentPhysics = parent->addComponent<CPhysicsBody2D>();
    parentPhysics->initialize(b2Vec2{100, 100}, BodyType::Dynamic);

    child->setParent(parent);

    std::string parentGuid = parent->getGuid();
    std::string childGuid = child->getGuid();

    // Process pending entities before saving
    EntityManager::instance().update(0.0f);

    // Save to a temp file
    std::string filename = tempFileName("test_hierarchy_physics");
    EntityManager::instance().saveToFile(filename);

    // Clear and reload
    EntityManager::instance().clear();
    EntityManager::instance().loadFromFile(filename);
    EntityManager::instance().update(0.0f);  // Process loaded entities

    // Verify entities were loaded
    auto loadedParent = EntityManager::instance().getEntityByGuid(parentGuid);
    auto loadedChild = EntityManager::instance().getEntityByGuid(childGuid);

    ASSERT_NE(loadedParent, nullptr);
    ASSERT_NE(loadedChild, nullptr);

    // Verify parent-child relationship
    EXPECT_EQ(loadedChild->getParent(), loadedParent);

    // Verify physics body exists
    EXPECT_TRUE(loadedParent->hasComponent<CPhysicsBody2D>());

    // Verify physics root owner
    Entity* physicsRoot = CPhysicsBody2D::getPhysicsRootOwner(loadedChild.get());
    EXPECT_EQ(physicsRoot, loadedParent.get());
}

// Test deep hierarchy serialization
TEST_F(HierarchyTest, SerializeDeserializeDeepHierarchy)
{
    // Create a deep hierarchy
    auto root = EntityManager::instance().addEntity("Root");
    auto level1 = EntityManager::instance().addEntity("Level1");
    auto level2 = EntityManager::instance().addEntity("Level2");
    auto level3 = EntityManager::instance().addEntity("Level3");

    root->addComponent<CTransform>(Vec2(0, 0), Vec2(1, 1), 0.0f);
    level1->addComponent<CTransform>(Vec2(10, 0), Vec2(1, 1), 0.0f);
    level2->addComponent<CTransform>(Vec2(10, 0), Vec2(1, 1), 0.0f);
    level3->addComponent<CTransform>(Vec2(10, 0), Vec2(1, 1), 0.0f);

    level1->setParent(root);
    level2->setParent(level1);
    level3->setParent(level2);

    std::string rootGuid = root->getGuid();
    std::string level3Guid = level3->getGuid();

    // Process pending entities before saving
    EntityManager::instance().update(0.0f);

    // Save to a temp file
    std::string filename = tempFileName("test_deep_hierarchy");
    EntityManager::instance().saveToFile(filename);

    // Clear and reload
    EntityManager::instance().clear();
    EntityManager::instance().loadFromFile(filename);
    EntityManager::instance().update(0.0f);  // Process loaded entities

    // Verify entities were loaded
    auto loadedRoot = EntityManager::instance().getEntityByGuid(rootGuid);
    auto loadedLevel3 = EntityManager::instance().getEntityByGuid(level3Guid);

    ASSERT_NE(loadedRoot, nullptr);
    ASSERT_NE(loadedLevel3, nullptr);

    // Verify hierarchy is intact
    auto parent = loadedLevel3->getParent();
    ASSERT_NE(parent, nullptr);
    parent = parent->getParent();
    ASSERT_NE(parent, nullptr);
    parent = parent->getParent();
    EXPECT_EQ(parent, loadedRoot);
}

// Test removing child from parent
TEST_F(HierarchyTest, RemoveChildFromParent)
{
    auto parent = EntityManager::instance().addEntity("Parent");
    auto child = EntityManager::instance().addEntity("Child");

    child->setParent(parent);
    EXPECT_EQ(child->getParent(), parent);

    // Note: removeChild is a low-level method. After calling it, the child's parent
    // pointer is not automatically cleared because removeChild only updates the parent's
    // children list. This is by design - users should call child->setParent(nullptr) instead.
    parent->removeChild(child);
    
    // Child still thinks it has a parent (internal state not updated)
    EXPECT_EQ(child->getParent(), parent);
    // But parent no longer lists this as a child
    EXPECT_EQ(parent->getChildren().size(), 0);
    
    // To properly clear the relationship, use setParent(nullptr)
    child->setParent(nullptr);
    EXPECT_EQ(child->getParent(), nullptr);
}

// Test clearing parent
TEST_F(HierarchyTest, ClearParent)
{
    auto parent = EntityManager::instance().addEntity("Parent");
    auto child = EntityManager::instance().addEntity("Child");

    child->setParent(parent);
    EXPECT_EQ(child->getParent(), parent);

    child->setParent(nullptr);
    EXPECT_EQ(child->getParent(), nullptr);
    EXPECT_EQ(parent->getChildren().size(), 0);
}

// Test changing parent
TEST_F(HierarchyTest, ChangeParent)
{
    auto parent1 = EntityManager::instance().addEntity("Parent1");
    auto parent2 = EntityManager::instance().addEntity("Parent2");
    auto child = EntityManager::instance().addEntity("Child");

    child->setParent(parent1);
    EXPECT_EQ(child->getParent(), parent1);
    EXPECT_EQ(parent1->getChildren().size(), 1);

    child->setParent(parent2);
    EXPECT_EQ(child->getParent(), parent2);
    EXPECT_EQ(parent1->getChildren().size(), 0);
    EXPECT_EQ(parent2->getChildren().size(), 1);
}

// Test that when the parent owns the physics body, child's world transform
// matches parent's body world transform plus the child's local offset/rotation
TEST_F(HierarchyTest, PhysicsTransformInheritance_FromParentBody)
{
    auto parent = EntityManager::instance().addEntity("Parent");
    auto child = EntityManager::instance().addEntity("Child");

    // Parent transform and physics body
    auto parentTransform = parent->addComponent<CTransform>(Vec2(50, 50), Vec2(1,1), 0.5f);
    auto childTransform = child->addComponent<CTransform>(Vec2(10, 0), Vec2(1,1), 0.25f);

    auto parentPhysics = parent->addComponent<CPhysicsBody2D>();
    parentPhysics->initialize(b2Vec2{50.0f, 50.0f}, BodyType::Dynamic);

    child->setParent(parent);

    // Ensure entities/components are registered
    EntityManager::instance().update(0.0f);

    // Step physics once to sync body -> transform
    SBox2DPhysics::instance().update(1.0f/60.0f);

    // Parent world transform comes from physics body
    b2Vec2 pPos = parentPhysics->getPosition();
    float pRot = parentPhysics->getRotation();

    // Compute expected child world position: rotate local by parent rotation, then add
    float cosR = std::cos(pRot);
    float sinR = std::sin(pRot);
    Vec2 local = childTransform->getLocalPosition();
    Vec2 rotated = Vec2(local.x * cosR - local.y * sinR, local.x * sinR + local.y * cosR);
    Vec2 expectedWorld = Vec2(pPos.x + rotated.x, pPos.y + rotated.y);

    EXPECT_TRUE(vec2Equals(childTransform->getWorldPosition(), expectedWorld));

    // Rotation should add
    float expectedRot = pRot + childTransform->getLocalRotation();
    EXPECT_TRUE(floatEquals(childTransform->getWorldRotation(), expectedRot));
}

// Test that when the child owns the physics body, its body position matches
// the child's world transform (including parent's transform influence)
TEST_F(HierarchyTest, PhysicsTransformInheritance_FromChildBody)
{
    auto parent = EntityManager::instance().addEntity("Parent");
    auto child = EntityManager::instance().addEntity("Child");

    auto parentTransform = parent->addComponent<CTransform>(Vec2(100, 0), Vec2(1,1), 0.0f);
    auto childTransform = child->addComponent<CTransform>(Vec2(10, 0), Vec2(1,1), 0.0f);

    auto childPhysics = child->addComponent<CPhysicsBody2D>();
    childPhysics->initialize(b2Vec2{110.0f, 0.0f}, BodyType::Dynamic);

    child->setParent(parent);

    EntityManager::instance().update(0.0f);

    // Step physics to sync child body -> child's transform
    SBox2DPhysics::instance().update(1.0f/60.0f);

    // Child physics body world pos should equal child's world transform
    b2Vec2 cPos = childPhysics->getPosition();
    float cRot = childPhysics->getRotation();

    EXPECT_TRUE(vec2Equals(childTransform->getWorldPosition(), Vec2(cPos.x, cPos.y)));
    EXPECT_TRUE(floatEquals(childTransform->getWorldRotation(), cRot));
}
