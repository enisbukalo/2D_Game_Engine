#pragma once

#include "Component.h"
#include "box2d/box2d.h"

class CTransform;

/**
 * @brief Body type enumeration for physics bodies
 */
enum class BodyType
{
    Static,     // Zero mass, zero velocity, may be manually moved
    Kinematic,  // Zero mass, non-zero velocity set by user, moved by solver
    Dynamic     // Positive mass, non-zero velocity determined by forces, moved by solver
};

/**
 * @brief Physics Body Component - Wraps a Box2D rigid body
 *
 * This component represents a physics body in the Box2D world.
 * It must be paired with a CTransform component.
 *
 * Material properties (density, friction, restitution) are stored here as defaults
 * and applied when fixtures (CCollider2D) are attached.
 *
 * Note: The body must be initialized by calling initialize() after adding to entity.
 */
class CPhysicsBody2D : public Component
{
private:
    b2BodyId m_bodyId;
    BodyType m_bodyType;

    // Material properties (default values for fixtures)
    float m_density;
    float m_friction;
    float m_restitution;

    // Body properties
    bool  m_fixedRotation;
    float m_linearDamping;
    float m_angularDamping;
    float m_gravityScale;

    bool m_initialized;

public:
    CPhysicsBody2D();
    ~CPhysicsBody2D() override;

    /**
     * @brief Initialize the physics body
     * @param position Initial position in meters
     * @param type Body type (Static, Kinematic, or Dynamic)
     */
    void initialize(const b2Vec2& position, BodyType type = BodyType::Dynamic);

    /**
     * @brief Check if the body has been initialized
     */
    bool isInitialized() const
    {
        return m_initialized;
    }

    /**
     * @brief Get the Box2D body ID
     */
    b2BodyId getBodyId() const
    {
        return m_bodyId;
    }

    /**
     * @brief Set the body type
     */
    void setBodyType(BodyType type);

    /**
     * @brief Get the body type
     */
    BodyType getBodyType() const
    {
        return m_bodyType;
    }

    /**
     * @brief Set default density for fixtures
     */
    void setDensity(float density)
    {
        m_density = density;
    }

    /**
     * @brief Get default density
     */
    float getDensity() const
    {
        return m_density;
    }

    /**
     * @brief Set default friction for fixtures
     */
    void setFriction(float friction)
    {
        m_friction = friction;
    }

    /**
     * @brief Get default friction
     */
    float getFriction() const
    {
        return m_friction;
    }

    /**
     * @brief Set default restitution (bounciness) for fixtures
     */
    void setRestitution(float restitution)
    {
        m_restitution = restitution;
    }

    /**
     * @brief Get default restitution
     */
    float getRestitution() const
    {
        return m_restitution;
    }

    /**
     * @brief Set whether rotation is fixed
     */
    void setFixedRotation(bool fixed);

    /**
     * @brief Check if rotation is fixed
     */
    bool isFixedRotation() const
    {
        return m_fixedRotation;
    }

    /**
     * @brief Set linear damping (resistance to linear motion)
     */
    void setLinearDamping(float damping);

    /**
     * @brief Get linear damping
     */
    float getLinearDamping() const
    {
        return m_linearDamping;
    }

    /**
     * @brief Set angular damping (resistance to rotation)
     */
    void setAngularDamping(float damping);

    /**
     * @brief Get angular damping
     */
    float getAngularDamping() const
    {
        return m_angularDamping;
    }

    /**
     * @brief Set gravity scale (multiplier for world gravity)
     */
    void setGravityScale(float scale);

    /**
     * @brief Get gravity scale
     */
    float getGravityScale() const
    {
        return m_gravityScale;
    }

    /**
     * @brief Apply a force at a world point
     * @param force Force vector in Newtons
     * @param point Application point in world coordinates
     */
    void applyForce(const b2Vec2& force, const b2Vec2& point);

    /**
     * @brief Apply a force at the center of mass
     * @param force Force vector in Newtons
     */
    void applyForceToCenter(const b2Vec2& force);

    /**
     * @brief Apply an impulse at a world point
     * @param impulse Impulse vector in N*s
     * @param point Application point in world coordinates
     */
    void applyLinearImpulse(const b2Vec2& impulse, const b2Vec2& point);

    /**
     * @brief Apply an impulse at the center of mass
     * @param impulse Impulse vector in N*s
     */
    void applyLinearImpulseToCenter(const b2Vec2& impulse);

    /**
     * @brief Apply an angular impulse
     * @param impulse Angular impulse in N*m*s
     */
    void applyAngularImpulse(float impulse);

    /**
     * @brief Apply a torque (continuous rotational force)
     * @param torque Torque in N*m
     */
    void applyTorque(float torque);

    /**
     * @brief Set linear velocity
     * @param velocity Velocity in m/s
     */
    void setLinearVelocity(const b2Vec2& velocity);

    /**
     * @brief Get linear velocity
     */
    b2Vec2 getLinearVelocity() const;

    /**
     * @brief Set angular velocity
     * @param omega Angular velocity in rad/s
     */
    void setAngularVelocity(float omega);

    /**
     * @brief Get angular velocity
     */
    float getAngularVelocity() const;

    /**
     * @brief Get body position
     */
    b2Vec2 getPosition() const;

    /**
     * @brief Get body rotation (angle)
     */
    float getRotation() const;

    /**
     * @brief Get the forward direction vector (Y-axis) based on current rotation
     * @return Forward direction as b2Vec2
     */
    b2Vec2 getForwardVector() const;

    /**
     * @brief Get the right direction vector (X-axis) based on current rotation
     * @return Right direction as b2Vec2
     */
    b2Vec2 getRightVector() const;

    /**
     * @brief Sync Box2D body state to CTransform component
     * @param transform Transform component to update
     */
    void syncToTransform(CTransform* transform);

    /**
     * @brief Sync CTransform component state to Box2D body
     * @param transform Transform component to read from
     */
    void syncFromTransform(const CTransform* transform);

    // Component interface
    void        init() override;
    void        serialize(JsonBuilder& builder) const override;
    void        deserialize(const JsonValue& value) override;
    std::string getType() const override;
};
