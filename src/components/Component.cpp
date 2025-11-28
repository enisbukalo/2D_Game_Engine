#include "Component.h"
#include "Guid.h"

Component::Component() : m_guid(Guid::generate()) {}

bool Component::isActive() const
{
    return m_active;
}

Entity* Component::getOwner() const
{
    return m_owner;
}

void Component::setOwner(Entity* owner)
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
