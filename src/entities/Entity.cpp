#include "Entity.h"
#include "ComponentFactory.h"
#include "Guid.h"

namespace Entity
{

Entity::Entity(const std::string& tag, size_t id) : m_tag(tag), m_id(id), m_guid(::Internal::Guid::generate()) {}

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

void Entity::serialize(Serialization::JsonBuilder& builder) const
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
        component->serialize(builder);
    }
    builder.endArray();
    builder.endObject();
}

void Entity::deserialize(const Serialization::SSerialization::JsonValue& value)
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
            // First check for audio components that use "type" and "data" keys
            // (std::map sorts alphabetically, so "data" comes before "type")
            if (component.hasKey("type") && component.hasKey("data"))
            {
                type = component["type"].getString();
            }
            else
            {
                type = obj.begin()->first;

                // Map JSON keys to registered component type names
                if (type == "cTransform")
                    type = "Transform";
                else if (type == "cName")
                    type = "Name";
                else if (type == "cPhysicsBody2D")
                    type = "CPhysicsBody2D";
                else if (type == "cCollider2D")
                    type = "CCollider2D";
                else if (type == "cRenderable")
                    type = "CRenderable";
                else if (type == "cTexture")
                    type = "CTexture";
                else if (type == "cShader")
                    type = "CShader";
                else if (type == "cMaterial")
                    type = "CMaterial";
                else if (type == "cInputController")
                    type = "CInputController";
                else if (type == "cParticleEmitter")
                    type = "CParticleEmitter";
                else if (type == "cAudioSource")
                    type = "CAudioSource";
                else if (type == "cAudioListener")
                    type = "CAudioListener";
            }
        }

        ::Components::Component* comp = ::Components::ComponentFactory::instance().createComponent(type);
        if (comp)
        {
            std::unique_ptr<::Components::Component> newComponent(comp);
            newComponent->setOwner(this);
            newComponent->deserialize(component);
            m_components[std::type_index(typeid(*comp))] = std::move(newComponent);
        }
    }

    // Initialize all components after deserialization
    // Order matters: CPhysicsBody2D must be initialized before CCollider2D
    // because colliders need to attach to physics bodies

    // First pass: Initialize physics bodies
    for (auto& [type, component] : m_components)
    {
        if (component && component->getType() == "CPhysicsBody2D")
        {
            component->init();
        }
    }

    // Second pass: Initialize colliders (they need physics bodies to exist)
    for (auto& [type, component] : m_components)
    {
        if (component && component->getType() == "CCollider2D")
        {
            component->init();
        }
    }

    // Third pass: Initialize all other components
    for (auto& [type, component] : m_components)
    {
        if (component && component->getType() != "CPhysicsBody2D" && component->getType() != "CCollider2D")
        {
            component->init();
        }
    }
}

}  // namespace Entity
