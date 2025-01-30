#include "EntityManager.h"
#include <algorithm>
#include <fstream>

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
    JsonBuilder builder;

    // Start the root object
    builder.beginObject();

    // Add entities array
    builder.addKey("entities");
    builder.beginArray();

    // Serialize each entity
    for (const auto& entity : m_entities)
    {
        if (entity->isAlive())
        {
            entity->serialize(builder);
        }
    }

    builder.endArray();   // end entities
    builder.endObject();  // end root

    // Write to file
    std::ofstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file for writing: " + filename);
    }
    file << builder.toString();
}

void EntityManager::loadFromFile(const std::string& filename) {}

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