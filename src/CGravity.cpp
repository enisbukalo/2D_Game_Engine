#include "CGravity.h"

void CGravity::update(float deltaTime)
{
    // Implementation will be added when physics system is implemented
}

std::string CGravity::getType() const
{
    return "Gravity";
}

void CGravity::serialize(JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cGravity");
    builder.beginObject();
    builder.addKey("force");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(m_force.x);
    builder.addKey("y");
    builder.addNumber(m_force.y);
    builder.endObject();
    builder.endObject();
    builder.endObject();
}

void CGravity::deserialize(const JsonValue& value) {}

Vec2 CGravity::getForce() const
{
    return m_force;
}

void CGravity::setForce(const Vec2& force)
{
    m_force = force;
}
