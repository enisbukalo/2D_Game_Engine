#ifndef CTRANSFORM_H
#define CTRANSFORM_H

#include "Component.h"
#include "Vec2.h"

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

#endif  // CTRANSFORM_H