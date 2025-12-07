#include <gtest/gtest.h>

#include "CCollider2D.h"
#include "CPhysicsBody2D.h"
#include "CTransform.h"
#include "Entity.h"
#include "EntityManager.h"
#include "SBox2DPhysics.h"
#include "Vec2.h"
#include "box2d/box2d.h"

class CCollider2DSegmentTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Ensure physics system is initialized
        SBox2DPhysics::instance();
    }

    void TearDown() override
    {
        // Entity cleanup happens automatically through EntityManager
    }

    // Helper to create entity with physics body
    std::shared_ptr<Entity> createPhysicsEntity(const Vec2& pos = Vec2(0.0f, 0.0f),
                                                BodyType type = BodyType::Dynamic)
    {
        auto& manager = EntityManager::instance();
        auto  entity  = manager.addEntity("test_entity");

        auto transform = entity->addComponent<CTransform>();
        transform->setPosition(pos);

        auto physicsBody = entity->addComponent<CPhysicsBody2D>();
        physicsBody->initialize({pos.x, pos.y}, type);

        return entity;
    }
};

// ============================================================================
// Basic Segment Creation Tests
// ============================================================================

TEST_F(CCollider2DSegmentTest, CreateSimpleSegment)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    b2Vec2 point1 = {-1.0f, 0.0f};
    b2Vec2 point2 = {1.0f, 0.0f};

    collider->createSegment(point1, point2);

    EXPECT_TRUE(collider->isInitialized());
    EXPECT_EQ(collider->getShapeType(), ColliderShape::Segment);
    EXPECT_TRUE(b2Shape_IsValid(collider->getShapeId()));
}

TEST_F(CCollider2DSegmentTest, CreateVerticalSegment)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    b2Vec2 point1 = {0.0f, -2.0f};
    b2Vec2 point2 = {0.0f, 2.0f};

    collider->createSegment(point1, point2);

    EXPECT_TRUE(collider->isInitialized());
    EXPECT_EQ(collider->getShapeType(), ColliderShape::Segment);
}

TEST_F(CCollider2DSegmentTest, CreateDiagonalSegment)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    b2Vec2 point1 = {-1.0f, -1.0f};
    b2Vec2 point2 = {1.0f, 1.0f};

    collider->createSegment(point1, point2);

    EXPECT_TRUE(collider->isInitialized());
    EXPECT_EQ(collider->getShapeType(), ColliderShape::Segment);
}

// ============================================================================
// Multi-Segment Tests
// ============================================================================

TEST_F(CCollider2DSegmentTest, AddMultipleSegments)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    // Create first segment
    collider->createSegment({-1.0f, 0.0f}, {0.0f, 0.0f});

    // Add additional segments
    collider->addSegment({0.0f, 0.0f}, {1.0f, 0.0f});
    collider->addSegment({1.0f, 0.0f}, {1.0f, 1.0f});

    EXPECT_TRUE(collider->isInitialized());
    
    const auto& fixtures = collider->getFixtures();
    EXPECT_EQ(fixtures.size(), 3);
    
    for (const auto& fixture : fixtures)
    {
        EXPECT_EQ(fixture.shapeType, ColliderShape::Segment);
        EXPECT_TRUE(b2Shape_IsValid(fixture.shapeId));
    }
}

TEST_F(CCollider2DSegmentTest, CreateBoxFromSegments)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    // Create a box outline from 4 segments
    collider->createSegment({-1.0f, -1.0f}, {1.0f, -1.0f});  // Bottom
    collider->addSegment({1.0f, -1.0f}, {1.0f, 1.0f});       // Right
    collider->addSegment({1.0f, 1.0f}, {-1.0f, 1.0f});       // Top
    collider->addSegment({-1.0f, 1.0f}, {-1.0f, -1.0f});     // Left

    EXPECT_TRUE(collider->isInitialized());
    
    const auto& fixtures = collider->getFixtures();
    EXPECT_EQ(fixtures.size(), 4);
}

TEST_F(CCollider2DSegmentTest, CreateTriangleFromSegments)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    // Create a triangle outline from 3 segments
    b2Vec2 p1 = {0.0f, 1.0f};
    b2Vec2 p2 = {-1.0f, -1.0f};
    b2Vec2 p3 = {1.0f, -1.0f};

    collider->createSegment(p1, p2);
    collider->addSegment(p2, p3);
    collider->addSegment(p3, p1);

    EXPECT_TRUE(collider->isInitialized());
    
    const auto& fixtures = collider->getFixtures();
    EXPECT_EQ(fixtures.size(), 3);
}

