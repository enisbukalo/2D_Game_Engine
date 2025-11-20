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
    builder.addKey("multiplier");
    builder.addNumber(m_multiplier);
    builder.endObject();
    builder.endObject();
}

void CGravity::deserialize(const JsonValue& value)
{
    const auto& gravity = value["cGravity"];
    m_multiplier        = static_cast<float>(gravity["multiplier"].getNumber());
}

float CGravity::getMultiplier() const
{
    return m_multiplier;
}

void CGravity::setMultiplier(float multiplier)
{
    m_multiplier = multiplier;
}
