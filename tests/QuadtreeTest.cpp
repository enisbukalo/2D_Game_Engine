#include <gtest/gtest.h>
#include "CCircleCollider.h"
#include "CTransform.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Quadtree.h"
#include "algorithm"

class QuadtreeTest : public ::testing::Test
{
protected:
    QuadtreeTest() : bounds(Vec2(0, 0), Vec2(100, 100))  // Full size 100x100, halfSize will be 50x50
    {
    }

    void SetUp() override
    {
        // Create a quadtree with bounds initialized in constructor
        tree = std::make_unique<Quadtree>(0, bounds);
    }

    // Helper function to create an entity at a specific position with a collider
    Entity* createEntityWithCollider(const Vec2& pos, float radius)
    {
        auto& entityManager = EntityManager::instance();
        auto  entity        = entityManager.addEntity("test");
        auto  transform     = entity->addComponent<CTransform>();
        transform->setPosition(pos);
        auto collider = entity->addComponent<CCircleCollider>(radius);
        return entity.get();
    }

    void TearDown() override
    {
        EntityManager::instance().clear();
        createdEntities.clear();
    }

    AABB                      bounds;
    std::unique_ptr<Quadtree> tree;
    std::vector<Entity*>      createdEntities;
};

TEST_F(QuadtreeTest, InsertSingleEntity)
{
    auto* entity = createEntityWithCollider(Vec2(0, 0), 1.0f);
    createdEntities.push_back(entity);

    // Debug checks
    auto transform = entity->getComponent<CTransform>();
    auto collider  = entity->getComponent<CCircleCollider>();
    ASSERT_TRUE(transform != nullptr) << "Transform component is null";
    ASSERT_TRUE(collider != nullptr) << "Collider component is null";
    ASSERT_EQ(transform->getPosition(), Vec2(0, 0)) << "Transform position is incorrect";
    ASSERT_EQ(collider->getRadius(), 1.0f) << "Collider radius is incorrect";

    tree->insert(entity);

    // Query the entire area using the same bounds as the tree
    AABB queryArea(Vec2(0, 0), Vec2(100, 100));  // Full size 100x100
    auto results = tree->query(queryArea);
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], entity);
    EXPECT_EQ(results[0]->getComponent<CTransform>()->getPosition(), Vec2(0, 0));
}

TEST_F(QuadtreeTest, QueryEmptyArea)
{
    // Query an area where no entities exist
    AABB emptyArea(Vec2(100, 100), Vec2(20, 20));  // Full size 20x20
    auto results = tree->query(emptyArea);
    EXPECT_TRUE(results.empty());
}

TEST_F(QuadtreeTest, InsertMultipleEntities)
{
    // Create entities in different quadrants
    std::vector<Vec2> positions = {
        Vec2(-25, 25),   // Top-left
        Vec2(25, 25),    // Top-right
        Vec2(-25, -25),  // Bottom-left
        Vec2(25, -25)    // Bottom-right
    };

    for (const auto& pos : positions)
    {
        auto* entity = createEntityWithCollider(pos, 1.0f);
        createdEntities.push_back(entity);
        tree->insert(entity);
    }

    // Query the entire area
    auto results = tree->query(bounds);
    EXPECT_EQ(results.size(), 4);
    EXPECT_EQ(results[0]->getComponent<CTransform>()->getPosition(), Vec2(-25, 25));
    EXPECT_EQ(results[1]->getComponent<CTransform>()->getPosition(), Vec2(25, 25));
    EXPECT_EQ(results[2]->getComponent<CTransform>()->getPosition(), Vec2(-25, -25));
    EXPECT_EQ(results[3]->getComponent<CTransform>()->getPosition(), Vec2(25, -25));
}

TEST_F(QuadtreeTest, QuerySpecificQuadrant)
{
    // Insert entities in all quadrants
    std::vector<Vec2> positions = {
        Vec2(-25, 25),   // Top-left
        Vec2(25, 25),    // Top-right
        Vec2(-25, -25),  // Bottom-left
        Vec2(25, -25)    // Bottom-right
    };

    for (const auto& pos : positions)
    {
        auto* entity = createEntityWithCollider(pos, 1.0f);
        createdEntities.push_back(entity);
        tree->insert(entity);
    }

    // Query only the top-left quadrant
    AABB topLeft(Vec2(-25, 25), Vec2(20, 20));  // Full size 20x20
    auto results = tree->query(topLeft);
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]->getComponent<CTransform>()->getPosition(), Vec2(-25, 25));
}

