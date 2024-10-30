#ifndef ENTITY_H
#define ENTITY_H

#include <memory>
#include <string>

#include "Component.h"

class Entity {
public:
  Entity(const std::string &tag, uint8_t id);
  ~Entity() {};

  std::shared_ptr<CName> cName;
  std::shared_ptr<CTransform> cTransform;
  std::shared_ptr<CGravity> cGravity;

private:
  const uint8_t m_id = 0;
  const std::string m_tag = "Default";
  bool m_alive = true;
};

#endif // ENTITY_H