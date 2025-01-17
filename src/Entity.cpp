#include "../include/Entity.h"

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

json Entity::serialize() const
{
    json j;
    j["id"]    = m_id;
    j["tag"]   = m_tag;
    j["alive"] = m_alive;

    json components = json::array();
    for (const auto& [type, component] : m_components)
    {
        if (component)
        {
            components.push_back(component->serialize());
        }
    }
    j["components"] = components;
    return j;
}

void Entity::deserialize(const json& data)
{
    if (data.contains("components"))
    {
        for (const auto& componentData : data["components"])
        {
            if (componentData.contains("type"))
            {
                std::string type = componentData["type"];
                if (Component* component = ComponentFactory::instance().createComponent(type))
                {
                    component->owner                                  = this;
                    m_components[std::type_index(typeid(*component))] = std::unique_ptr<Component>(component);
                    component->deserialize(componentData);
                    component->init();
                }
            }
        }
    }
}