#include "CGravity.h"

void CGravity::update(float deltaTime)
{
    // Implementation will be added when physics system is implemented
}

std::string CGravity::getType() const
{
    return "Gravity";
}

void CGravity::serialize() const {}

void CGravity::deserialize() {}

Vec2 CGravity::getForce() const
{
    return m_force;
}

void CGravity::setForce(const Vec2& force)
{
    m_force = force;
}
