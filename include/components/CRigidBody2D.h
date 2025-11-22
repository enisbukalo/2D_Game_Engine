#pragma once

#include "Component.h"
#include "Vec2.h"

/**
 * CRigidBody2D - Unified physics component for 2D rigid body dynamics
 *
 * Replaces CGravity and CForceDebug components with a single, comprehensive
 * physics interface. Manages mass, material properties, forces, and constraints.
 */
class CRigidBody2D : public Component
{
private:
    // Mass properties
    float m_mass;
    float m_inverseMass;  // Cached for performance (1/mass, or 0 for infinite mass)

    // Material properties
    float m_restitution;  // Bounciness factor (0 = no bounce, 1 = perfect bounce)
    float m_friction;     // Surface friction coefficient (0 = ice, 1 = maximum grip)
    float m_linearDrag;   // Air resistance/drag (0 = no drag, higher = more resistance)
    float m_angularDrag;  // Rotational drag (for future angular physics)

    // Gravity settings
    bool  m_useGravity;    // Whether global gravity affects this body
    float m_gravityScale;  // Gravity multiplier (replaces CGravity component)

    // Body type
    bool m_isKinematic;  // If true, not affected by forces (e.g., moving platforms)

    // Constraints
    bool m_freezePositionX;  // Prevent movement on X axis
    bool m_freezePositionY;  // Prevent movement on Y axis
    bool m_freezeRotation;   // Prevent rotation (for future angular physics)

    // Force accumulation (replaces CForceDebug functionality)
    Vec2 m_accumulatedForce;  // Forces accumulated this frame
    Vec2 m_totalForce;        // Total force from last physics update (for visualization)

public:
    CRigidBody2D();
    ~CRigidBody2D() override = default;

    // Component interface
    void        update(float deltaTime) override;
    std::string getType() const override
    {
        return "RigidBody2D";
    }
    void serialize(JsonBuilder& builder) const override;
    void deserialize(const JsonValue& value) override;

    // Force management
    void addForce(const Vec2& force);
    void addImpulse(const Vec2& impulse);  // Instant velocity change
    void clearForces();
    Vec2 getTotalForce() const
    {
        return m_totalForce;
    }
    Vec2 getAccumulatedForce() const
    {
        return m_accumulatedForce;
    }

    // Mass property getters/setters
    float getMass() const
    {
        return m_mass;
    }
    float getInverseMass() const
    {
        return m_inverseMass;
    }
    void setMass(float mass);

    // Material property getters/setters
    float getRestitution() const
    {
        return m_restitution;
    }
    void setRestitution(float restitution)
    {
        m_restitution = restitution;
    }

    float getFriction() const
    {
        return m_friction;
    }
    void setFriction(float friction)
    {
        m_friction = friction;
    }

    float getLinearDrag() const
    {
        return m_linearDrag;
    }
    void setLinearDrag(float drag)
    {
        m_linearDrag = drag;
    }

    float getAngularDrag() const
    {
        return m_angularDrag;
    }
    void setAngularDrag(float drag)
    {
        m_angularDrag = drag;
    }

    // Gravity getters/setters
    bool getUseGravity() const
    {
        return m_useGravity;
    }
    void setUseGravity(bool useGravity)
    {
        m_useGravity = useGravity;
    }

    float getGravityScale() const
    {
        return m_gravityScale;
    }
    void setGravityScale(float scale)
    {
        m_gravityScale = scale;
    }

    // Body type getters/setters
    bool isKinematic() const
    {
        return m_isKinematic;
    }
    void setKinematic(bool kinematic)
    {
        m_isKinematic = kinematic;
    }

    // Constraint getters/setters
    bool getFreezePositionX() const
    {
        return m_freezePositionX;
    }
    void setFreezePositionX(bool freeze)
    {
        m_freezePositionX = freeze;
    }

    bool getFreezePositionY() const
    {
        return m_freezePositionY;
    }
    void setFreezePositionY(bool freeze)
    {
        m_freezePositionY = freeze;
    }

    bool getFreezeRotation() const
    {
        return m_freezeRotation;
    }
    void setFreezeRotation(bool freeze)
    {
        m_freezeRotation = freeze;
    }
};
