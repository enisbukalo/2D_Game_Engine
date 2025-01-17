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
#include "ComponentFactory.h"

using json = nlohmann::json;

class Entity : public std::enable_shared_from_this<Entity>
{
public:
    friend class EntityManager;
    friend class TestEntity;
    virtual ~Entity() = default;

#pragma region Templates
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
#pragma endregion

#pragma region Methods
    void               destroy();
    bool               isAlive() const;
    uint8_t            getId() const;
    const std::string &getTag() const;
    void               update(float deltaTime);
    json               serialize() const;
    void               deserialize(const json &data);
#pragma endregion

protected:
#pragma region Constructors
    Entity(const std::string &tag, uint8_t id) : m_tag(tag), m_id(id) {}
#pragma endregion

private:
#pragma region Variables
    std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;
    const uint8_t                                                   m_id    = 0;
    const std::string                                               m_tag   = "Default";
    bool                                                            m_alive = true;
#pragma endregion
};

#endif  // ENTITY_H