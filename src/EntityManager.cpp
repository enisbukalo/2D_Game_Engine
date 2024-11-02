#include "EntityManager.h"

EntityManager::EntityManager() {};
EntityManager::~EntityManager() {};

std::shared_ptr<Entity> EntityManager::addEntity(const std::string &tag) { return std::make_shared<Entity>(new Entity(tag, m_totalEntities)); };

void EntityManager::update()
{
  for (std::shared_ptr<Entity> entity : m_toAdd) {
    m_entities.push_back(entity);
    m_entityMap[entity->cName->name].push_back(entity);
    m_totalEntities++;
  }

  m_toAdd.clear();

  for (std::shared_ptr<Entity> entity : m_entities) {
    if (!entity->isAlive()) {
      // Remove entity from m_entities.
      // Remove entity from m_entityMap[entity->tag()]
    }
  }
};