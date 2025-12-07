#include <gtest/gtest.h>

#include "CCollider2D.h"
#include "CPhysicsBody2D.h"
#include "CTransform.h"
#include "Entity.h"
#include "SEntity.h"
#include "S2DPhysics.h"
#include "Vec2.h"
#include "box2d/box2d.h"

class CCollider2DPolygonTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Ensure physics system is initialized
        S2DPhysics::instance();
    }

    void TearDown() override
    {
        // Entity cleanup happens automatically through EntityManager
    }

    // Helper to create entity with physics body
    std::shared_ptr<Entity> createPhysicsEntity(const Vec2& pos = Vec2(0.0f, 0.0f),
                                                BodyType type = BodyType::Dynamic)
    {
        auto& manager = SEntity::instance();
        auto  entity  = manager.addEntity("test_entity");

        auto transform = entity->addComponent<CTransform>();
        transform->setPosition(pos);

        auto physicsBody = entity->addComponent<CPhysicsBody2D>();
        physicsBody->initialize({pos.x, pos.y}, type);

        return entity;
    }
};

// ============================================================================
// Basic Polygon Creation Tests
// ============================================================================

TEST_F(CCollider2DPolygonTest, CreateTrianglePolygon)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    // Create a triangle
    b2Vec2 vertices[3] = {{0.0f, 1.0f}, {-0.5f, -0.5f}, {0.5f, -0.5f}};

    collider->createPolygon(vertices, 3);

    EXPECT_TRUE(collider->isInitialized());
    EXPECT_EQ(collider->getShapeType(), ColliderShape::Polygon);
    EXPECT_EQ(collider->getPolygonVertexCount(), 3);
    EXPECT_TRUE(b2Shape_IsValid(collider->getShapeId()));
}

TEST_F(CCollider2DPolygonTest, CreateRectanglePolygon)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    // Create a rectangle (4 vertices)
    b2Vec2 vertices[4] = {{-1.0f, -0.5f}, {1.0f, -0.5f}, {1.0f, 0.5f}, {-1.0f, 0.5f}};

    collider->createPolygon(vertices, 4);

    EXPECT_TRUE(collider->isInitialized());
    EXPECT_EQ(collider->getShapeType(), ColliderShape::Polygon);
    EXPECT_EQ(collider->getPolygonVertexCount(), 4);
}

TEST_F(CCollider2DPolygonTest, CreateComplexPolygon)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    // Create a hexagon (6 vertices)
    b2Vec2 vertices[6];
    for (int i = 0; i < 6; ++i)
    {
        float angle = (i / 6.0f) * 2.0f * B2_PI;
        vertices[i] = {cosf(angle), sinf(angle)};
    }

    collider->createPolygon(vertices, 6);

    EXPECT_TRUE(collider->isInitialized());
    EXPECT_EQ(collider->getShapeType(), ColliderShape::Polygon);
    EXPECT_EQ(collider->getPolygonVertexCount(), 6);
}

TEST_F(CCollider2DPolygonTest, CreatePolygonWithRadius)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    b2Vec2 vertices[4] = {{-0.5f, -0.5f}, {0.5f, -0.5f}, {0.5f, 0.5f}, {-0.5f, 0.5f}};

    float radius = 0.1f;
    collider->createPolygon(vertices, 4, radius);

    EXPECT_TRUE(collider->isInitialized());
    EXPECT_FLOAT_EQ(collider->getPolygonRadius(), radius);
}

// ============================================================================
// Hull Computation Tests
// ============================================================================

TEST_F(CCollider2DPolygonTest, HullComputationUnsortedVertices)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    // Unsorted vertices - hull should sort them
    b2Vec2 vertices[4] = {{0.5f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f}, {-0.5f, 0.5f}};

    collider->createPolygon(vertices, 4);

    EXPECT_TRUE(collider->isInitialized());
    EXPECT_EQ(collider->getPolygonVertexCount(), 4);
}

TEST_F(CCollider2DPolygonTest, HullComputationTooFewPoints)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    // Only 2 points - should fail
    b2Vec2 vertices[2] = {{0.0f, 0.0f}, {1.0f, 0.0f}};

    collider->createPolygon(vertices, 2);

    // Should not create a polygon (collider remains uninitialized or with previous state)
    // The shape type should not be Polygon if it failed
    if (collider->isInitialized())
    {
        EXPECT_NE(collider->getShapeType(), ColliderShape::Polygon);
    }
}

