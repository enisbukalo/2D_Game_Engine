#include "Entity.h"

void Entity::destroy()
{
    m_alive = false;
}

bool Entity::isAlive() const
{
    return m_alive;
}

uint8_t Entity::getId() const
{
    return m_id;
}

const std::string& Entity::getTag() const
{
    return m_tag;
}

void Entity::update(float deltaTime)
{
    for (auto& [type, component] : m_components)
    {
        if (component && component->isActive())
        {
            component->update(deltaTime);
        }
    }
}

void Entity::serialize() const {}

void Entity::deserialize() {}
