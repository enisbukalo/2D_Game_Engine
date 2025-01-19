#include "CTransform.h"

void CTransform::update(float deltaTime)
{
    position += velocity * deltaTime;
}

std::string CTransform::getType() const
{
    return "Transform";
}

json CTransform::serialize() const
{
    json j        = Component::serialize();
    j["position"] = {{"x", position.x}, {"y", position.y}};
    j["velocity"] = {{"x", velocity.x}, {"y", velocity.y}};
    j["scale"]    = {{"x", scale.x}, {"y", scale.y}};
    j["rotation"] = rotation;
    return j;
}

void CTransform::deserialize(const json& data)
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