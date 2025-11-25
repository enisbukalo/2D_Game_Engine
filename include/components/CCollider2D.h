#pragma once

#include "Component.h"
#include "box2d/box2d.h"

class CPhysicsBody2D;

/**
 * @brief Collider shape type enumeration
 */
enum class ColliderShape
{
    Circle,
    Box,
    Polygon
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
class CCollider2D : public Component
{
private:
    b2ShapeId     m_shapeId;
    ColliderShape m_shapeType;

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
    } m_shapeData;

    // Fixture properties
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
     * @brief Check if the collider has been initialized
     */
    bool isInitialized() const
    {
        return m_initialized;
    }

    /**
     * @brief Get the Box2D shape ID
     */
    b2ShapeId getShapeId() const
    {
        return m_shapeId;
    }

    /**
     * @brief Get the shape type
     */
    ColliderShape getShapeType() const
    {
        return m_shapeType;
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

    // Component interface
    void        init() override;
    void        serialize(JsonBuilder& builder) const override;
    void        deserialize(const JsonValue& value) override;
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
