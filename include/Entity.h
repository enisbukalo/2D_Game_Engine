#ifndef ENTITY_H
#define ENTITY_H

#include <algorithm>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include "Component.h"

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
    friend class EntityManager;
    friend class TestEntity;

    virtual ~Entity() = default;

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
    };

    /**
     * @brief Gets a component of the specified type or derived from it
     * @tparam T The base type of component to get
     * @return Pointer to the component if found (may be a derived type), nullptr otherwise
     *
     * This method uses dynamic_cast to check all components, so it's slower than getComponent
     * but works with inheritance hierarchies.
     */
    template <typename T>
    T *getComponentDerived()
    {
        for (const auto &pair : m_components)
        {
            T *component = dynamic_cast<T *>(pair.second.get());
            if (component != nullptr)
            {
                return component;
            }
        }
        return nullptr;
    };

    /**
     * @brief Gets all of the components attached to this entity
     * @tparam T The type of components to get
     * @return Vector of pointers to components of the specified type
     *
     * This method retrieves all components of any type attached to this entity.
     */
    std::vector<Component *> getAllComponents()
    {
        std::vector<Component *> components;
        components.reserve(m_components.size());
        std::transform(m_components.begin(),
                       m_components.end(),
                       std::back_inserter(components),
                       [](const auto &pair) { return pair.second.get(); });
        return components;
    };

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
        T *component = new T(std::forward<Args>(args)...);
        component->setOwner(this);
        m_components[std::type_index(typeid(T))] = std::unique_ptr<Component>(component);
        component->init();
        return component;
    };

    /**
     * @brief Checks if the entity has a component of the specified type
     * @tparam T The type of component to check for
     * @return true if the component exists, false otherwise
     */
    template <typename T>
    bool hasComponent()
    {
        return m_components.find(std::type_index(typeid(T))) != m_components.end();
    };

    /**
     * @brief Checks if the entity has a component of the specified type or derived from it
     * @tparam T The base type of component to check for
     * @return true if a component of type T or derived from T exists, false otherwise
     *
     * This method uses dynamic_cast to check all components, so it's slower than hasComponent
     * but works with inheritance hierarchies.
     */
    template <typename T>
    bool hasComponentDerived()
    {
        return std::any_of(m_components.begin(),
                           m_components.end(),
                           [](const auto &pair) { return dynamic_cast<T *>(pair.second.get()) != nullptr; });
    };

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
    };

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
    size_t getId() const;

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
     * @brief Serializes the entity to binary data
     */
    void serialize(JsonBuilder &builder) const;

    /**
     * @brief Deserializes the entity from binary data
     */
    void deserialize(const JsonValue &value);

protected:
    /**
     * @brief Constructs an entity with a tag and ID
     * @param tag The entity's tag for identification and grouping
     * @param id Unique identifier for the entity
     */
    Entity(const std::string &tag, size_t id) : m_tag(tag), m_id(id) {}

private:
    std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;  ///< Map of components indexed by type
    size_t            m_id    = 0;                                                 ///< Unique identifier
    const std::string m_tag   = "Default";                                         ///< Entity tag
    bool              m_alive = true;                                              ///< Entity state flag
};

#endif  // ENTITY_H