TEST_F(CCollider2DPolygonTest, HullComputationNullVertices)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    // Null vertices - should fail gracefully
    collider->createPolygon(nullptr, 3);

    // Should not create a polygon (collider remains uninitialized or with previous state)
    // The shape type should not be Polygon if it failed
    if (collider->isInitialized())
    {
        EXPECT_NE(collider->getShapeType(), ColliderShape::Polygon);
    }
}

// ============================================================================
// Polygon from Hull Tests
// ============================================================================

TEST_F(CCollider2DPolygonTest, CreatePolygonFromPrecomputedHull)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    // Pre-compute hull
    b2Vec2 vertices[5] = {{0.0f, 1.0f}, {0.95f, 0.31f}, {0.59f, -0.81f}, {-0.59f, -0.81f},
                          {-0.95f, 0.31f}};
    b2Hull hull        = b2ComputeHull(vertices, 5);

    ASSERT_GE(hull.count, 3);

    collider->createPolygonFromHull(hull);

    EXPECT_TRUE(collider->isInitialized());
    EXPECT_EQ(collider->getPolygonVertexCount(), hull.count);
}

// ============================================================================
// Offset Polygon Tests
// ============================================================================

TEST_F(CCollider2DPolygonTest, CreateOffsetPolygon)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    // Create hull
    b2Vec2 vertices[3] = {{0.0f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f}};
    b2Hull hull        = b2ComputeHull(vertices, 3);

    b2Vec2 offset   = {1.0f, 2.0f};
    float  rotation = B2_PI / 4.0f;  // 45 degrees

    collider->createOffsetPolygon(hull, offset, rotation);

    EXPECT_TRUE(collider->isInitialized());
    EXPECT_EQ(collider->getShapeType(), ColliderShape::Polygon);
}

TEST_F(CCollider2DPolygonTest, CreateOffsetPolygonWithRadius)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    b2Vec2 vertices[4] = {{-0.5f, -0.5f}, {0.5f, -0.5f}, {0.5f, 0.5f}, {-0.5f, 0.5f}};
    b2Hull hull        = b2ComputeHull(vertices, 4);

    b2Vec2 offset   = {0.0f, 0.0f};
    float  rotation = 0.0f;
    float  radius   = 0.15f;

    collider->createOffsetPolygon(hull, offset, rotation, radius);

    EXPECT_TRUE(collider->isInitialized());
    EXPECT_FLOAT_EQ(collider->getPolygonRadius(), radius);
}

// ============================================================================
// Polygon Properties Tests
// ============================================================================

TEST_F(CCollider2DPolygonTest, PolygonDensityFrictionRestitution)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    collider->setDensity(2.5f);
    collider->setFriction(0.8f);
    collider->setRestitution(0.6f);

    b2Vec2 vertices[3] = {{0.0f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f}};
    collider->createPolygon(vertices, 3);

    EXPECT_TRUE(collider->isInitialized());
    EXPECT_FLOAT_EQ(collider->getDensity(), 2.5f);
    EXPECT_FLOAT_EQ(collider->getFriction(), 0.8f);
    EXPECT_FLOAT_EQ(collider->getRestitution(), 0.6f);
}

TEST_F(CCollider2DPolygonTest, PolygonAsSensor)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    collider->setIsSensor(true);

    b2Vec2 vertices[4] = {{-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f}};
    collider->createPolygon(vertices, 4);

    EXPECT_TRUE(collider->isInitialized());
    EXPECT_TRUE(collider->isSensor());
}

TEST_F(CCollider2DPolygonTest, GetPolygonVertices)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    b2Vec2 vertices[3] = {{0.0f, 1.0f}, {-0.5f, -0.5f}, {0.5f, -0.5f}};
    collider->createPolygon(vertices, 3);

    const b2Vec2* retrievedVertices = collider->getPolygonVertices();
    ASSERT_NE(retrievedVertices, nullptr);

    int count = collider->getPolygonVertexCount();
    EXPECT_EQ(count, 3);

    // Vertices should be stored (though may be reordered by hull computation)
    for (int i = 0; i < count; ++i)
    {
        EXPECT_TRUE(retrievedVertices[i].x != 0.0f || retrievedVertices[i].y != 0.0f);
    }
}

// ============================================================================
// Physics Simulation Tests
// ============================================================================

TEST_F(CCollider2DPolygonTest, PolygonFallsUnderGravity)
{
    auto entity = createPhysicsEntity(Vec2(0.0f, 10.0f));

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    b2Vec2 vertices[3] = {{0.0f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f}};
    collider->createPolygon(vertices, 3);

    auto physicsBody = entity->getComponent<CPhysicsBody2D>();
    b2Vec2 initialPos = physicsBody->getPosition();

    // Simulate for 1 second
    for (int i = 0; i < 60; ++i)
    {
        S2DPhysics::instance().update(1.0f / 60.0f);
    }

    b2Vec2 finalPos = physicsBody->getPosition();

    // Should fall due to gravity
    EXPECT_LT(finalPos.y, initialPos.y);
}

