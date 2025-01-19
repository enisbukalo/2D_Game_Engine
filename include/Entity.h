#ifndef ENTITY_H
#define ENTITY_H

#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include "CGravity.h"
#include "CName.h"
#include "CTransform.h"
#include "Component.h"
#include "ComponentFactory.h"

using json = nlohmann::json;

/**
 * @brief Base class for all game entities in the engine
 *
 * @description
 * Entity is the core class of the Entity Component System. It manages a collection
 * of components that define its behavior and properties. Entities can be created,
 * destroyed, and modified at runtime, providing a flexible foundation for game objects.
 * The class supports serialization for save/load functionality and provides methods
 * for component management.
 */
class Entity : public std::enable_shared_from_this<Entity>
{
public:
#pragma region For Testing
    friend class EntityManager;
    friend class TestEntity;
#pragma endregion

    virtual ~Entity() = default;

#pragma region Templates
    /**
     * @brief Gets a component of the specified type
     * @tparam T The type of component to get
     * @return Pointer to the component if found, nullptr otherwise
     */
    template <typename T>
    T *getComponent()
    {
        auto it = m_components.find(std::type_index(typeid(T)));
        return it != m_components.end() ? static_cast<T *>(it->second.get()) : nullptr;
    }

    /**
     * @brief Adds a component of the specified type with given arguments
     * @tparam T The type of component to add
     * @tparam Args Types of arguments to forward to the component constructor
     * @param args Arguments to forward to the component constructor
     * @return Pointer to the newly created component
     */
    template <typename T, typename... Args>
    T *addComponent(Args &&...args)
    {
        T *component                             = new T(std::forward<Args>(args)...);
        component->owner                         = this;
        m_components[std::type_index(typeid(T))] = std::unique_ptr<Component>(component);
        component->init();
        return component;
    }

    /**
     * @brief Checks if the entity has a component of the specified type
     * @tparam T The type of component to check for
     * @return true if the component exists, false otherwise
     */
    template <typename T>
    bool hasComponent()
    {
        return m_components.find(std::type_index(typeid(T))) != m_components.end();
    }

    /**
     * @brief Removes a component of the specified type
     * @tparam T The type of component to remove
     */
    template <typename T>
    void removeComponent()
    {
        auto it = m_components.find(std::type_index(typeid(T)));
        if (it != m_components.end())
        {
            m_components.erase(it);
        }
    }
#pragma endregion

#pragma region Methods
    /**
     * @brief Marks the entity for destruction
     */
    void destroy();

    /**
     * @brief Checks if the entity is still alive
     * @return true if the entity is alive, false if marked for destruction
     */
    bool isAlive() const;

    /**
     * @brief Gets the entity's unique identifier
     * @return The entity's ID
     */
    uint8_t getId() const;

    /**
     * @brief Gets the entity's tag
     * @return The entity's tag string
     */
    const std::string &getTag() const;

    /**
     * @brief Updates the entity and all its components
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime);

    /**
     * @brief Serializes the entity to JSON
     * @return JSON object containing the entity's data
     */
    json serialize() const;

    /**
     * @brief Deserializes the entity from JSON
     * @param data JSON object containing entity data
     */
    void deserialize(const json &data);
#pragma endregion

protected:
#pragma region Constructors
    /**
     * @brief Constructs an entity with a tag and ID
     * @param tag The entity's tag for identification and grouping
     * @param id Unique identifier for the entity
     */
    Entity(const std::string &tag, uint8_t id) : m_tag(tag), m_id(id) {}
#pragma endregion

private:
#pragma region Variables
    std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;  ///< Map of components indexed by type
    const uint8_t     m_id    = 0;                                                 ///< Unique identifier
    const std::string m_tag   = "Default";                                         ///< Entity tag
    bool              m_alive = true;                                              ///< Entity state flag
#pragma endregion
};

#endif  // ENTITY_H