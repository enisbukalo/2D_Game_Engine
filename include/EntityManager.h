#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include "Entity.h"

using json = nlohmann::json;

class EntityManager
{
public:
#pragma region Constructors
    EntityManager()  = default;
    ~EntityManager() = default;
#pragma endregion

#pragma region Methods
    void                                        update(float deltaTime);
    std::shared_ptr<Entity>                     addEntity(const std::string& tag);
    void                                        removeEntity(std::shared_ptr<Entity> entity);
    const std::vector<std::shared_ptr<Entity>>& getEntities() const;
    std::vector<std::shared_ptr<Entity>>        getEntitiesByTag(const std::string& tag);
#pragma endregion

#pragma region Templates
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
    void saveToFile(const std::string& filename);
    void loadFromFile(const std::string& filename);
#pragma endregion

private:
#pragma region Methods
    void removeDeadEntities();
#pragma endregion

#pragma region Variables
    std::vector<std::shared_ptr<Entity>>                                  m_entities;
    std::vector<std::shared_ptr<Entity>>                                  m_entitiesToAdd;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Entity>>> m_entityMap;
    uint8_t                                                               m_totalEntities = 0;
#pragma endregion
};

#endif  // ENTITYMANAGER_H