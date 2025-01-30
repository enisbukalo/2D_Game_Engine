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

void Entity::serialize(JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("tag");
    builder.addString(m_tag);
    builder.addKey("id");
    builder.addNumber(m_id);
    builder.addKey("components");
    builder.beginArray();
    for (auto& [type, component] : m_components)
    {
        component->serialize(builder);
    }
    builder.endArray();
    builder.endObject();
}

void Entity::deserialize(const JsonValue& value) {}
