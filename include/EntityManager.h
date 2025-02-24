#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "Entity.h"

/**
 * @brief Manager class for handling entities in the game engine
 *
 * @description
 * EntityManager is a singleton class responsible for creating, destroying, and managing all entities
 * in the game. It provides functionality for entity lifecycle management, querying
 * entities by tags or components, and serialization of the game state. The manager
 * uses a deferred system for entity creation and destruction to prevent issues
 * during iteration.
 */
class EntityManager
{
public:
    /**
     * @brief Gets the singleton instance of the EntityManager
     * @return Reference to the EntityManager instance
     */
    static EntityManager& instance();

    // Delete copy constructor and assignment operator
    EntityManager(const EntityManager&)            = delete;
    EntityManager& operator=(const EntityManager&) = delete;

    /**
     * @brief Updates all active entities and processes pending operations
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime);

    /**
     * @brief Creates a new entity with the specified tag
     * @param tag The tag to assign to the entity
     * @return Shared pointer to the newly created entity
     */
    std::shared_ptr<Entity> addEntity(const std::string& tag);

    /**
     * @brief Marks an entity for removal
     * @param entity The entity to remove
     */
    void removeEntity(std::shared_ptr<Entity> entity);

    /**
     * @brief Gets all active entities
     * @return Const reference to the vector of entity pointers
     */
    const std::vector<std::shared_ptr<Entity>>& getEntities() const;

    /**
     * @brief Gets all entities with a specific tag
     * @param tag The tag to search for
     * @return Vector of entity pointers matching the tag
     */
    std::vector<std::shared_ptr<Entity>> getEntitiesByTag(const std::string& tag);

    /**
     * @brief Gets all entities that have a specific component type
     * @tparam T The component type to search for
     * @return Vector of pointers to entities with the specified component
     */
    template <typename T>
    std::vector<Entity*> getEntitiesWithComponent()
    {
        std::vector<Entity*> result;
        for (auto& entity : m_entities)
        {
            if (entity->isAlive() && entity->hasComponent<T>())
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

private:
    // Private constructor to prevent direct instantiation
    EntityManager() = default;
    // Private destructor to prevent deletion through pointers
    ~EntityManager() = default;

    /**
     * @brief Removes entities that have been marked for destruction
     */
    void removeDeadEntities();

    std::vector<std::shared_ptr<Entity>> m_entities;       ///< List of all active entities
    std::vector<std::shared_ptr<Entity>> m_entitiesToAdd;  ///< Queue of entities to be added
    std::unordered_map<std::string, std::vector<std::shared_ptr<Entity>>> m_entityMap;  ///< Map of entities by tag
    uint8_t m_totalEntities = 0;  ///< Counter for generating unique entity IDs
};

#endif  // ENTITYMANAGER_H