#pragma once

namespace Components
{

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
 * @brief Physics body data used by the physics system.
 *
 * All Box2D resource ownership and simulation logic lives in `Systems::S2DPhysics`.
 * This component only stores configuration that the system consumes when creating
 * or updating the underlying rigid body.
 */
struct CPhysicsBody2D
{
    BodyType bodyType{BodyType::Dynamic};

    // Material defaults used when attaching fixtures
    float density{1.0f};
    float friction{0.3f};
    float restitution{0.15f};

    // Body properties
    bool  fixedRotation{false};
    float linearDamping{0.25f};
    float angularDamping{0.10f};
    float gravityScale{1.0f};

    inline BodyType getBodyType() const
    {
        return bodyType;
    }
    inline void setBodyType(BodyType newType)
    {
        bodyType = newType;
    }

    inline float getDensity() const
    {
        return density;
    }
    inline void setDensity(float d)
    {
        density = d;
    }
    inline float getFriction() const
    {
        return friction;
    }
    inline void setFriction(float f)
    {
        friction = f;
    }
    inline float getRestitution() const
    {
        return restitution;
    }
    inline void setRestitution(float r)
    {
        restitution = r;
    }
};

}  // namespace Components
