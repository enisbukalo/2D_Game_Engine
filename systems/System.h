#ifndef SYSTEM_H
#define SYSTEM_H

#include <vector>
#include "../components/CGravity.h"
#include "../components/CName.h"
#include "../components/CTransform.h"
#include "Component.h"
#include "Entity.h"

/**
 * @brief Base class for all systems in the Entity Component System
 *
 * @description
 * System is the base class for all game systems that operate on entities and
 * their components. Systems implement the game logic that processes components,
 * such as physics, rendering, or AI. Each system type focuses on specific
 * component combinations and provides the behavior for those components.
 */
class System
{
public:
#pragma region Constructors
    /** @brief Default constructor */
    System();

    /** @brief Virtual destructor for proper cleanup of derived classes */
    ~System();
#pragma endregion

#pragma region Virtual Methods
    /**
     * @brief Updates the system's logic
     *
     * This method is called each frame to update the system's state and
     * process its associated components. Derived classes must implement
     * this method to provide specific system behavior.
     */
    virtual void update() = 0;
#pragma endregion

private:
#pragma region Variables
    std::unordered_map<uint8_t, std::unique_ptr<Component>> m_components;  ///< Map of components managed by this system
#pragma endregion
};
#endif  // SYSTEM_H