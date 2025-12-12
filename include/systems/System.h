#ifndef SYSTEM_H
#define SYSTEM_H

#include <vector>
#include "Entity.h"
#include "ISystem.h"

class World;

namespace Systems
{

/**
 * @brief Base class for all systems in the Entity Component System
 *
 * @description
 * System is the base class for all game systems that operate on entities and
 * their components. Systems implement the game logic that processes components,
 * such as physics, rendering, or AI. Each system type focuses on specific
 * component combinations and provides the behavior for those components.
 */
class System : public ISystem
{
public:
    /** @brief Default constructor */
    System();

    /** @brief Virtual destructor for proper cleanup of derived classes */
    virtual ~System();

    /**
     * @brief Updates the system's logic
     * @param deltaTime Time elapsed since last update in seconds
     *
     * This method is called each frame to update the system's state and
     * process its associated components. Derived classes must implement
     * this method to provide specific system behavior.
     */
    void update(float deltaTime, World& world) override = 0;
};

}  // namespace Systems

#endif  // SYSTEM_H