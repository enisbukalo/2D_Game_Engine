#include "Entity.h"
#include "ComponentFactory.h"
#include "CPhysicsBody2D.h"
#include "Guid.h"
#include "SBox2DPhysics.h"

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

    // Serialize parent GUID if exists
    auto parent = m_parent.lock();
    if (parent)
    {
        builder.addKey("parentGuid");
        builder.addString(parent->getGuid());
    }

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

    // Note: parentGuid is handled by EntityManager during two-pass loading

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

bool Entity::setParent(std::shared_ptr<Entity> parent)
{
    // Allow clearing parent
    if (!parent)
    {
        // Remove this entity from old parent's children
        auto oldParent = m_parent.lock();
        if (oldParent)
        {
            oldParent->removeChild(shared_from_this());
        }
        m_parent.reset();
        return true;
    }

    // Check for cycle: parent cannot be a descendant of this entity
    if (parent.get() == this || isDescendant(parent))
    {
        return false;
    }

    // Remove from old parent
    auto oldParent = m_parent.lock();
    if (oldParent)
    {
        oldParent->removeChild(shared_from_this());
    }

    // Set new parent
    m_parent = parent;
    parent->addChild(shared_from_this());

    // Update physics userData pointers if this entity or its children have physics bodies
    SBox2DPhysics::instance().updatePhysicsRootOwners(this);

    return true;
}

std::shared_ptr<Entity> Entity::getParent() const
{
    return m_parent.lock();
}

bool Entity::addChild(std::shared_ptr<Entity> child)
{
    if (!child || child.get() == this)
    {
        return false;
    }

    // Check if already a child
    for (const auto& weakChild : m_children)
    {
        if (auto existingChild = weakChild.lock())
        {
            if (existingChild == child)
            {
                return true;  // Already a child
            }
        }
    }

    // Check for cycles
    if (isDescendant(child))
    {
        return false;
    }

    m_children.push_back(child);
    return true;
}

void Entity::removeChild(std::shared_ptr<Entity> child)
{
    if (!child)
    {
        return;
    }

    m_children.erase(
        std::remove_if(m_children.begin(), m_children.end(),
                       [&child](const std::weak_ptr<Entity>& weakChild) {
                           auto lockedChild = weakChild.lock();
                           return !lockedChild || lockedChild == child;
                       }),
        m_children.end());
}

std::vector<std::shared_ptr<Entity>> Entity::getChildren() const
{
    std::vector<std::shared_ptr<Entity>> result;
    for (const auto& weakChild : m_children)
    {
        if (auto child = weakChild.lock())
        {
            result.push_back(child);
        }
    }
    return result;
}

bool Entity::isDescendant(std::shared_ptr<Entity> entity) const
{
    if (!entity)
    {
        return false;
    }

    // Check all children recursively
    for (const auto& weakChild : m_children)
    {
        if (auto child = weakChild.lock())
        {
            if (child == entity || child->isDescendant(entity))
            {
                return true;
            }
        }
    }
    return false;
}
