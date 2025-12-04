#ifndef COMPONENT_H
#define COMPONENT_H

#include <JsonBuilder.h>
#include <JsonValue.h>
#include <string>

class Entity;  // Forward declaration

/**
 * @brief Base class for all components in the Entity Component System
 *
 * @description
 * Component is the base class from which all game components inherit. Components
 * represent individual aspects of game objects (entities) such as physics,
 * rendering, or behavior. The class provides a common interface for initialization,
 * updates, serialization, and type identification. Components can be enabled or
 * disabled at runtime. Each component has a unique GUID for identification.
 */
class Component
{
public:
    Component();

    /** @brief Virtual destructor for proper cleanup of derived classes */
    virtual ~Component() = default;

    /**
     * @brief Initializes the component
     * Called after the component is added to an entity
     */
    virtual void init() {};

    /**
     * @brief Updates the component's state
     * @param deltaTime Time elapsed since last update
     *
     * This method has an empty default implementation since not all components
     * need to update their state every frame. Components that do need per-frame
     * updates (like Transform or Gravity) should override this method.
     */
    virtual void update(float deltaTime) {};

    /**
     * @brief Serializes the component to binary data
     * @param builder The JSON builder to serialize to
     */
    virtual void serialize(JsonBuilder& builder) const = 0;

    /**
     * @brief Deserializes the component from binary data
     * @param value The JSON value to deserialize
     */
    virtual void deserialize(const JsonValue& value) = 0;

    /**
     * @brief Gets the type identifier of the component
     * @return String identifying the component type
     */
    virtual std::string getType() const = 0;

    /**
     * @brief Checks if the component is active
     * @return true if the component is active, false otherwise
     */
    bool isActive() const;

    /**
     * @brief Gets the owner of the component
     * @return Pointer to the owner entity
     */
    Entity* getOwner() const;

    /**
     * @brief Sets the owner of the component
     * @param owner Pointer to the owner entity
     */
    void setOwner(Entity* owner);

    /**
     * @brief Sets the active state of the component
     * @param active The new active state
     */
    void setActive(bool active);

    /**
     * @brief Gets the unique GUID of this component
     * @return The component's GUID string
     */
    const std::string& getGuid() const;

    /**
     * @brief Sets the GUID of this component (used during deserialization)
     * @param guid The GUID string to set
     */
    void setGuid(const std::string& guid);

private:
    Entity*     m_owner  = nullptr;  ///< Pointer to the entity that owns this component
    bool        m_active = true;     ///< Flag indicating if the component is active
    std::string m_guid;              ///< Unique identifier for this component
};

#endif  // COMPONENT_H