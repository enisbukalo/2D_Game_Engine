#include "Component.h"
#include "Guid.h"
#include "systems/SComponentManager.h"

namespace Components
{

Component::Component() : m_guid(::Internal::Guid::generate()) {}

bool Component::isActive() const
{
    return m_active;
}

Entity Component::getOwner() const
{
    return m_owner;
}

void Component::setOwner(Entity owner)
{
    m_owner = owner;
}

void Component::setActive(bool active)
{
    if (m_active == active)
        return;
    m_active = active;
    ::Systems::SComponentManager::instance().setActive(this, m_active);
}

Component::~Component()
{
    ::Systems::SComponentManager::instance().unregisterComponent(this);
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
