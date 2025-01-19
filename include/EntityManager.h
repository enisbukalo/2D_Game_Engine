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
 * EntityManager is responsible for creating, destroying, and managing all entities
 * in the game. It provides functionality for entity lifecycle management, querying
 * entities by tags or components, and serialization of the game state. The manager
 * uses a deferred system for entity creation and destruction to prevent issues
 * during iteration.
 */
class EntityManager
{
public:
#pragma region Constructors
    /** @brief Default constructor */
    EntityManager() = default;

    /** @brief Default destructor */
    ~EntityManager() = default;
#pragma endregion

#pragma region Methods
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
#pragma endregion

#pragma region Templates
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
    }
#pragma endregion

#pragma region Serialization
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
#pragma endregion

private:
#pragma region Methods
    /**
     * @brief Removes entities that have been marked for destruction
     */
    void removeDeadEntities();
#pragma endregion

#pragma region Variables
    std::vector<std::shared_ptr<Entity>> m_entities;       ///< List of all active entities
    std::vector<std::shared_ptr<Entity>> m_entitiesToAdd;  ///< Queue of entities to be added
    std::unordered_map<std::string, std::vector<std::shared_ptr<Entity>>> m_entityMap;  ///< Map of entities by tag
    uint8_t m_totalEntities = 0;  ///< Counter for generating unique entity IDs
#pragma endregion
};

#endif  // ENTITYMANAGER_H