TEST_F(CCollider2DPolygonTest, PolygonCollidesWithBox)
{
    // Create static ground (box)
    auto ground        = createPhysicsEntity(Vec2(0.0f, -5.0f), BodyType::Static);
    auto groundCollider = ground->addComponent<CCollider2D>();
    groundCollider->createBox(10.0f, 0.5f);

    // Create dynamic polygon above ground
    auto entity = createPhysicsEntity(Vec2(0.0f, 5.0f));
    auto collider = entity->addComponent<CCollider2D>();

    b2Vec2 vertices[4] = {{-0.5f, -0.5f}, {0.5f, -0.5f}, {0.5f, 0.5f}, {-0.5f, 0.5f}};
    collider->createPolygon(vertices, 4);

    auto physicsBody = entity->getComponent<CPhysicsBody2D>();

    // Simulate until polygon settles on ground
    for (int i = 0; i < 200; ++i)
    {
        S2DPhysics::instance().update(1.0f / 60.0f);
    }

    b2Vec2 finalPos = physicsBody->getPosition();

    // Polygon should have fallen and rested on ground
    EXPECT_GT(finalPos.y, -5.0f);  // Above ground center
    EXPECT_LT(finalPos.y, 5.0f);   // Below initial position

    // Velocity should be near zero (resting)
    b2Vec2 velocity = physicsBody->getLinearVelocity();
    EXPECT_NEAR(velocity.y, 0.0f, 1.0f);
}

TEST_F(CCollider2DPolygonTest, PolygonCollidesWithPolygon)
{
    // Create static polygon ground
    auto ground        = createPhysicsEntity(Vec2(0.0f, -5.0f), BodyType::Static);
    auto groundCollider = ground->addComponent<CCollider2D>();

    b2Vec2 groundVerts[4] = {{-5.0f, -0.5f}, {5.0f, -0.5f}, {5.0f, 0.5f}, {-5.0f, 0.5f}};
    groundCollider->createPolygon(groundVerts, 4);

    // Create dynamic polygon
    auto entity = createPhysicsEntity(Vec2(0.0f, 5.0f));
    auto collider = entity->addComponent<CCollider2D>();

    b2Vec2 vertices[3] = {{0.0f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f}};
    collider->createPolygon(vertices, 3);

    auto physicsBody = entity->getComponent<CPhysicsBody2D>();

    // Simulate
    for (int i = 0; i < 200; ++i)
    {
        S2DPhysics::instance().update(1.0f / 60.0f);
    }

    b2Vec2 finalPos = physicsBody->getPosition();

    // Should rest on ground
    EXPECT_GT(finalPos.y, -5.0f);
    EXPECT_LT(finalPos.y, 5.0f);
}

// ============================================================================
// Serialization Tests
// ============================================================================

TEST_F(CCollider2DPolygonTest, SerializePolygon)
{
    auto entity = createPhysicsEntity();

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    b2Vec2 vertices[4] = {{-1.0f, -0.5f}, {1.0f, -0.5f}, {1.0f, 0.5f}, {-1.0f, 0.5f}};
    collider->createPolygon(vertices, 4, 0.05f);
    collider->setDensity(1.5f);
    collider->setFriction(0.7f);
    collider->setRestitution(0.3f);

    Serialization::JsonBuilder builder;
    collider->serialize(builder);

    std::string json = builder.toString();

    // Verify JSON contains expected keys
    EXPECT_NE(json.find("Polygon"), std::string::npos);
    EXPECT_NE(json.find("vertexCount"), std::string::npos);
    EXPECT_NE(json.find("vertices"), std::string::npos);
    EXPECT_NE(json.find("radius"), std::string::npos);
}

