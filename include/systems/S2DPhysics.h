#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include "Entity.h"
#include "System.h"
#include "box2d/box2d.h"

namespace Components
{
class CPhysicsBody2D;
struct CTransform;
}

namespace Systems
{

/**
 * @brief Box2D Physics System - Manages the Box2D physics world and simulation
 *
 * This system wraps Box2D v3.1.1 and provides:
 * - Physics world management
 * - Body creation and destruction
 * - Collision detection and response
 * - Physics simulation stepping
 * - Spatial queries (AABB, raycasting)
 *
 * Coordinate System: Y-up (positive Y = upward)
 * Units: 1 unit = 1 meter
 * Default Gravity: (0, -10) m/s²
 */
class S2DPhysics : public System
{
private:
    b2WorldId m_worldId;

    // Entity handle to b2BodyId mapping (generation-aware via Entity key)
    std::unordered_map<Entity, b2BodyId> m_entityBodyMap;

    // Registered physics bodies for fixed-update callbacks
    std::vector<Components::CPhysicsBody2D*> m_registeredBodies;

    // Timestep settings
    float m_timeStep;
    int   m_subStepCount;

    // Internal helpers
    void ensureBodyForEntity(Entity entity, Components::CTransform& transform, Components::CPhysicsBody2D& body);
    void pruneDestroyedBodies(const World& world);

public:
public:
public:
    S2DPhysics();
    ~S2DPhysics();

    // Delete copy and move constructors/assignment operators
    S2DPhysics(const S2DPhysics&)            = delete;
    S2DPhysics(S2DPhysics&&)                 = delete;
    S2DPhysics& operator=(const S2DPhysics&) = delete;
    S2DPhysics& operator=(S2DPhysics&&)      = delete;

    /**
     * @brief Update the physics simulation
     * @param deltaTime Time elapsed since last update (not used - fixed timestep)
     */
    void update(float deltaTime, World& world) override;

    /**
     * @brief Get the Box2D world ID
     */
    b2WorldId getWorldId() const
    {
        return m_worldId;
    }

    /**
     * @brief Set world gravity
     * @param gravity Gravity vector in m/s² (e.g., {0, -10} for standard Earth gravity)
     */
    void setGravity(const b2Vec2& gravity);

    /**
     * @brief Get world gravity
     */
    b2Vec2 getGravity() const;

    /**
     * @brief Set the fixed timestep for physics simulation
     * @param timeStep Time step in seconds (default: 1/60)
     */
    void setTimeStep(float timeStep)
    {
        m_timeStep = timeStep;
    }

    /**
     * @brief Get the fixed timestep
     */
    float getTimeStep() const
    {
        return m_timeStep;
    }

    /**
     * @brief Set the number of sub-steps per physics update
     * @param subStepCount Number of sub-steps (default: 4)
     */
    void setSubStepCount(int subStepCount)
    {
        m_subStepCount = subStepCount;
    }

    /**
     * @brief Get the number of sub-steps
     */
    int getSubStepCount() const
    {
        return m_subStepCount;
    }

    /**
     * @brief Create a Box2D body for an entity
     * @param entity Entity ID to associate with the body
     * @param bodyDef Body definition
     * @return Box2D body ID
     */
    b2BodyId createBody(Entity entity, const b2BodyDef& bodyDef);

    /**
     * @brief Destroy the Box2D body associated with an entity
     * @param entity Entity ID whose body should be destroyed
     */
    void destroyBody(Entity entity);

    /**
     * @brief Get the Box2D body associated with an entity
     * @param entity Entity ID to query
     * @return Body ID (invalid if entity has no body)
     */
    b2BodyId getBody(Entity entity);

    /**
     * @brief Query the world for all bodies overlapping an AABB
     * @param aabb Axis-aligned bounding box to query
     * @param callback Callback function called for each overlapping body
     */
    void queryAABB(const b2AABB& aabb, b2OverlapResultFcn* callback, void* context);

    /**
     * @brief Cast a ray through the world
     * @param origin Ray origin point
     * @param translation Ray direction and length
     * @param callback Callback function called for each hit
     */
    void rayCast(const b2Vec2& origin, const b2Vec2& translation, b2CastResultFcn* callback, void* context);

    /**
     * @brief Register a physics body for fixed-update callbacks
     * @param body Physics body component to register
     *
     * This is called automatically by CPhysicsBody2D::initialize().
     * Do not call manually unless you know what you're doing.
     */
    void registerBody(Components::CPhysicsBody2D* body);

    /**
     * @brief Unregister a physics body from fixed-update callbacks
     * @param body Physics body component to unregister
     *
     * This is called automatically by CPhysicsBody2D destructor.
     * Do not call manually unless you know what you're doing.
     */
    void unregisterBody(Components::CPhysicsBody2D* body);

    /**
     * @brief Run fixed-update callbacks for all registered physics bodies
     * @param timeStep Fixed timestep value to pass to callbacks
     *
     * This is called by GameEngine before each physics step.
     * It iterates all registered bodies and invokes their fixed-update callbacks.
     */
    void runFixedUpdates(float timeStep);
};

}  // namespace Systems
