#ifndef ENTITY_H
#define ENTITY_H

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <typeindex>
#include <unordered_map>
#include "../components/CGravity.h"
#include "../components/CName.h"
#include "../components/CTransform.h"
#include "Component.h"

using json = nlohmann::json;

class Entity : public std::enable_shared_from_this<Entity>
{
public:
    friend class EntityManager;

    virtual ~Entity() = default;

    template <typename T>
    T *getComponent()
    {
        auto it = m_components.find(std::type_index(typeid(T)));
        return it != m_components.end() ? static_cast<T *>(it->second.get()) : nullptr;
    }

    template <typename T, typename... Args>
    T *addComponent(Args &&...args)
    {
        T *component                             = new T(std::forward<Args>(args)...);
        component->owner                         = this;
        m_components[std::type_index(typeid(T))] = std::unique_ptr<Component>(component);
        component->init();
        return component;
    }

    template <typename T>
    bool hasComponent()
    {
        return m_components.find(std::type_index(typeid(T))) != m_components.end();
    }

    template <typename T>
    void removeComponent()
    {
        auto it = m_components.find(std::type_index(typeid(T)));
        if (it != m_components.end())
        {
            m_components.erase(it);
        }
    }

    void destroy()
    {
        m_alive = false;
    }
    bool isAlive() const
    {
        return m_alive;
    }
    uint8_t getId() const
    {
        return m_id;
    }
    const std::string &getTag() const
    {
        return m_tag;
    }

    void update(float deltaTime)
    {
        for (auto &[type, component] : m_components)
        {
            if (component && component->isActive())
            {
                component->update(deltaTime);
            }
        }
    }

    json serialize() const
    {
        json j;
        j["id"]    = m_id;
        j["tag"]   = m_tag;
        j["alive"] = m_alive;

        json components = json::array();
        for (const auto &[type, component] : m_components)
        {
            if (component)
            {
                components.push_back(component->serialize());
            }
        }
        j["components"] = components;
        return j;
    }

    void deserialize(const json &data)
    {
        if (data.contains("components"))
        {
            for (const auto &componentData : data["components"])
            {
                if (componentData.contains("type"))
                {
                    std::string type = componentData["type"];
                    if (type == "Transform")
                    {
                        auto component = addComponent<CTransform>();
                        component->deserialize(componentData);
                    }
                    else if (type == "Name")
                    {
                        auto component = addComponent<CName>();
                        component->deserialize(componentData);
                    }
                    else if (type == "Gravity")
                    {
                        auto component = addComponent<CGravity>();
                        component->deserialize(componentData);
                    }
                }
            }
        }
    }

protected:
    Entity(const std::string &tag, uint8_t id) : m_tag(tag), m_id(id) {}

private:
    std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;
    const uint8_t                                                   m_id    = 0;
    const std::string                                               m_tag   = "Default";
    bool                                                            m_alive = true;
};

#endif  // ENTITY_H