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

  void update();
  std::shared_ptr<Entity> addEntity(const std::string &tag);
  void removeEntity(std::shared_ptr<Entity> entity);
  EntityVector getEntities();
  EntityVector getEntities(const std::string &tag);

private:
  EntityVector m_entities;
  EntityVector m_toAdd;
  EntityMap m_entityMap;
  uint8_t m_totalEntities = 0;
};

#endif // ENTITYMANAGER_H