// ============================================================================
// Segment Properties Tests
// ============================================================================

TEST_F(CCollider2DSegmentTest, SegmentDensityFrictionRestitution)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    collider->setDensity(2.5f);
    collider->setFriction(0.8f);
    collider->setRestitution(0.6f);

    collider->createSegment({-1.0f, 0.0f}, {1.0f, 0.0f});

    EXPECT_TRUE(collider->isInitialized());
    EXPECT_FLOAT_EQ(collider->getDensity(), 2.5f);
    EXPECT_FLOAT_EQ(collider->getFriction(), 0.8f);
    EXPECT_FLOAT_EQ(collider->getRestitution(), 0.6f);
}

TEST_F(CCollider2DSegmentTest, SegmentAsSensor)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    collider->setIsSensor(true);
    collider->createSegment({-1.0f, 0.0f}, {1.0f, 0.0f});

    EXPECT_TRUE(collider->isInitialized());
    EXPECT_TRUE(collider->isSensor());
}

// ============================================================================
// Physics Simulation Tests
// ============================================================================

TEST_F(CCollider2DSegmentTest, BallCollidesWithSegmentWall)
{
    // Create static segment wall
    auto wall = createPhysicsEntity(Vec2(0.0f, 0.0f), BodyType::Static);
    auto wallCollider = wall->addComponent<CCollider2D>();
    wallCollider->createSegment({-5.0f, -5.0f}, {5.0f, -5.0f});

    // Create dynamic ball above wall
    auto ball = createPhysicsEntity(Vec2(0.0f, 5.0f));
    auto ballCollider = ball->addComponent<CCollider2D>();
    ballCollider->createCircle(0.5f);
    ballCollider->setDensity(1.0f);
    ballCollider->setRestitution(0.5f);

    auto ballPhysics = ball->getComponent<CPhysicsBody2D>();
    b2Vec2 initialPos = ballPhysics->getPosition();

    // Simulate
    for (int i = 0; i < 200; ++i)
    {
        SBox2DPhysics::instance().update(1.0f / 60.0f);
    }

    b2Vec2 finalPos = ballPhysics->getPosition();

    // Ball should have fallen and bounced/settled on segment
    EXPECT_LT(finalPos.y, initialPos.y);
    EXPECT_GT(finalPos.y, -5.0f);  // Should rest above the segment
}

TEST_F(CCollider2DSegmentTest, BallRollsAlongInclinedSegment)
{
    // Create inclined segment
    auto ramp = createPhysicsEntity(Vec2(0.0f, 0.0f), BodyType::Static);
    auto rampCollider = ramp->addComponent<CCollider2D>();
    rampCollider->createSegment({-5.0f, 2.0f}, {5.0f, -2.0f});  // Downward slope

    // Create ball at top of ramp
    auto ball = createPhysicsEntity(Vec2(-4.0f, 3.0f));
    auto ballCollider = ball->addComponent<CCollider2D>();
    ballCollider->createCircle(0.3f);
    ballCollider->setDensity(1.0f);
    ballCollider->setFriction(0.5f);

    auto ballPhysics = ball->getComponent<CPhysicsBody2D>();
    b2Vec2 initialPos = ballPhysics->getPosition();

    // Simulate
    for (int i = 0; i < 300; ++i)
    {
        SBox2DPhysics::instance().update(1.0f / 60.0f);
    }

    b2Vec2 finalPos = ballPhysics->getPosition();

    // Ball should roll down the ramp (move right and down)
    EXPECT_GT(finalPos.x, initialPos.x);
    EXPECT_LT(finalPos.y, initialPos.y);
}

TEST_F(CCollider2DSegmentTest, BoxCollidesWithSegmentContainer)
{
    // Create container from segments (3 walls)
    auto container = createPhysicsEntity(Vec2(0.0f, 0.0f), BodyType::Static);
    auto containerCollider = container->addComponent<CCollider2D>();
    
    containerCollider->createSegment({-3.0f, -3.0f}, {3.0f, -3.0f});   // Bottom
    containerCollider->addSegment({-3.0f, -3.0f}, {-3.0f, 3.0f});      // Left
    containerCollider->addSegment({3.0f, -3.0f}, {3.0f, 3.0f});        // Right

    // Create dynamic box
    auto box = createPhysicsEntity(Vec2(0.0f, 5.0f));
    auto boxCollider = box->addComponent<CCollider2D>();
    boxCollider->createBox(0.5f, 0.5f);
    boxCollider->setDensity(1.0f);

    auto boxPhysics = box->getComponent<CPhysicsBody2D>();

    // Simulate
    for (int i = 0; i < 300; ++i)
    {
        SBox2DPhysics::instance().update(1.0f / 60.0f);
    }

    b2Vec2 finalPos = boxPhysics->getPosition();

    // Box should be contained within segment walls
    EXPECT_GT(finalPos.x, -3.0f);
    EXPECT_LT(finalPos.x, 3.0f);
    EXPECT_GT(finalPos.y, -3.0f);
}

