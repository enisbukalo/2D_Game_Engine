#ifndef S2D_PHYSICS_H
#define S2D_PHYSICS_H

#include "System.h"
#include "components/CCollider.h"
#include "physics/Quadtree.h"

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

private:
    // Private constructor to prevent direct instantiation
    S2DPhysics();
    // Private destructor to prevent deletion through pointers
    ~S2DPhysics() override = default;

    std::unique_ptr<Quadtree> m_quadtree;     ///< Spatial partitioning structure
    AABB                      m_worldBounds;  ///< World boundaries

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

    // Add these new declarations
    void handleCollision(Entity* a, Entity* b);
    void resolveCollision(const Entity* a, const Entity* b, const CCollider* colliderA, const CCollider* colliderB);
};

#endif  // S2D_PHYSICS_H