TEST_F(CCollider2DPolygonTest, DeserializePolygon)
{
    // Create JSON manually
    std::string json = R"({
        "cCollider2D": {
            "shapeType": "Polygon",
            "vertexCount": 3,
            "radius": 0.1,
            "vertices": [
                {"x": 0.0, "y": 1.0},
                {"x": -0.5, "y": -0.5},
                {"x": 0.5, "y": -0.5}
            ],
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
    EXPECT_EQ(collider->getShapeType(), ColliderShape::Polygon);
    EXPECT_EQ(collider->getPolygonVertexCount(), 3);
    EXPECT_FLOAT_EQ(collider->getPolygonRadius(), 0.1f);
    EXPECT_FLOAT_EQ(collider->getDensity(), 2.0f);
    EXPECT_FLOAT_EQ(collider->getFriction(), 0.5f);
    EXPECT_FLOAT_EQ(collider->getRestitution(), 0.2f);
}

TEST_F(CCollider2DPolygonTest, SerializeDeserializeRoundTrip)
{
    auto entity1 = createPhysicsEntity();
    auto collider1 = entity1->addComponent<CCollider2D>();

    b2Vec2 vertices[5] = {{0.0f, 1.0f}, {0.95f, 0.31f}, {0.59f, -0.81f}, {-0.59f, -0.81f},
                          {-0.95f, 0.31f}};
    collider1->createPolygon(vertices, 5, 0.08f);
    collider1->setDensity(1.2f);

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
    EXPECT_EQ(collider2->getShapeType(), collider1->getShapeType());
    EXPECT_EQ(collider2->getPolygonVertexCount(), collider1->getPolygonVertexCount());
    EXPECT_FLOAT_EQ(collider2->getPolygonRadius(), collider1->getPolygonRadius());
    EXPECT_FLOAT_EQ(collider2->getDensity(), collider1->getDensity());
}

// ============================================================================
// Edge Cases and Error Handling
// ============================================================================

TEST_F(CCollider2DPolygonTest, GetPolygonPropertiesOnNonPolygon)
{
    auto entity = createPhysicsEntity();
    auto collider = entity->addComponent<CCollider2D>();

    // Create a circle, not a polygon
    collider->createCircle(0.5f);

    // Polygon getters should return safe defaults
    EXPECT_EQ(collider->getPolygonVertices(), nullptr);
    EXPECT_EQ(collider->getPolygonVertexCount(), 0);
    EXPECT_FLOAT_EQ(collider->getPolygonRadius(), 0.0f);
}

TEST_F(CCollider2DPolygonTest, RecreatePolygonShape)
{
    auto entity = createPhysicsEntity();
    auto collider = entity->addComponent<CCollider2D>();

    // Create initial polygon
    b2Vec2 vertices1[3] = {{0.0f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f}};
    collider->createPolygon(vertices1, 3);
    EXPECT_TRUE(collider->isInitialized());

    // Recreate with different polygon
    b2Vec2 vertices2[4] = {{-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f}};
    collider->createPolygon(vertices2, 4);

    EXPECT_TRUE(collider->isInitialized());
    EXPECT_EQ(collider->getPolygonVertexCount(), 4);
}

TEST_F(CCollider2DPolygonTest, CreatePolygonWithoutPhysicsBody)
{
    auto& manager = SEntity::instance();
    auto entity = manager.addEntity("no_physics");

    auto transform = entity->addComponent<CTransform>();
    transform->setPosition(Vec2(0.0f, 0.0f));

    auto collider = entity->addComponent<CCollider2D>();

    b2Vec2 vertices[3] = {{0.0f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f}};
    collider->createPolygon(vertices, 3);

    // Should not be initialized without physics body
    EXPECT_FALSE(collider->isInitialized());
}

// ============================================================================
// Complex Shape Test (Car Chassis Example)
// ============================================================================

TEST_F(CCollider2DPolygonTest, CreateCarChassisShape)
{
    auto entity = createPhysicsEntity(Vec2(0.0f, 5.0f));

    auto collider = entity->addComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);

    // Car chassis shape from Box2D example (scaled)
    float scale     = 1.0f;
    b2Vec2 vertices[6] = {{-1.5f * scale, -0.5f * scale},
                          {1.5f * scale, -0.5f * scale},
                          {1.5f * scale, 0.0f * scale},
                          {0.0f * scale, 0.9f * scale},
                          {-1.15f * scale, 0.9f * scale},
                          {-1.5f * scale, 0.2f * scale}};

    // Adjust vertices slightly
    for (int i = 0; i < 6; ++i)
    {
        vertices[i].x *= 0.85f;
        vertices[i].y *= 0.85f;
    }

    collider->createPolygon(vertices, 6, 0.15f * scale);
    collider->setDensity(1.0f);
    collider->setFriction(0.2f);

    EXPECT_TRUE(collider->isInitialized());
    EXPECT_EQ(collider->getShapeType(), ColliderShape::Polygon);
    EXPECT_EQ(collider->getPolygonVertexCount(), 6);
}

// ============================================================================
// Bounding Box Tests
// ============================================================================

