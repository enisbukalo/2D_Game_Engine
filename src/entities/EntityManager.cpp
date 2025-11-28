#include "EntityManager.h"
#include <algorithm>
#include <fstream>
#include "FileUtilities.h"

EntityManager& EntityManager::instance()
{
    static EntityManager instance;
    return instance;
}

void EntityManager::update(float deltaTime)
{
    // Add any pending entities
    for (auto& entity : m_entitiesToAdd)
    {
        m_entities.push_back(entity);
        m_entityMap[entity->getTag()].push_back(entity);
        m_guidMap[entity->getGuid()] = entity;
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
    destroyEntityAndChildren(entity);
}

std::shared_ptr<Entity> EntityManager::getEntityByGuid(const std::string& guid)
{
    auto it = m_guidMap.find(guid);
    if (it != m_guidMap.end())
    {
        return it->second;
    }
    return nullptr;
}

void EntityManager::removeEntityByGuid(const std::string& guid)
{
    auto entity = getEntityByGuid(guid);
    if (entity)
    {
        destroyEntityAndChildren(entity);
    }
}

void EntityManager::destroyEntityAndChildren(std::shared_ptr<Entity> entity)
{
    if (!entity)
    {
        return;
    }

    // Recursively destroy all children first
    auto children = entity->getChildren();
    for (auto& child : children)
    {
        destroyEntityAndChildren(child);
    }

    // Mark this entity for destruction
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
    FileUtilities::writeFile(filename, builder.toString());
}

void EntityManager::loadFromFile(const std::string& filename)
{
    std::string json = FileUtilities::readFile(filename);
    JsonParser  parser(json);
    JsonValue   root = JsonValue::parse(parser);

    if (!root.isObject())
    {
        throw std::runtime_error("Invalid file format: " + filename);
    }

    const auto& entities = root["entities"].getArray();

    // Two-pass loading:
    // Pass 1: Create all entities and register them by GUID
    std::unordered_map<std::string, std::string> parentGuidMap;  // child GUID -> parent GUID

    for (const auto& entityData : entities)
    {
        std::shared_ptr<Entity> newEntity = addEntity(entityData["tag"].getString());
        newEntity->deserialize(entityData);

        // Store parent relationship for second pass
        if (entityData.hasKey("parentGuid"))
        {
            parentGuidMap[newEntity->getGuid()] = entityData["parentGuid"].getString();
        }
    }

    // Flush pending entities to guidMap
    for (auto& entity : m_entitiesToAdd)
    {
        m_guidMap[entity->getGuid()] = entity;
    }

    // Pass 2: Resolve parent relationships
    for (const auto& [childGuid, parentGuid] : parentGuidMap)
    {
        auto child = getEntityByGuid(childGuid);
        auto parent = getEntityByGuid(parentGuid);

        if (child && parent)
        {
            child->setParent(parent);
        }
    }
}

void EntityManager::clear()
{
    m_entities.clear();
    m_entitiesToAdd.clear();
    m_entityMap.clear();
    m_guidMap.clear();
    m_totalEntities = 0;
}

void EntityManager::removeDeadEntities()
{
    // Remove from GUID map first
    for (auto it = m_guidMap.begin(); it != m_guidMap.end();)
    {
        if (!it->second->isAlive())
        {
            it = m_guidMap.erase(it);
        }
        else
        {
            ++it;
        }
    }

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