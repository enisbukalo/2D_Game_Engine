#include "CTransform.h"
#include "SSerialization.h"

namespace Components
{

std::string CTransform::getType() const
{
    return "Transform";
}

void CTransform::serialize(Serialization::JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cTransform");
    builder.beginObject();
    builder.addKey("position");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(position.x);
    builder.addKey("y");
    builder.addNumber(position.y);
    builder.endObject();
    builder.addKey("velocity");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(velocity.x);
    builder.addKey("y");
    builder.addNumber(velocity.y);
    builder.endObject();
    builder.addKey("scale");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(scale.x);
    builder.addKey("y");
    builder.addNumber(scale.y);
    builder.endObject();
    builder.addKey("rotation");
    builder.addNumber(rotation);
    builder.endObject();
    builder.endObject();
}

void CTransform::deserialize(const Serialization::SSerialization::JsonValue& value)
{
    const auto& transform = value["cTransform"];
    const auto& pos = transform["position"];
    position.x      = pos["x"].getNumber();
    position.y      = pos["y"].getNumber();

    const auto& vel = transform["velocity"];
    velocity.x      = vel["x"].getNumber();
    velocity.y      = vel["y"].getNumber();

    const auto& scale = transform["scale"];
    this->scale.x      = scale["x"].getNumber();
    this->scale.y      = scale["y"].getNumber();

    rotation = transform["rotation"].getNumber();
}

Vec2 CTransform::getPosition() const
{
    return position;
}

void CTransform::setPosition(const Vec2& position)
{
    this->position = position;
}

Vec2 CTransform::getVelocity() const
{
    return velocity;
}

void CTransform::setVelocity(const Vec2& velocity)
{
    this->velocity = velocity;
}

Vec2 CTransform::getScale() const
{
    return scale;
}

void CTransform::setScale(const Vec2& scale)
{
    this->scale = scale;
}

float CTransform::getRotation() const
{
    return rotation;
}

void CTransform::setRotation(float rotation)
{
    this->rotation = rotation;
}

}  // namespace Components