TEST_F(QuadtreeTest, LargeColliderMultiQuadrant)
{
    // Create an entity with a large collider that spans multiple quadrants
    auto* entity = createEntityWithCollider(Vec2(0, 0), 20.0f);  // Radius of 20 will overlap multiple quadrants
    createdEntities.push_back(entity);
    tree->insert(entity);

    // Query each quadrant - the entity should be found in all of them
    std::vector<AABB> quadrantQueries = {
        AABB(Vec2(-20, 20), Vec2(10, 10)),   // Top-left, full size 10x10
        AABB(Vec2(20, 20), Vec2(10, 10)),    // Top-right, full size 10x10
        AABB(Vec2(-20, -20), Vec2(10, 10)),  // Bottom-left, full size 10x10
        AABB(Vec2(20, -20), Vec2(10, 10))    // Bottom-right, full size 10x10
    };

    for (const auto& queryArea : quadrantQueries)
    {
        auto results = tree->query(queryArea);
        EXPECT_EQ(results.size(), 1) << "Large entity should be found in all quadrants it overlaps";
        if (!results.empty())
        {
            EXPECT_EQ(results[0], entity);
        }
    }
}

TEST_F(QuadtreeTest, ColliderOnQuadrantBoundary)
{
    // Create an entity with a collider that sits exactly on the boundary between quadrants
    auto* entity = createEntityWithCollider(Vec2(0, 0), 5.0f);  // Place at center with radius that reaches into multiple quadrants
    createdEntities.push_back(entity);
    tree->insert(entity);

    // Query areas on both sides of the boundary
    AABB leftQuery(Vec2(-5, 0), Vec2(10, 10));   // Full size 10x10
    AABB rightQuery(Vec2(5, 0), Vec2(10, 10));   // Full size 10x10

    auto leftResults  = tree->query(leftQuery);
    auto rightResults = tree->query(rightQuery);

    EXPECT_FALSE(leftResults.empty()) << "Entity should be found in left quadrant";
    EXPECT_FALSE(rightResults.empty()) << "Entity should be found in right quadrant";
}

TEST_F(QuadtreeTest, MultipleOverlappingColliders)
{
    // Create several entities with overlapping colliders
    auto* entity1 = createEntityWithCollider(Vec2(0, 0), 10.0f);
    auto* entity2 = createEntityWithCollider(Vec2(5, 5), 10.0f);
    auto* entity3 = createEntityWithCollider(Vec2(-5, -5), 10.0f);

    createdEntities.push_back(entity1);
    createdEntities.push_back(entity2);
    createdEntities.push_back(entity3);

    tree->insert(entity1);
    tree->insert(entity2);
    tree->insert(entity3);

    // Query an area where all colliders overlap
    AABB queryArea(Vec2(0, 0), Vec2(10, 10));  // Full size 10x10
    auto results = tree->query(queryArea);

    EXPECT_EQ(results.size(), 3) << "Should find all entities with overlapping colliders";
}

TEST_F(QuadtreeTest, SubdivisionWithColliders)
{
    // Insert more entities than MAX_OBJECTS in the same area to force subdivision
    Vec2 basePos(-5, -5);
    for (int i = 0; i < Quadtree::MAX_OBJECTS + 2; ++i)
    {
        Vec2  pos    = basePos + Vec2(i * 2.0f, i * 2.0f);   // Spread them out more to avoid overlap
        auto* entity = createEntityWithCollider(pos, 1.0f);  // Small colliders
        createdEntities.push_back(entity);
        tree->insert(entity);
    }

    // Query a small area that should contain only a few entities
    AABB smallArea(basePos, Vec2(4, 4));  // Full size 4x4
    auto results = tree->query(smallArea);
    EXPECT_LT(results.size(), Quadtree::MAX_OBJECTS);
}

TEST_F(QuadtreeTest, ClearTest)
{
    // Insert some entities with colliders
    for (int i = 0; i < 5; ++i)
    {
        auto* entity = createEntityWithCollider(Vec2(i * 10.0f, 0), 2.0f);
        createdEntities.push_back(entity);
        tree->insert(entity);
    }

    // Clear the tree
    tree->clear();

    // Query the entire area - should be empty
    auto results = tree->query(bounds);
    EXPECT_TRUE(results.empty());
}

TEST_F(QuadtreeTest, QueryPartialOverlap)
{
    // Insert entities with colliders in a grid pattern
    for (float x = -40; x <= 40; x += 20)
    {
        for (float y = -40; y <= 40; y += 20)
        {
            auto* entity = createEntityWithCollider(Vec2(x, y), 5.0f);  // Collider radius of 5
            createdEntities.push_back(entity);
            tree->insert(entity);
        }
    }

    // Query an area that partially overlaps multiple quadrants
    AABB queryArea(Vec2(10, 10), Vec2(40, 40));  // Full size 40x40
    auto results = tree->query(queryArea);

    // Verify that we find all entities whose colliders overlap the query area
    int expectedCount = 0;
    for (auto* entity : createdEntities)
    {
        auto collider = entity->getComponent<CCircleCollider>();
        if (collider)
        {
            AABB entityBounds = collider->getBounds();
            if (queryArea.intersects(entityBounds))
            {
                expectedCount++;
            }
        }
    }

    EXPECT_EQ(results.size(), expectedCount);
}