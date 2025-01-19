#include "Component.h"

bool Component::isActive() const
{
    return m_active;
}

void Component::setActive(bool active)
{
    m_active = active;
}