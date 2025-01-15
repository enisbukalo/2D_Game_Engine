#ifndef COMPONENT_H
#define COMPONENT_H

#include <nlohmann/json.hpp>
#include <string>
#include "Vec2.h"

using json = nlohmann::json;

class Entity;  // Forward declaration

class Component
{
public:
    Component()          = default;
    virtual ~Component() = default;

    virtual void init() {}
    virtual void update(float deltaTime) {}
    virtual json serialize() const
    {
        return json{{"type", getType()}};
    }
    virtual void        deserialize(const json& data) {}
    virtual std::string getType() const = 0;

    Entity* owner = nullptr;

private:
    bool m_active = true;

public:
    bool isActive() const
    {
        return m_active;
    }
    void setActive(bool active)
    {
        m_active = active;
    }
};

// Basic components
struct CTransform : public Component
{
    Vec2  position = Vec2(0.0f, 0.0f);
    Vec2  velocity = Vec2(0.0f, 0.0f);
    Vec2  scale    = Vec2(1.0f, 1.0f);
    float rotation = 0.0f;

    void update(float deltaTime) override
    {
        position += velocity * deltaTime;
    }

    std::string getType() const override
    {
        return "Transform";
    }

    json serialize() const override
    {
        json j        = Component::serialize();
        j["position"] = {{"x", position.x}, {"y", position.y}};
        j["velocity"] = {{"x", velocity.x}, {"y", velocity.y}};
        j["scale"]    = {{"x", scale.x}, {"y", scale.y}};
        j["rotation"] = rotation;
        return j;
    }

    void deserialize(const json& data) override
    {
        if (data.contains("position"))
        {
            position.x = data["position"]["x"];
            position.y = data["position"]["y"];
        }
        if (data.contains("velocity"))
        {
            velocity.x = data["velocity"]["x"];
            velocity.y = data["velocity"]["y"];
        }
        if (data.contains("scale"))
        {
            scale.x = data["scale"]["x"];
            scale.y = data["scale"]["y"];
        }
        if (data.contains("rotation"))
        {
            rotation = data["rotation"];
        }
    }
};

struct CName : public Component
{
    std::string name;

    explicit CName(const std::string& n = "") : name(n) {}

    std::string getType() const override
    {
        return "Name";
    }

    json serialize() const override
    {
        json j    = Component::serialize();
        j["name"] = name;
        return j;
    }

    void deserialize(const json& data) override
    {
        if (data.contains("name"))
        {
            name = data["name"];
        }
    }
};

struct CGravity : public Component
{
    Vec2 force = Vec2(0.0f, -9.81f);

    void update(float deltaTime) override;

    std::string getType() const override
    {
        return "Gravity";
    }

    json serialize() const override
    {
        json j     = Component::serialize();
        j["force"] = {{"x", force.x}, {"y", force.y}};
        return j;
    }

    void deserialize(const json& data) override
    {
        if (data.contains("force"))
        {
            force.x = data["force"]["x"];
            force.y = data["force"]["y"];
        }
    }
};

#endif  // COMPONENT_H