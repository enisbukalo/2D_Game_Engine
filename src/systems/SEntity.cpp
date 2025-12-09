#include "SEntity.h"
#include <algorithm>
#include <fstream>
#include "FileUtilities.h"
#include "SComponentManager.h"

namespace Systems
{

SEntity& SEntity::instance()
{
    static SEntity instance;
    return instance;
}

void SEntity::update(float deltaTime)
{
    // Add any pending entities to active list (supports nested additions during init)
    while (!m_entitiesToAdd.empty())
    {
        auto pending = std::move(m_entitiesToAdd);
        m_entitiesToAdd.clear();

        for (auto& entity : pending)
        {
            m_activeEntities.push_back(entity);
            m_entities.push_back(entity);  // Keep for backward compatibility
            m_entityMap[entity->getTag()].push_back(entity);
            // Call init() so derived entities can perform their startup logic
            entity->init();
        }
    }

    // Update game logic in active entities BEFORE components
    // This allows entities to set up state that components may use
    for (auto& entity : m_activeEntities)
    {
        if (entity->isAlive() && entity->isActive())
        {
            entity->update(deltaTime);
        }
    }

    // Update all active components after entity logic
    ::Systems::SComponentManager::instance().updateAll(deltaTime);

    // Remove dead entities
    removeDeadEntities();
}

std::shared_ptr<::Entity::Entity> SEntity::addEntity(const std::string& tag)
{
    auto entity = std::shared_ptr<::Entity::Entity>(new ::Entity::Entity(tag, m_totalEntities++));
    m_entitiesToAdd.push_back(entity);
    return entity;
}

void SEntity::removeEntity(std::shared_ptr<::Entity::Entity> entity)
{
    entity->destroy();
}

const std::vector<std::shared_ptr<::Entity::Entity>>& SEntity::getEntities() const
{
    return m_entities;
}

std::vector<std::shared_ptr<::Entity::Entity>> SEntity::getEntitiesByTag(const std::string& tag)
{
    return m_entityMap[tag];
}

void SEntity::saveToFile(const std::string& filename)
{
    Serialization::JsonBuilder builder;

    // Start the root object
    builder.beginObject();

    // Add entities array
    builder.addKey("entities");
    builder.beginArray();

    // Serialize each active entity
    for (const auto& entity : m_activeEntities)
    {
        if (entity->isAlive())
        {
            entity->serialize(builder);
        }
    }

    // Serialize each inactive entity
    for (const auto& entity : m_inactiveEntities)
    {
        if (entity->isAlive())
        {
            entity->serialize(builder);
        }
    }

    builder.endArray();   // end entities
    builder.endObject();  // end root

    // Write to file
    ::Internal::FileUtilities::writeFile(filename, builder.toString());
}

void SEntity::loadFromFile(const std::string& filename)
{
    std::string                              json = ::Internal::FileUtilities::readFile(filename);
    Serialization::JsonParser                parser(json);
    Serialization::SSerialization::JsonValue root = Serialization::SSerialization::JsonValue::parse(parser);

    if (!root.isObject())
    {
        throw std::runtime_error("Invalid file format: " + filename);
    }

    const auto& entities = root["entities"].getArray();
    for (const auto& entity : entities)
    {
        std::shared_ptr<::Entity::Entity> newEntity = addEntity(entity["tag"].getString());
        newEntity->deserialize(entity);
    }
}

void SEntity::clear()
{
    m_entities.clear();
    m_activeEntities.clear();
    m_inactiveEntities.clear();
    m_entitiesToAdd.clear();
    m_entityMap.clear();
    m_totalEntities = 0;
}

void SEntity::removeDeadEntities()
{
    // Remove from main entity vector
    m_entities.erase(std::remove_if(m_entities.begin(),
                                    m_entities.end(),
                                    [](const auto& entity) { return !entity->isAlive(); }),
                     m_entities.end());

    // Remove from active entities
    m_activeEntities.erase(std::remove_if(m_activeEntities.begin(),
                                          m_activeEntities.end(),
                                          [](const auto& entity) { return !entity->isAlive(); }),
                           m_activeEntities.end());

    // Remove from inactive entities
    m_inactiveEntities.erase(std::remove_if(m_inactiveEntities.begin(),
                                            m_inactiveEntities.end(),
                                            [](const auto& entity) { return !entity->isAlive(); }),
                             m_inactiveEntities.end());

    // Remove from tag maps
    for (auto& [tag, entities] : m_entityMap)
    {
        entities.erase(std::remove_if(entities.begin(),
                                      entities.end(),
                                      [](const auto& entity) { return !entity->isAlive(); }),
                       entities.end());
    }
}

void SEntity::moveEntityBetweenLists(::Entity::Entity* entity, bool active)
{
    if (!entity)
        return;

    // Find the entity in both lists
    auto activeIt = std::find_if(m_activeEntities.begin(),
                                 m_activeEntities.end(),
                                 [entity](const auto& e) { return e.get() == entity; });

    auto inactiveIt = std::find_if(m_inactiveEntities.begin(),
                                   m_inactiveEntities.end(),
                                   [entity](const auto& e) { return e.get() == entity; });

    if (active)
    {
        // Move from inactive to active
        if (inactiveIt != m_inactiveEntities.end())
        {
            m_activeEntities.push_back(*inactiveIt);
            m_inactiveEntities.erase(inactiveIt);
        }
    }
    else
    {
        // Move from active to inactive
        if (activeIt != m_activeEntities.end())
        {
            m_inactiveEntities.push_back(*activeIt);
            m_activeEntities.erase(activeIt);
        }
    }
}

}  // namespace Systems