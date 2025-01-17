#include "../components/Component.h"

json Component::serialize() const
{
    return json{{"type", getType()}};
}

bool Component::isActive() const
{
    return m_active;
}

void Component::setActive(bool active)
{
    m_active = active;
}