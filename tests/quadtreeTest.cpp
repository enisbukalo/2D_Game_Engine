#include <gtest/gtest.h>
#include "physics/Quadtree.h"
#include "Entity.h"
#include "components/CTransform.h"
#include "EntityManager.h"
#include "algorithm"

class QuadtreeTest : public ::testing::Test {
protected:
    QuadtreeTest()
        : bounds(Vec2(0, 0), Vec2(50, 50))  // Initialize bounds in constructor
    {
    }

    void SetUp() override {
        // Create a quadtree with bounds initialized in constructor
        tree = std::make_unique<Quadtree>(0, bounds);
    }

    // Helper function to create an entity at a specific position
    Entity* createEntityAtPosition(const Vec2& pos) {
        auto& entityManager = EntityManager::instance();
        auto entity = entityManager.addEntity("test");  // Use EntityManager to create entity
        auto transform = entity->addComponent<CTransform>();
        transform->setPosition(pos);
        return entity.get();  // Return raw pointer from shared_ptr
    }

    void TearDown() override {
        // No need to manually delete entities - EntityManager will handle cleanup
        EntityManager::instance().clear();
        createdEntities.clear();
    }

    AABB bounds;
    std::unique_ptr<Quadtree> tree;
    std::vector<Entity*> createdEntities;
};

TEST_F(QuadtreeTest, InsertSingleEntity) {
    auto* entity = createEntityAtPosition(Vec2(0, 0));
    createdEntities.push_back(entity);

    tree->insert(entity);

    // Query the entire area
    auto results = tree->query(bounds);
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], entity);
    EXPECT_EQ(results[0]->getComponent<CTransform>()->getPosition(), Vec2(0, 0));
}

TEST_F(QuadtreeTest, QueryEmptyArea) {
    // Query an area where no entities exist
    AABB emptyArea(Vec2(100, 100), Vec2(10, 10));
    auto results = tree->query(emptyArea);
    EXPECT_TRUE(results.empty());
}

TEST_F(QuadtreeTest, InsertMultipleEntities) {
    // Create entities in different quadrants
    std::vector<Vec2> positions = {
        Vec2(-25, 25),  // Top-left
        Vec2(25, 25),   // Top-right
        Vec2(-25, -25), // Bottom-left
        Vec2(25, -25)   // Bottom-right
    };

    for (const auto& pos : positions) {
        auto* entity = createEntityAtPosition(pos);
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

TEST_F(QuadtreeTest, QuerySpecificQuadrant) {
    // Insert entities in all quadrants
    std::vector<Vec2> positions = {
        Vec2(-25, 25),  // Top-left
        Vec2(25, 25),   // Top-right
        Vec2(-25, -25), // Bottom-left
        Vec2(25, -25)   // Bottom-right
    };

    for (const auto& pos : positions) {
        auto* entity = createEntityAtPosition(pos);
        createdEntities.push_back(entity);
        tree->insert(entity);
    }

    // Query only the top-left quadrant
    AABB topLeft(Vec2(-25, 25), Vec2(10, 10));
    auto results = tree->query(topLeft);
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]->getComponent<CTransform>()->getPosition(), Vec2(-25, 25));
}

TEST_F(QuadtreeTest, SubdivisionTest) {
    // Insert more entities than MAX_OBJECTS in the same area to force subdivision
    Vec2 basePos(-5, -5);
    for (int i = 0; i < Quadtree::MAX_OBJECTS + 2; ++i) {
        Vec2 pos = basePos + Vec2(i * 1.0f, i * 1.0f);
        auto* entity = createEntityAtPosition(pos);
        createdEntities.push_back(entity);
        tree->insert(entity);
    }

    // Query a small area that should contain only a few entities
    AABB smallArea(basePos, Vec2(2, 2));
    auto results = tree->query(smallArea);
    EXPECT_LT(results.size(), Quadtree::MAX_OBJECTS);
}

TEST_F(QuadtreeTest, ClearTest) {
    // Insert some entities
    for (int i = 0; i < 5; ++i) {
        auto* entity = createEntityAtPosition(Vec2(i * 10.0f, 0));
        createdEntities.push_back(entity);
        tree->insert(entity);
    }

    // Clear the tree
    tree->clear();

    // Query the entire area - should be empty
    auto results = tree->query(bounds);
    EXPECT_TRUE(results.empty());
}

TEST_F(QuadtreeTest, BoundaryConditions) {
    // Test entities exactly on boundaries
    // Expectation: All entities should be in the tree
    std::vector<Vec2> positions = {
        Vec2(-50, 0),   // Left edge
        Vec2(50, 0),    // Right edge
        Vec2(0, 50),    // Top edge
        Vec2(0, -50),   // Bottom edge
        Vec2(0, 0)      // Center
    };

    for (const auto& pos : positions) {
        auto* entity = createEntityAtPosition(pos);
        createdEntities.push_back(entity);
        tree->insert(entity);
    }

    auto results = tree->query(bounds);
    EXPECT_EQ(results.size(), 5);
    EXPECT_EQ(results[0]->getComponent<CTransform>()->getPosition(), Vec2(-50, 0));
    EXPECT_EQ(results[1]->getComponent<CTransform>()->getPosition(), Vec2(50, 0));
    EXPECT_EQ(results[2]->getComponent<CTransform>()->getPosition(), Vec2(0, 50));
    EXPECT_EQ(results[3]->getComponent<CTransform>()->getPosition(), Vec2(0, -50));
    EXPECT_EQ(results[4]->getComponent<CTransform>()->getPosition(), Vec2(0, 0));
}

TEST_F(QuadtreeTest, QueryPartialOverlap) {
    // Insert entities in a grid pattern
    for (float x = -40; x <= 40; x += 20) {
        for (float y = -40; y <= 40; y += 20) {
            auto* entity = createEntityAtPosition(Vec2(x, y));
            createdEntities.push_back(entity);
            tree->insert(entity);
        }
    }

    // Query an area that partially overlaps multiple quadrants
    AABB queryArea(Vec2(10, 10), Vec2(20, 20));
    auto results = tree->query(queryArea);

    // Count how many entities should be in this area
    // Expectation: All entities should be in the tree
    int expectedCount = 0;
    for (auto* entity : createdEntities) {
        auto transform = entity->getComponent<CTransform>();
        if (queryArea.contains(transform->getPosition())) {
            expectedCount++;
        }
    }

    EXPECT_EQ(results.size(), expectedCount);

}

TEST_F(QuadtreeTest, SmoothMovement) {
    // Create an entity
    auto* entity = createEntityAtPosition(Vec2(-1.23f, 4.56f));
    createdEntities.push_back(entity);
    tree->insert(entity);

    // Move it by a small amount
    auto transform = entity->getComponent<CTransform>();
    Vec2 newPos(-1.24f, 4.57f);  // Tiny movement
    transform->setPosition(newPos);

    // Clear and reinsert
    tree->clear();
    tree->insert(entity);

    // Query to find it
    AABB queryArea(newPos, Vec2(0.1f, 0.1f));
    auto results = tree->query(queryArea);

    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], entity);

    auto finalPos = results[0]->getComponent<CTransform>()->getPosition();
    EXPECT_FLOAT_EQ(finalPos.x, newPos.x);
    EXPECT_FLOAT_EQ(finalPos.y, newPos.y);
}