#ifndef COMPONENT_H
#define COMPONENT_H

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class Entity;  // Forward declaration

/**
 * @brief Base class for all components in the Entity Component System
 *
 * @description
 * Component is the base class from which all game components inherit. Components
 * represent individual aspects of game objects (entities) such as physics,
 * rendering, or behavior. The class provides a common interface for initialization,
 * updates, serialization, and type identification. Components can be enabled or
 * disabled at runtime.
 */
class Component
{
public:
#pragma region Constructors
    Component() = default;

    /** @brief Virtual destructor for proper cleanup of derived classes */
    virtual ~Component() = default;
#pragma endregion

#pragma region Virtual Methods
    /**
     * @brief Initializes the component
     * Called after the component is added to an entity
     */
    virtual void init() {}

    /**
     * @brief Updates the component's state
     * @param deltaTime Time elapsed since last update
     */
    virtual void update(float deltaTime) {}

    /**
     * @brief Serializes the component to JSON
     * @return JSON object containing the component's data
     */
    virtual json serialize() const;

    /**
     * @brief Deserializes the component from JSON
     * @param data JSON object containing component data
     */
    virtual void deserialize(const json& data) {}

    /**
     * @brief Gets the type identifier of the component
     * @return String identifying the component type
     */
    virtual std::string getType() const = 0;
#pragma endregion

#pragma region Variables
    Entity* owner = nullptr;  ///< Pointer to the entity that owns this component
#pragma endregion

private:
#pragma region Variables
    bool m_active = true;  ///< Flag indicating if the component is active
#pragma endregion

public:
#pragma region Methods
    /**
     * @brief Checks if the component is active
     * @return true if the component is active, false otherwise
     */
    bool isActive() const;

    /**
     * @brief Sets the active state of the component
     * @param active The new active state
     */
    void setActive(bool active);
#pragma endregion
};

#endif  // COMPONENT_H