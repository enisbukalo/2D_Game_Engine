#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include "Vec2.h"

class Entity;  // Forward declaration

class Component
{
public:
    Component()          = default;
    virtual ~Component() = default;

    virtual void init() {}
    virtual void update(float deltaTime) {}

    Entity *owner = nullptr;

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
};

struct CName : public Component
{
    std::string name;

    explicit CName(const std::string &n = "") : name(n) {}
};

struct CGravity : public Component
{
    Vec2 force = Vec2(0.0f, -9.81f);
    void update(float deltaTime) override;
};

#endif  // COMPONENT_H