#include "CTransform.h"

namespace Components
{
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
