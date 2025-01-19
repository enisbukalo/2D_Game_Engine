#include <algorithm>
#include <fstream>
#include "EntityManager.h"

void EntityManager::update(float deltaTime)
{
    // Add any pending entities
    for (auto& entity : m_entitiesToAdd)
    {
        m_entities.push_back(entity);
        m_entityMap[entity->getTag()].push_back(entity);
    }
    m_entitiesToAdd.clear();

    // Update all entities
    for (auto& entity : m_entities)
    {
        if (entity->isAlive())
        {
            entity->update(deltaTime);
        }
    }

    // Remove dead entities
    removeDeadEntities();
}

std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag)
{
    auto entity = std::shared_ptr<Entity>(new Entity(tag, m_totalEntities++));
    m_entitiesToAdd.push_back(entity);
    return entity;
}

void EntityManager::removeEntity(std::shared_ptr<Entity> entity)
{
    entity->destroy();
}

const std::vector<std::shared_ptr<Entity>>& EntityManager::getEntities() const
{
    return m_entities;
}

std::vector<std::shared_ptr<Entity>> EntityManager::getEntitiesByTag(const std::string& tag)
{
    return m_entityMap[tag];
}

void EntityManager::saveToFile(const std::string& filename)
{
    json j;
    j["totalEntities"] = m_totalEntities;

    json entitiesArray = json::array();
    for (const auto& entity : m_entities)
    {
        if (entity && entity->isAlive())
        {
            entitiesArray.push_back(entity->serialize());
        }
    }
    j["entities"] = entitiesArray;

    std::ofstream file(filename);
    file << j.dump(4);  // Pretty print with 4-space indent
}

void EntityManager::loadFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + filename);
    }

    json j;
    file >> j;

    // Clear existing entities
    m_entities.clear();
    m_entitiesToAdd.clear();
    m_entityMap.clear();

    if (j.contains("totalEntities"))
    {
        m_totalEntities = j["totalEntities"];
    }

    if (j.contains("entities"))
    {
        for (const auto& entityData : j["entities"])
        {
            std::string tag    = entityData["tag"];
            auto        entity = addEntity(tag);
            entity->deserialize(entityData);
        }
        update(0.0f);  // Process pending entities
    }
}

void EntityManager::removeDeadEntities()
{
    // Remove from main entity vector
    m_entities.erase(std::remove_if(m_entities.begin(),
                                    m_entities.end(),
                                    [](const auto& entity) { return !entity->isAlive(); }),
                     m_entities.end());

    // Remove from tag maps
    for (auto& [tag, entities] : m_entityMap)
    {
        entities.erase(std::remove_if(entities.begin(),
                                      entities.end(),
                                      [](const auto& entity) { return !entity->isAlive(); }),
                       entities.end());
    }
}