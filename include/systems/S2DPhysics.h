#ifndef S2D_PHYSICS_H
#define S2D_PHYSICS_H

#include "System.h"
#include "components/CCollider.h"
#include "physics/CollisionDetector.h"
#include "physics/Quadtree.h"

// Forward declarations
class CTransform;
class CCircleCollider;
class CBoxCollider;

/**
 * @brief 2D Physics system for handling physics simulations
 *
 * @description
 * S2DPhysics is a singleton system responsible for managing physics
 * calculations and simulations in the 2D game world. It handles
 * gravity, collisions, and other physics-related computations.
 */
class S2DPhysics : public System
{
public:
    /**
     * @brief Gets the singleton instance of the S2DPhysics system
     * @return Reference to the S2DPhysics instance
     */
    static S2DPhysics& instance();

    // Delete copy constructor and assignment operator
    S2DPhysics(const S2DPhysics&)            = delete;
    S2DPhysics& operator=(const S2DPhysics&) = delete;

    /**
     * @brief Updates the physics simulation
     * @param deltaTime Time elapsed since last update in seconds
     *
     * Processes all entities with physics components, updating their
     * positions and velocities based on forces and collisions.
     */
    void update(float deltaTime) override;

    /**
     * @brief Sets the world bounds for physics calculations
     * @param center Center point of the world
     * @param size Size of the world (width and height)
     */
    void setWorldBounds(const Vec2& center, const Vec2& size);

    /**
     * @brief Gets the quadtree for visualization or debugging
     * @return Pointer to the quadtree, or nullptr if not initialized
     */
    const Quadtree* getQuadtree() const;

    /**
     * @brief Sets the global gravity vector
     * @param gravity The global gravity vector to apply
     */
    void setGlobalGravity(const Vec2& gravity);

    /**
     * @brief Gets the current global gravity vector
     * @return The global gravity vector
     */
    Vec2 getGlobalGravity() const;

private:
    // Private constructor to prevent direct instantiation
    S2DPhysics();
    // Private destructor to prevent deletion through pointers
    ~S2DPhysics() override = default;

    std::unique_ptr<Quadtree> m_quadtree;       ///< Spatial partitioning structure
    AABB                      m_worldBounds;    ///< World boundaries
    Vec2                      m_globalGravity;  ///< Global gravity vector applied to all entities

    /**
     * @brief Checks for collisions between entities
     */
    void checkCollisions();

    /**
     * @brief Updates the quadtree with current entity positions
     */
    void updateQuadtree();

    /**
     * @brief Handles gravity for all entities
     * @param deltaTime Time elapsed since last update in seconds
     */
    void handleGravity(float deltaTime);

    /**
     * @brief Integrates velocity to update positions
     * @param deltaTime Time elapsed since last update in seconds
     */
    void integratePositions(float deltaTime);

    /**
     * @brief Clears accumulated forces on all rigid bodies (saves to totalForce for visualization)
     */
    void clearForces();

    /**
     * @brief Applies linear drag to entities with rigid bodies
     * @param deltaTime Time elapsed since last update in seconds
     */
    void applyDrag(float deltaTime);

    /**
     * @brief Handles collisions between entities
     * @param a First entity involved in collision
     * @param b Second entity involved in collision
     */
    void handleCollision(Entity* a, Entity* b);

    /**
     * @brief Resolves collisions between entities
     * @param a First entity involved in collision
     * @param b Second entity involved in collision
     * @param colliderA Collider of first entity
     * @param colliderB Collider of second entity
     */
    void resolveCollision(Entity* a, Entity* b, const CCollider* colliderA, const CCollider* colliderB);

    /**
     * @brief Resolves circle-circle collision using edge-based contact points
     * @param transformA Transform of first entity
     * @param transformB Transform of second entity
     * @param circleA First circle collider
     * @param circleB Second circle collider
     * @param aIsStatic Whether first entity is static
     * @param bIsStatic Whether second entity is static
     * @param manifold Collision manifold with edge contact points
     * @param restitution Combined restitution value for the collision
     */
    void resolveCircleVsCircle(CTransform*              transformA,
                               CTransform*              transformB,
                               const CCircleCollider*   circleA,
                               const CCircleCollider*   circleB,
                               bool                     aIsStatic,
                               bool                     bIsStatic,
                               const CollisionManifold& manifold,
                               float                    restitution);

    /**
     * @brief Resolves circle-box collision using edge-based contact points
     * @param transformA Transform of first entity
     * @param transformB Transform of second entity
     * @param circleA Circle collider (can be nullptr if A is box)
     * @param boxA Box collider (can be nullptr if A is circle)
     * @param circleB Circle collider (can be nullptr if B is box)
     * @param boxB Box collider (can be nullptr if B is circle)
     * @param aIsStatic Whether first entity is static
     * @param bIsStatic Whether second entity is static
     * @param manifold Collision manifold with edge contact points
     * @param restitution Combined restitution value for the collision
     */
    void resolveCircleVsBox(CTransform*              transformA,
                            CTransform*              transformB,
                            const CCircleCollider*   circleA,
                            const CBoxCollider*      boxA,
                            const CCircleCollider*   circleB,
                            const CBoxCollider*      boxB,
                            bool                     aIsStatic,
                            bool                     bIsStatic,
                            const CollisionManifold& manifold,
                            float                    restitution);

    /**
     * @brief Resolves box-box collision using edge-based contact points
     * @param transformA Transform of first entity
     * @param transformB Transform of second entity
     * @param boxA First box collider
     * @param boxB Second box collider
     * @param aIsStatic Whether first entity is static
     * @param bIsStatic Whether second entity is static
     * @param manifold Collision manifold with edge contact points
     * @param restitution Combined restitution value for the collision
     */
    void resolveBoxVsBox(CTransform*              transformA,
                         CTransform*              transformB,
                         const CBoxCollider*      boxA,
                         const CBoxCollider*      boxB,
                         bool                     aIsStatic,
                         bool                     bIsStatic,
                         const CollisionManifold& manifold,
                         float                    restitution);
};

#endif  // S2D_PHYSICS_H
