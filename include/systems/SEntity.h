#ifndef SENTITY_H
#define SENTITY_H

#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include "Entity.h"

namespace Systems
{

/**
 * @brief System for handling entities in the game engine
 *
 * @description
 * SEntity is a singleton class responsible for creating, destroying, and managing all entities
 * in the game. It provides functionality for entity lifecycle management, querying
 * entities by tags or components, and serialization of the game state. The system
 * uses a deferred system for entity creation and destruction to prevent issues
 * during iteration.
 */
class SEntity
{
public:
    /**
     * @brief Gets the singleton instance of the SEntity
     * @return Reference to the SEntity instance
     */
    static SEntity& instance();

    // Delete copy constructor and assignment operator
    SEntity(const SEntity&)            = delete;
    SEntity& operator=(const SEntity&) = delete;

    /**
     * @brief Updates all active entities and processes pending operations
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime);

    /**
     * @brief Creates a new entity with the specified tag
     * @param tag The tag to assign to the entity
     * @return Shared pointer to the created entity
     */
    std::shared_ptr<::Entity::Entity> addEntity(const std::string& tag);

    /**
     * @brief Creates a new entity of the specified derived type with the given tag and constructor args
     * @tparam T Derived entity type (must inherit from Entity)
     * @tparam Args Constructor argument types
     * @param tag Tag to assign to the entity
     * @param args Arguments forwarded to the derived entity constructor
     * @return Shared pointer to the created derived entity
     *
     * The entity is initialized immediately and ready to use when this method returns.
     * The entity's init() method is called before returning, so all components are fully set up.
     */
    template <typename T, typename... Args>
    std::shared_ptr<T> addEntity(const std::string& tag, Args&&... args)
    {
        static_assert(std::is_base_of<::Entity::Entity, T>::value, "T must derive from Entity");

        auto derived = std::shared_ptr<T>(new T(tag, m_totalEntities++, std::forward<Args>(args)...));

        // Add to active lists immediately
        m_activeEntities.push_back(derived);
        m_entities.push_back(derived);
        m_entityMap[tag].push_back(derived);

        // Initialize entity immediately so it's ready to use
        derived->init();

        return derived;
    }

    /**
     * @brief Removes an entity from the system
     * @param entity The entity to remove
     */
    void removeEntity(std::shared_ptr<::Entity::Entity> entity);

    /**
     * @brief Gets all active entities
     * @return Const reference to the vector of entity pointers
     */
    const std::vector<std::shared_ptr<::Entity::Entity>>& getEntities() const;

    /**
     * @brief Gets all entities with a specific tag
     * @param tag The tag to search for
     * @return Vector of entity pointers matching the tag
     */
    std::vector<std::shared_ptr<::Entity::Entity>> getEntitiesByTag(const std::string& tag);

    /**
     * @brief Gets all entities that have a specific component type
     * @tparam T The component type to search for
     * @return Vector of pointers to entities with the specified component
     */
    template <typename T>
    std::vector<::Entity::Entity*> getEntitiesWithComponent()
    {
        std::vector<::Entity::Entity*> result;
        for (auto& entity : m_activeEntities)
        {
            if (entity->isAlive() && entity->hasComponent<T>())
            {
                result.push_back(entity.get());
            }
        }
        for (auto& entity : m_inactiveEntities)
        {
            if (entity->isAlive() && entity->hasComponent<T>())
            {
                result.push_back(entity.get());
            }
        }
        return result;
    };

    /**
     * @brief Gets all entities that have a component of the specified type or derived from it
     * @tparam T The base component type to search for
     * @return Vector of pointers to entities with components derived from T
     *
     * This uses dynamic_cast internally, so it's slower than getEntitiesWithComponent
     * but works with inheritance hierarchies.
     */
    template <typename T>
    std::vector<::Entity::Entity*> getEntitiesWithComponentDerived()
    {
        std::vector<::Entity::Entity*> result;
        for (auto& entity : m_activeEntities)
        {
            if (entity->isAlive() && entity->hasComponentDerived<T>())
            {
                result.push_back(entity.get());
            }
        }
        for (auto& entity : m_inactiveEntities)
        {
            if (entity->isAlive() && entity->hasComponentDerived<T>())
            {
                result.push_back(entity.get());
            }
        }
        return result;
    };

    /**
     * @brief Saves the current game state to a file
     * @param filename Path to the file to save to
     */
    void saveToFile(const std::string& filename);

    /**
     * @brief Loads game state from a file
     * @param filename Path to the file to load from
     */
    void loadFromFile(const std::string& filename);

    /**
     * @brief Clears all entities and resets the manager state
     */
    void clear();

    /**
     * @brief Moves an entity between active and inactive lists
     * @param entity The entity to move
     * @param active True to move to active, false to move to inactive
     */
    void moveEntityBetweenLists(::Entity::Entity* entity, bool active);

private:
    // Private constructor to prevent direct instantiation
    SEntity() = default;
    // Private destructor to prevent deletion through pointers
    ~SEntity() = default;

    /**
     * @brief Removes entities that have been marked for destruction
     */
    void removeDeadEntities();

    std::vector<std::shared_ptr<::Entity::Entity>> m_entities;  ///< List of all entities (deprecated, kept for compatibility)
    std::vector<std::shared_ptr<::Entity::Entity>> m_activeEntities;    ///< List of active entities
    std::vector<std::shared_ptr<::Entity::Entity>> m_inactiveEntities;  ///< List of inactive entities
    std::vector<std::shared_ptr<::Entity::Entity>> m_entitiesToAdd;     ///< Queue of entities to be added
    std::unordered_map<std::string, std::vector<std::shared_ptr<::Entity::Entity>>> m_entityMap;  ///< Map of entities by tag
    size_t m_totalEntities = 0;  ///< Counter for generating unique entity IDs
};

}  // namespace Systems

#endif  // SENTITY_H