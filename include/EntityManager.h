#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Entity.h"

typedef std::vector<std::shared_ptr<Entity>> EntityVector;
typedef std::map<std::string, EntityVector> EntityMap;

class EntityManager {
public:
  EntityManager();
  ~EntityManager();

  std::shared_ptr<Entity> addEntity(const std::string &tag);
  EntityVector getEntities();
  EntityVector getEntities(const std::string &tag);

private:
  EntityVector m_entities;
  EntityMap m_entityMap;
  uint8_t m_totalEntities = 0;
};

#endif // ENTITYMANAGER_H