// ============================================================================
// Serialization Tests
// ============================================================================

TEST_F(CCollider2DSegmentTest, SerializeSegment)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    collider->createSegment({-1.5f, -0.5f}, {1.5f, 0.5f});
    collider->setDensity(1.5f);
    collider->setFriction(0.7f);
    collider->setRestitution(0.3f);

    Serialization::JsonBuilder builder;
    collider->serialize(builder);

    std::string json = builder.toString();

    // Verify JSON contains expected keys
    EXPECT_NE(json.find("Segment"), std::string::npos);
    EXPECT_NE(json.find("point1"), std::string::npos);
    EXPECT_NE(json.find("point2"), std::string::npos);
}

TEST_F(CCollider2DSegmentTest, DeserializeSegment)
{
    // Create JSON manually
    std::string json = R"({
        "cCollider2D": {
            "fixtures": [{
                "shapeType": "Segment",
                "point1": {"x": -2.0, "y": -1.0},
                "point2": {"x": 2.0, "y": 1.0}
            }],
            "isSensor": false,
            "density": 2.0,
            "friction": 0.5,
            "restitution": 0.2
        }
    })";    Serialization::SSerialization::JsonValue value(json);

    auto entity = createPhysicsEntity();
    auto collider = entity->addComponent<CCollider2D>();

    collider->deserialize(value);
    collider->init();  // Attach to body

    EXPECT_TRUE(collider->isInitialized());
    
    const auto& fixtures = collider->getFixtures();
    ASSERT_EQ(fixtures.size(), 1);
    EXPECT_EQ(fixtures[0].shapeType, ColliderShape::Segment);
    
    EXPECT_FLOAT_EQ(collider->getDensity(), 2.0f);
    EXPECT_FLOAT_EQ(collider->getFriction(), 0.5f);
    EXPECT_FLOAT_EQ(collider->getRestitution(), 0.2f);
}

TEST_F(CCollider2DSegmentTest, SerializeMultipleSegments)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    // Create triangle from segments
    collider->createSegment({0.0f, 1.0f}, {-1.0f, -1.0f});
    collider->addSegment({-1.0f, -1.0f}, {1.0f, -1.0f});
    collider->addSegment({1.0f, -1.0f}, {0.0f, 1.0f});

    Serialization::JsonBuilder builder;
    collider->serialize(builder);

    std::string json = builder.toString();

    // Verify JSON contains fixtures array
    EXPECT_NE(json.find("fixtures"), std::string::npos);
    EXPECT_NE(json.find("Segment"), std::string::npos);
}

TEST_F(CCollider2DSegmentTest, DeserializeMultipleSegments)
{
    // Create JSON with multiple segments
    std::string json = R"({
        "cCollider2D": {
            "fixtures": [
                {
                    "shapeType": "Segment",
                    "point1": {"x": -1.0, "y": 0.0},
                    "point2": {"x": 0.0, "y": 0.0}
                },
                {
                    "shapeType": "Segment",
                    "point1": {"x": 0.0, "y": 0.0},
                    "point2": {"x": 1.0, "y": 0.0}
                },
                {
                    "shapeType": "Segment",
                    "point1": {"x": 1.0, "y": 0.0},
                    "point2": {"x": 1.0, "y": 1.0}
                }
            ],
            "isSensor": false,
            "density": 1.0,
            "friction": 0.3,
            "restitution": 0.0
        }
    })";    Serialization::SSerialization::JsonValue value(json);

    auto entity = createPhysicsEntity();
    auto collider = entity->addComponent<CCollider2D>();

    collider->deserialize(value);
    collider->init();

    EXPECT_TRUE(collider->isInitialized());
    
    const auto& fixtures = collider->getFixtures();
    EXPECT_EQ(fixtures.size(), 3);
    
    for (const auto& fixture : fixtures)
    {
        EXPECT_EQ(fixture.shapeType, ColliderShape::Segment);
        EXPECT_TRUE(b2Shape_IsValid(fixture.shapeId));
    }
}

