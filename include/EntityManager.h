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
    EntityManager()  = default;
    ~EntityManager() = default;

    void                                        update(float deltaTime);
    std::shared_ptr<Entity>                     addEntity(const std::string& tag);
    void                                        removeEntity(std::shared_ptr<Entity> entity);
    const std::vector<std::shared_ptr<Entity>>& getEntities() const;
    std::vector<std::shared_ptr<Entity>>        getEntitiesByTag(const std::string& tag);

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

    // Serialization methods
    void saveToFile(const std::string& filename);
    void loadFromFile(const std::string& filename);

private:
    void removeDeadEntities();

    std::vector<std::shared_ptr<Entity>>                                  m_entities;
    std::vector<std::shared_ptr<Entity>>                                  m_entitiesToAdd;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Entity>>> m_entityMap;
    uint8_t                                                               m_totalEntities = 0;
};

#endif  // ENTITYMANAGER_H