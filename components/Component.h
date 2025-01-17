#ifndef COMPONENT_H
#define COMPONENT_H

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class Entity;  // Forward declaration

class Component
{
public:
    Component()          = default;
    virtual ~Component() = default;

    virtual void        init() {}
    virtual void        update(float deltaTime) {}
    virtual json        serialize() const;
    virtual void        deserialize(const json& data) {}
    virtual std::string getType() const = 0;

    Entity* owner = nullptr;

private:
    bool m_active = true;

public:
    bool isActive() const;
    void setActive(bool active);
};

#endif  // COMPONENT_H