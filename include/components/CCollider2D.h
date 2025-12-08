#pragma once

#include <vector>
#include "Component.h"
#include "box2d/box2d.h"

namespace Components
{
class CPhysicsBody2D;
}

namespace Components
{

/**
 * @brief Collider shape type enumeration
 */
enum class ColliderShape
{
    Circle,
    Box,
    Polygon,
    Segment,
    ChainSegment
};

/**
 * @brief Collider Component - Wraps a Box2D fixture (shape)
 *
 * This component creates a collision shape attached to a physics body.
 * It requires a CPhysicsBody2D component to exist on the same entity.
 *
 * The collider can be a sensor (trigger) or a solid collider.
 * Sensors detect collisions but do not generate collision responses.
 */
/**
 * @brief Structure to hold data for a single shape/fixture
 */
struct ShapeFixture
{
    b2ShapeId     shapeId;
    ColliderShape shapeType;

    // Shape parameters
    union ShapeData
    {
        struct
        {
            b2Vec2 center;
            float  radius;
        } circle;
        struct
        {
            float halfWidth;
            float halfHeight;
        } box;
        struct
        {
            b2Vec2 vertices[B2_MAX_POLYGON_VERTICES];
            int    vertexCount;
            float  radius;
        } polygon;
        struct
        {
            b2Vec2 point1;
            b2Vec2 point2;
        } segment;
        struct
        {
            b2Vec2 ghost1;
            b2Vec2 point1;
            b2Vec2 point2;
            b2Vec2 ghost2;
        } chainSegment;
    } shapeData;
};

class CCollider2D : public Component
{
private:
    std::vector<ShapeFixture> m_fixtures;

    // Fixture properties (default for all fixtures)
    bool  m_isSensor;
    float m_density;
    float m_friction;
    float m_restitution;

    bool m_initialized;

public:
    CCollider2D();
    ~CCollider2D() override;

    /**
     * @brief Create a circle collider
     * @param radius Circle radius in meters
     * @param center Local center offset (default: origin)
     */
    void createCircle(float radius, const b2Vec2& center = {0.0f, 0.0f});

    /**
     * @brief Create a box collider
     * @param halfWidth Half-width of the box in meters
     * @param halfHeight Half-height of the box in meters
     */
    void createBox(float halfWidth, float halfHeight);

    /**
     * @brief Create a polygon collider from arbitrary vertices
     * @param vertices Array of vertices (will compute convex hull)
     * @param count Number of vertices
     * @param radius Skin radius for the polygon (default: 0.0f)
     * @note Vertices will be automatically sorted to form a convex hull
     */
    void createPolygon(const b2Vec2* vertices, int count, float radius = 0.0f);

    /**
     * @brief Add an additional polygon shape to this collider
     * @param vertices Array of vertices (will compute convex hull)
     * @param count Number of vertices
     * @param radius Skin radius for the polygon (default: 0.0f)
     * @note This adds another fixture to the same physics body
     */
    void addPolygon(const b2Vec2* vertices, int count, float radius = 0.0f);

    /**
     * @brief Create a segment (line) collider
     * @param point1 First endpoint
     * @param point2 Second endpoint
     */
    void createSegment(const b2Vec2& point1, const b2Vec2& point2);

    /**
     * @brief Add an additional segment to this collider
     * @param point1 First endpoint
     * @param point2 Second endpoint
     */
    void addSegment(const b2Vec2& point1, const b2Vec2& point2);

    /**
     * @brief Create a chain segment with ghost vertices to prevent ghost collisions
     * @param ghost1 Ghost vertex before point1
     * @param point1 First endpoint
     * @param point2 Second endpoint
     * @param ghost2 Ghost vertex after point2
     */
    void createChainSegment(const b2Vec2& ghost1, const b2Vec2& point1, const b2Vec2& point2, const b2Vec2& ghost2);

    /**
     * @brief Add an additional chain segment to this collider
     * @param ghost1 Ghost vertex before point1
     * @param point1 First endpoint
     * @param point2 Second endpoint
     * @param ghost2 Ghost vertex after point2
     */
    void addChainSegment(const b2Vec2& ghost1, const b2Vec2& point1, const b2Vec2& point2, const b2Vec2& ghost2);

    /**
     * @brief Create a polygon collider from a pre-computed hull
     * @param hull Pre-computed convex hull
     * @param radius Skin radius for the polygon (default: 0.0f)
     */
    void createPolygonFromHull(const b2Hull& hull, float radius = 0.0f);

