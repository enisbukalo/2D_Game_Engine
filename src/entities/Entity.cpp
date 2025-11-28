#include "Entity.h"
#include "ComponentFactory.h"
#include "Guid.h"

Entity::Entity(const std::string &tag, size_t id) : m_tag(tag), m_id(id), m_guid(Guid::generate())
{
}

void Entity::destroy()
{
    m_alive = false;
}

bool Entity::isAlive() const
{
    return m_alive;
}

size_t Entity::getId() const
{
    return m_id;
}

const std::string& Entity::getGuid() const
{
    return m_guid;
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
    builder.addKey("guid");
    builder.addString(m_guid);
    builder.addKey("tag");
    builder.addString(m_tag);
    builder.addKey("id");
    builder.addNumber(m_id);
    builder.addKey("components");
    builder.beginArray();
    for (auto& [type, component] : m_components)
    {
        builder.beginObject();
        builder.addKey("guid");
        builder.addString(component->getGuid());
        builder.endObject();
        
        component->serialize(builder);
    }
    builder.endArray();
    builder.endObject();
}

void Entity::deserialize(const JsonValue& value)
{
    m_id = value["id"].getNumber();

    // Deserialize GUID if present
    if (value.hasKey("guid"))
    {
        m_guid = value["guid"].getString();
    }

    const auto& components = value["components"].getArray();
    for (const auto& component : components)
    {
        // Determine component type from the component object's first key
        std::string type;
        const auto& obj = component.getObject();
        if (!obj.empty())
        {
            type = obj.begin()->first;
            if (type == "cTransform")
                type = "Transform";
            else if (type == "cName")
                type = "Name";
            else if (type == "cPhysicsBody2D")
                type = "CPhysicsBody2D";
            else if (type == "cCollider2D")
                type = "CCollider2D";
        }

        Component* comp = ComponentFactory::instance().createComponent(type);
        if (comp)
        {
            std::unique_ptr<Component> newComponent(comp);
            newComponent->setOwner(this);
            newComponent->deserialize(component);
            m_components[std::type_index(typeid(*comp))] = std::move(newComponent);
        }
    }

    // Initialize all components after deserialization
    for (auto& [type, component] : m_components)
    {
        if (component)
        {
            component->init();
        }
    }
}
