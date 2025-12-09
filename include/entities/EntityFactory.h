#ifndef ENTITY_FACTORY_H
#define ENTITY_FACTORY_H

#include "Entity.h"
#include "systems/SEntity.h"

namespace Entity
{

// Template implementation for Entity::create (requires SEntity to be fully defined)
template <typename T, typename... Args>
std::shared_ptr<T> Entity::create(const std::string& tag, Args&&... args)
{
    static_assert(std::is_base_of<Entity, T>::value, "T must derive from Entity");
    return Systems::SEntity::instance().addEntity<T>(tag, std::forward<Args>(args)...);
}

}  // namespace Entity

#endif  // ENTITY_FACTORY_H
