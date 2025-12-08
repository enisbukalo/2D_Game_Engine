#include "Component.h"
#include "Guid.h"

namespace Components
{

Component::Component() : m_guid(::Internal::Guid::generate()) {}

bool Component::isActive() const
{
    return m_active;
}

::Entity::Entity* Component::getOwner() const
{
    return m_owner;
}

void Component::setOwner(::Entity::Entity* owner)
{
    m_owner = owner;
}

void Component::setActive(bool active)
{
    m_active = active;
}

const std::string& Component::getGuid() const
{
    return m_guid;
}

void Component::setGuid(const std::string& guid)
{
    m_guid = guid;
}

}  // namespace Components