TEST_F(CCollider2DSegmentTest, SerializeDeserializeRoundTrip)
{
    auto entity1 = createPhysicsEntity();
    auto collider1 = entity1->addComponent<CCollider2D>();

    // Create segments
    collider1->createSegment({-2.0f, -1.0f}, {2.0f, -1.0f});
    collider1->addSegment({2.0f, -1.0f}, {2.0f, 1.0f});
    collider1->setDensity(1.8f);

    // Serialize
    Serialization::JsonBuilder builder;
    collider1->serialize(builder);
    std::string json = builder.toString();

    // Deserialize into new collider
    Serialization::SSerialization::JsonValue value(json);

    auto entity2 = createPhysicsEntity();
    auto collider2 = entity2->addComponent<CCollider2D>();
    collider2->deserialize(value);
    collider2->init();

    // Verify properties match
    const auto& fixtures1 = collider1->getFixtures();
    const auto& fixtures2 = collider2->getFixtures();
    
    EXPECT_EQ(fixtures2.size(), fixtures1.size());
    EXPECT_FLOAT_EQ(collider2->getDensity(), collider1->getDensity());
    
    for (size_t i = 0; i < fixtures2.size(); ++i)
    {
        EXPECT_EQ(fixtures2[i].shapeType, fixtures1[i].shapeType);
    }
}

// ============================================================================
// Mixed Fixtures Tests
// ============================================================================

TEST_F(CCollider2DSegmentTest, MixedPolygonAndSegments)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    // Create a polygon hull
    b2Vec2 hullVerts[4] = {{-0.5f, -0.5f}, {0.5f, -0.5f}, {0.5f, 0.5f}, {-0.5f, 0.5f}};
    collider->createPolygon(hullVerts, 4);

    // Add segment edges extending from the hull
    collider->addSegment({0.5f, 0.0f}, {1.5f, 0.0f});
    collider->addSegment({-0.5f, 0.0f}, {-1.5f, 0.0f});

    EXPECT_TRUE(collider->isInitialized());
    
    const auto& fixtures = collider->getFixtures();
    EXPECT_EQ(fixtures.size(), 3);
    
    EXPECT_EQ(fixtures[0].shapeType, ColliderShape::Polygon);
    EXPECT_EQ(fixtures[1].shapeType, ColliderShape::Segment);
    EXPECT_EQ(fixtures[2].shapeType, ColliderShape::Segment);
}

// ============================================================================
// Edge Cases and Error Handling
// ============================================================================

TEST_F(CCollider2DSegmentTest, CreateSegmentWithoutPhysicsBody)
{
    auto& manager = EntityManager::instance();
    auto entity = manager.addEntity("no_physics");

    auto transform = entity->addComponent<CTransform>();
    transform->setPosition(Vec2(0.0f, 0.0f));

    auto collider = entity->addComponent<CCollider2D>();
    collider->createSegment({-1.0f, 0.0f}, {1.0f, 0.0f});

    // Should not be initialized without physics body
    EXPECT_FALSE(collider->isInitialized());
}

TEST_F(CCollider2DSegmentTest, RecreateSegmentShape)
{
    auto entity = createPhysicsEntity();
    auto collider = entity->addComponent<CCollider2D>();

    // Create initial segment
    collider->createSegment({-1.0f, 0.0f}, {1.0f, 0.0f});
    EXPECT_TRUE(collider->isInitialized());

    // Recreate with different segment
    collider->createSegment({0.0f, -2.0f}, {0.0f, 2.0f});

    EXPECT_TRUE(collider->isInitialized());
    
    const auto& fixtures = collider->getFixtures();
    EXPECT_EQ(fixtures.size(), 1);
    EXPECT_EQ(fixtures[0].shapeType, ColliderShape::Segment);
}

TEST_F(CCollider2DSegmentTest, VeryShortSegment)
{
    auto entity = createPhysicsEntity();
    auto collider = entity->addComponent<CCollider2D>();

    // Create a very short segment (should still work if above Box2D threshold)
    collider->createSegment({0.0f, 0.0f}, {0.01f, 0.01f});

    // May or may not initialize depending on Box2D's minimum segment length
    // This test documents the behavior
    if (collider->isInitialized())
    {
        EXPECT_EQ(collider->getShapeType(), ColliderShape::Segment);
    }
}

TEST_F(CCollider2DSegmentTest, LongSegment)
{
    auto entity = createPhysicsEntity();
    auto collider = entity->addComponent<CCollider2D>();

    // Create a very long segment
    collider->createSegment({-100.0f, 0.0f}, {100.0f, 0.0f});

    EXPECT_TRUE(collider->isInitialized());
    EXPECT_EQ(collider->getShapeType(), ColliderShape::Segment);
}