    /**
     * @brief Create an offset polygon collider
     * @param hull Pre-computed convex hull
     * @param position Local position offset
     * @param rotation Rotation offset in radians
     * @param radius Skin radius for the polygon (default: 0.0f)
     */
    void createOffsetPolygon(const b2Hull& hull, const b2Vec2& position, float rotation, float radius = 0.0f);

    /**
     * @brief Check if the collider has been initialized
     */
    bool isInitialized() const
    {
        return m_initialized;
    }

    /**
     * @brief Get the Box2D shape ID of the first fixture
     */
    b2ShapeId getShapeId() const
    {
        return m_fixtures.empty() ? b2_nullShapeId : m_fixtures[0].shapeId;
    }

    /**
     * @brief Get the shape type of the first fixture
     */
    ColliderShape getShapeType() const
    {
        return m_fixtures.empty() ? ColliderShape::Box : m_fixtures[0].shapeType;
    }

    /**
     * @brief Get all fixtures in this collider
     */
    const std::vector<ShapeFixture>& getFixtures() const
    {
        return m_fixtures;
    }

    /**
     * @brief Get the number of fixtures in this collider
     */
    size_t getFixtureCount() const
    {
        return m_fixtures.size();
    }

    /**
     * @brief Set whether this is a sensor (trigger)
     * @param isSensor True for sensor, false for solid collider
     */
    void setIsSensor(bool isSensor);

    /**
     * @brief Check if this is a sensor
     */
    bool isSensor() const
    {
        return m_isSensor;
    }

    /**
     * @brief Set density (mass per area)
     * @param density Density in kg/m²
     */
    void setDensity(float density);

    /**
     * @brief Get density
     */
    float getDensity() const
    {
        return m_density;
    }

    /**
     * @brief Set friction coefficient
     * @param friction Friction (0 = no friction, 1 = high friction)
     */
    void setFriction(float friction);

    /**
     * @brief Get friction
     */
    float getFriction() const
    {
        return m_friction;
    }

    /**
     * @brief Set restitution (bounciness)
     * @param restitution Restitution (0 = no bounce, 1 = perfect bounce)
     */
    void setRestitution(float restitution);

    /**
     * @brief Get restitution
     */
    float getRestitution() const
    {
        return m_restitution;
    }

    /**
     * @brief Get circle radius (only valid for circle shapes)
     */
    float getCircleRadius() const;

    /**
     * @brief Get circle center (only valid for circle shapes)
     */
    b2Vec2 getCircleCenter() const;

    /**
     * @brief Get box half-width (only valid for box shapes)
     */
    float getBoxHalfWidth() const;

    /**
     * @brief Get box half-height (only valid for box shapes)
     */
    float getBoxHalfHeight() const;

    /**
     * @brief Get polygon vertices for a specific fixture (only valid for polygon shapes)
     * @param fixtureIndex Index of the fixture (default: 0)
     * @return Pointer to vertex array (nullptr if not a polygon)
     */
    const b2Vec2* getPolygonVertices(size_t fixtureIndex = 0) const;

    /**
     * @brief Get polygon vertex count for a specific fixture (only valid for polygon shapes)
     * @param fixtureIndex Index of the fixture (default: 0)
     * @return Number of vertices (0 if not a polygon)
     */
    int getPolygonVertexCount(size_t fixtureIndex = 0) const;

    /**
     * @brief Get polygon radius/skin for a specific fixture (only valid for polygon shapes)
     * @param fixtureIndex Index of the fixture (default: 0)
     * @return Radius value (0.0f if not a polygon)
     */
    float getPolygonRadius(size_t fixtureIndex = 0) const;

    /**
     * @brief Calculate the axis-aligned bounding box of all fixtures
     * @param outWidth Output width of the bounding box in meters
     * @param outHeight Output height of the bounding box in meters
     * @return True if bounds were calculated, false if no fixtures exist
     */
    bool getBounds(float& outWidth, float& outHeight) const;

    // Component interface
    void        init() override;
    void        serialize(Serialization::JsonBuilder& builder) const override;
    void        deserialize(const Serialization::SSerialization::JsonValue& value) override;
    std::string getType() const override;

private:
    /**
     * @brief Attach the shape to the physics body
     */
    void attachToBody();

    /**
     * @brief Destroy the current shape if it exists
     */
    void destroyShape();
};

}  // namespace Components
