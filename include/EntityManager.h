#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "Entity.h"

class EntityManager
{
public:
    EntityManager()  = default;
    ~EntityManager() = default;

    void update(float deltaTime)
    {
        // Add any pending entities
        for (auto &entity : m_entitiesToAdd)
        {
            m_entities.push_back(entity);
            m_entityMap[entity->getTag()].push_back(entity);
        }
        m_entitiesToAdd.clear();

        // Update all entities
        for (auto &entity : m_entities)
        {
            if (entity->isAlive())
            {
                entity->update(deltaTime);
            }
        }

        // Remove dead entities
        removeDeadEntities();
    }

    std::shared_ptr<Entity> addEntity(const std::string &tag)
    {
        auto entity = std::shared_ptr<Entity>(new Entity(tag, m_totalEntities++));
        m_entitiesToAdd.push_back(entity);
        return entity;
    }

    void removeEntity(std::shared_ptr<Entity> entity)
    {
        entity->destroy();
    }

    const std::vector<std::shared_ptr<Entity>> &getEntities() const
    {
        return m_entities;
    }

    std::vector<std::shared_ptr<Entity>> getEntitiesByTag(const std::string &tag)
    {
        return m_entityMap[tag];
    }

    template <typename T>
    std::vector<Entity *> getEntitiesWithComponent()
    {
        std::vector<Entity *> result;
        for (auto &entity : m_entities)
        {
            if (entity->isAlive() && entity->hasComponent<T>())
            {
                result.push_back(entity.get());
            }
        }
        return result;
    }

private:
    void removeDeadEntities()
    {
        // Remove from main entity vector
        m_entities.erase(::std::remove_if(m_entities.begin(),
                                          m_entities.end(),
                                          [](const auto &entity) { return !entity->isAlive(); }),
                         m_entities.end());

        // Remove from tag maps
        for (auto &[tag, entities] : m_entityMap)
        {
            entities.erase(::std::remove_if(entities.begin(),
                                            entities.end(),
                                            [](const auto &entity) { return !entity->isAlive(); }),
                           entities.end());
        }
    }

    std::vector<std::shared_ptr<Entity>>                                  m_entities;
    std::vector<std::shared_ptr<Entity>>                                  m_entitiesToAdd;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Entity>>> m_entityMap;
    uint8_t                                                               m_totalEntities = 0;
};

#endif  // ENTITYMANAGER_H