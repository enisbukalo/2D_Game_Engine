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

void CTransform::serialize() const {}

void CTransform::deserialize() {}

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
