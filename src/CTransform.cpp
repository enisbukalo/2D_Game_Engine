#include "CTransform.h"
#include "JsonBuilder.h"
#include "JsonParser.h"

void CTransform::update(float deltaTime)
{
    m_position += m_velocity * deltaTime;
}

std::string CTransform::getType() const
{
    return "Transform";
}

void CTransform::serialize(JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cTransform");
    builder.beginObject();
    builder.addKey("position");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(m_position.x);
    builder.addKey("y");
    builder.addNumber(m_position.y);
    builder.endObject();
    builder.addKey("velocity");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(m_velocity.x);
    builder.addKey("y");
    builder.addNumber(m_velocity.y);
    builder.endObject();
    builder.addKey("scale");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(m_scale.x);
    builder.addKey("y");
    builder.addNumber(m_scale.y);
    builder.endObject();
    builder.addKey("rotation");
    builder.addNumber(m_rotation);
    builder.endObject();
    builder.endObject();
}

void CTransform::deserialize(const JsonValue& value) {}

Vec2 CTransform::getPosition() const
{
    return m_position;
}

void CTransform::setPosition(const Vec2& position)
{
    m_position = position;
}

Vec2 CTransform::getVelocity() const
{
    return m_velocity;
}

void CTransform::setVelocity(const Vec2& velocity)
{
    m_velocity = velocity;
}

Vec2 CTransform::getScale() const
{
    return m_scale;
}

void CTransform::setScale(const Vec2& scale)
{
    m_scale = scale;
}

float CTransform::getRotation() const
{
    return m_rotation;
}

void CTransform::setRotation(float rotation)
{
    m_rotation = rotation;
}