TEST_F(CCollider2DPolygonTest, GetBoundsForCircle)
{
    auto entity = createPhysicsEntity();
    auto collider = entity->addComponent<CCollider2D>();
    
    collider->createCircle(0.5f, {0.0f, 0.0f});
    
    float width, height;
    bool result = collider->getBounds(width, height);
    
    EXPECT_TRUE(result);
    EXPECT_NEAR(width, 1.0f, 0.001f);   // diameter = 2 * radius
    EXPECT_NEAR(height, 1.0f, 0.001f);
}

TEST_F(CCollider2DPolygonTest, GetBoundsForBox)
{
    auto entity = createPhysicsEntity();
    auto collider = entity->addComponent<CCollider2D>();
    
    collider->createBox(1.5f, 2.0f);  // halfWidth, halfHeight
    
    float width, height;
    bool result = collider->getBounds(width, height);
    
    EXPECT_TRUE(result);
    EXPECT_NEAR(width, 3.0f, 0.001f);   // 2 * halfWidth
    EXPECT_NEAR(height, 4.0f, 0.001f);  // 2 * halfHeight
}

TEST_F(CCollider2DPolygonTest, GetBoundsForSinglePolygon)
{
    auto entity = createPhysicsEntity();
    auto collider = entity->addComponent<CCollider2D>();
    
    // Square polygon
    b2Vec2 vertices[4] = {{-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f}};
    collider->createPolygon(vertices, 4);
    
    float width, height;
    bool result = collider->getBounds(width, height);
    
    EXPECT_TRUE(result);
    EXPECT_NEAR(width, 2.0f, 0.001f);
    EXPECT_NEAR(height, 2.0f, 0.001f);
}

TEST_F(CCollider2DPolygonTest, GetBoundsForMultiplePolygons)
{
    auto entity = createPhysicsEntity();
    auto collider = entity->addComponent<CCollider2D>();
    
    // Create first polygon (hull)
    b2Vec2 hull[4] = {{-0.5f, -1.0f}, {0.5f, -1.0f}, {0.5f, 0.0f}, {-0.5f, 0.0f}};
    collider->createPolygon(hull, 4);
    
    // Add second polygon (bow)
    b2Vec2 bow[3] = {{-0.3f, 0.0f}, {0.3f, 0.0f}, {0.0f, 1.5f}};
    collider->addPolygon(bow, 3);
    
    float width, height;
    bool result = collider->getBounds(width, height);
    
    EXPECT_TRUE(result);
    EXPECT_NEAR(width, 1.0f, 0.001f);   // -0.5 to 0.5
    EXPECT_NEAR(height, 2.5f, 0.001f);  // -1.0 to 1.5
}

TEST_F(CCollider2DPolygonTest, GetBoundsForEmptyCollider)
{
    auto entity = createPhysicsEntity();
    auto collider = entity->addComponent<CCollider2D>();
    
    // Don't create any shape
    float width, height;
    bool result = collider->getBounds(width, height);
    
    EXPECT_FALSE(result);
    EXPECT_FLOAT_EQ(width, 0.0f);
    EXPECT_FLOAT_EQ(height, 0.0f);
}

TEST_F(CCollider2DPolygonTest, GetBoundsForComplexBoatShape)
{
    auto entity = createPhysicsEntity();
    auto collider = entity->addComponent<CCollider2D>();
    
    // Simulate boat hull (like in the example)
    const float boatLength = 0.875f;  // 0.25 * 3.5
    const float boatWidth = 0.45f;    // 0.25 * 1.8
    
    b2Vec2 hullVertices[6] = {
        {-boatWidth * 0.35f, -boatLength * 0.45f},  // Back left
        {boatWidth * 0.35f, -boatLength * 0.45f},   // Back right
        {boatWidth * 0.5f, -boatLength * 0.1f},     // Mid-back right
        {boatWidth * 0.5f, 0.0f},                   // Front right
        {-boatWidth * 0.5f, 0.0f},                  // Front left
        {-boatWidth * 0.5f, -boatLength * 0.1f}     // Mid-back left
    };
    collider->createPolygon(hullVertices, 6);
    
    // Add bow polygon
    const float bowLength = boatLength * 0.55f;
    b2Vec2 bowVertices[4] = {
        {-boatWidth * 0.5f, 0.0f},
        {boatWidth * 0.5f, 0.0f},
        {boatWidth * 0.25f, bowLength},
        {-boatWidth * 0.25f, bowLength}
    };
    collider->addPolygon(bowVertices, 4);
    
    float width, height;
    bool result = collider->getBounds(width, height);
    
    EXPECT_TRUE(result);
    EXPECT_NEAR(width, boatWidth, 0.001f);  // Full width
    EXPECT_GT(height, boatLength * 0.45f);  // Should include hull back
    EXPECT_GT(height, bowLength);            // Should include bow
}
