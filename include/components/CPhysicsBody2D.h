#pragma once

#include "Entity.h"

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

    // Owning entity (set by systems for bookkeeping)
    Entity owner{Entity::null()};
};

}  // namespace Components
