#ifndef S2D_PHYSICS_H
#define S2D_PHYSICS_H

#include "System.h"

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

private:
    // Private constructor to prevent direct instantiation
    S2DPhysics() = default;
    // Private destructor to prevent deletion through pointers
    ~S2DPhysics() override = default;
};

#endif  // S2D_PHYSICS_H
