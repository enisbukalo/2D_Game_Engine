#include "CTransform.h"
#include "Entity.h"
#include "JsonBuilder.h"
#include "JsonParser.h"
#include <cmath>

std::string CTransform::getType() const
{
    return "Transform";
}

void CTransform::serialize(JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cTransform");
    builder.beginObject();
    // Write both world position and local position for compatibility
    // Write world position under "position"
    Vec2 worldPos = getWorldPosition();
    builder.addKey("position");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(worldPos.x);
    builder.addKey("y");
    builder.addNumber(worldPos.y);
    builder.endObject();

    // Also write local position under "localPosition"
    builder.addKey("localPosition");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(m_localPosition.x);
    builder.addKey("y");
    builder.addNumber(m_localPosition.y);
    builder.endObject();
    builder.addKey("velocity");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(m_velocity.x);
    builder.addKey("y");
    builder.addNumber(m_velocity.y);
    builder.endObject();
    // Write world scale under "scale"
    Vec2 worldScale = getWorldScale();
    builder.addKey("scale");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(worldScale.x);
    builder.addKey("y");
    builder.addNumber(worldScale.y);
    builder.endObject();

    // Also write local scale under "localScale"
    builder.addKey("localScale");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(m_localScale.x);
    builder.addKey("y");
    builder.addNumber(m_localScale.y);
    builder.endObject();
    // Write world rotation under "rotation"
    builder.addKey("rotation");
    builder.addNumber(getWorldRotation());

    // Also write local rotation under "localRotation"
    builder.addKey("localRotation");
    builder.addNumber(m_localRotation);
    builder.endObject();
    builder.endObject();
}

void CTransform::deserialize(const JsonValue& value)
{
    const auto& transform = value["cTransform"];

    // Try to load local transforms first (new format)
    if (transform.hasKey("localPosition"))
    {
        const auto& pos = transform["localPosition"];
        m_localPosition.x = pos["x"].getNumber();
        m_localPosition.y = pos["y"].getNumber();
    }
    // Fall back to old position format for backward compatibility
    else if (transform.hasKey("position"))
    {
        const auto& pos = transform["position"];
        // Position is provided as world position; convert to local position
        setWorldPosition(Vec2(pos["x"].getNumber(), pos["y"].getNumber()));
    }

    // Deserialize velocity
    const auto& vel = transform["velocity"];
    m_velocity.x    = vel["x"].getNumber();
    m_velocity.y    = vel["y"].getNumber();

    // Try to load local scale first (new format)
    if (transform.hasKey("localScale"))
    {
        const auto& scale = transform["localScale"];
        m_localScale.x = scale["x"].getNumber();
        m_localScale.y = scale["y"].getNumber();
    }
    // Fall back to old scale format for backward compatibility
    else if (transform.hasKey("scale"))
    {
        const auto& scale = transform["scale"];
        // Scale provided as world scale; convert to local scale
        setWorldScale(Vec2(scale["x"].getNumber(), scale["y"].getNumber()));
    }

    // Try to load local rotation first (new format)
    if (transform.hasKey("localRotation"))
    {
        m_localRotation = transform["localRotation"].getNumber();
    }
    // Fall back to old rotation format for backward compatibility
    else if (transform.hasKey("rotation"))
    {
        // Rotation provided as world rotation; convert to local rotation
        setWorldRotation(transform["rotation"].getNumber());
    }
}

Vec2 CTransform::getPosition() const
{
    return getWorldPosition();
}

Vec2 CTransform::getLocalPosition() const
{
    return m_localPosition;
}

Vec2 CTransform::getWorldPosition() const
{
    if (!getOwner())
    {
        return m_localPosition;
    }

    auto parent = getOwner()->getParent();
    if (!parent)
    {
        return m_localPosition;
    }

    auto parentTransform = parent->getComponent<CTransform>();
    if (!parentTransform)
    {
        return m_localPosition;
    }

    // Compute world position from parent's world transform
    Vec2 parentWorldPos = parentTransform->getWorldPosition();
    float parentWorldRot = parentTransform->getWorldRotation();
    Vec2 parentWorldScale = parentTransform->getWorldScale();

    // Rotate and scale local position by parent's transform
    float cosRot = std::cos(parentWorldRot);
    float sinRot = std::sin(parentWorldRot);

    Vec2 scaledLocalPos = Vec2(m_localPosition.x * parentWorldScale.x, m_localPosition.y * parentWorldScale.y);
    Vec2 rotatedPos = Vec2(scaledLocalPos.x * cosRot - scaledLocalPos.y * sinRot,
                          scaledLocalPos.x * sinRot + scaledLocalPos.y * cosRot);

    return Vec2(parentWorldPos.x + rotatedPos.x, parentWorldPos.y + rotatedPos.y);
}

void CTransform::setPosition(const Vec2& position)
{
    setWorldPosition(position);
}

void CTransform::setLocalPosition(const Vec2& pos)
{
    m_localPosition = pos;
}

void CTransform::setWorldPosition(const Vec2& pos)
{
    if (!getOwner())
    {
        m_localPosition = pos;
        return;
    }

    auto parent = getOwner()->getParent();
    if (!parent)
    {
        m_localPosition = pos;
        return;
    }

    auto parentTransform = parent->getComponent<CTransform>();
    if (!parentTransform)
    {
        m_localPosition = pos;
        return;
    }

    // Convert world position to local position
    Vec2 parentWorldPos = parentTransform->getWorldPosition();
    float parentWorldRot = parentTransform->getWorldRotation();
    Vec2 parentWorldScale = parentTransform->getWorldScale();

    // Inverse transform: translate, rotate, scale
    Vec2 relativePos = Vec2(pos.x - parentWorldPos.x, pos.y - parentWorldPos.y);

    float cosRot = std::cos(-parentWorldRot);
    float sinRot = std::sin(-parentWorldRot);
    Vec2 rotatedPos = Vec2(relativePos.x * cosRot - relativePos.y * sinRot,
                          relativePos.x * sinRot + relativePos.y * cosRot);

    m_localPosition = Vec2(rotatedPos.x / parentWorldScale.x, rotatedPos.y / parentWorldScale.y);
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
    return getWorldScale();
}

Vec2 CTransform::getLocalScale() const
{
    return m_localScale;
}

Vec2 CTransform::getWorldScale() const
{
    if (!getOwner())
    {
        return m_localScale;
    }

    auto parent = getOwner()->getParent();
    if (!parent)
    {
        return m_localScale;
    }

    auto parentTransform = parent->getComponent<CTransform>();
    if (!parentTransform)
    {
        return m_localScale;
    }

    Vec2 parentWorldScale = parentTransform->getWorldScale();
    return Vec2(m_localScale.x * parentWorldScale.x, m_localScale.y * parentWorldScale.y);
}

void CTransform::setScale(const Vec2& scale)
{
    setWorldScale(scale);
}

void CTransform::setLocalScale(const Vec2& scl)
{
    m_localScale = scl;
}

void CTransform::setWorldScale(const Vec2& scl)
{
    if (!getOwner())
    {
        m_localScale = scl;
        return;
    }

    auto parent = getOwner()->getParent();
    if (!parent)
    {
        m_localScale = scl;
        return;
    }

    auto parentTransform = parent->getComponent<CTransform>();
    if (!parentTransform)
    {
        m_localScale = scl;
        return;
    }

    Vec2 parentWorldScale = parentTransform->getWorldScale();
    m_localScale = Vec2(scl.x / parentWorldScale.x, scl.y / parentWorldScale.y);
}

float CTransform::getRotation() const
{
    return getWorldRotation();
}

float CTransform::getLocalRotation() const
{
    return m_localRotation;
}

float CTransform::getWorldRotation() const
{
    if (!getOwner())
    {
        return m_localRotation;
    }

    auto parent = getOwner()->getParent();
    if (!parent)
    {
        return m_localRotation;
    }

    auto parentTransform = parent->getComponent<CTransform>();
    if (!parentTransform)
    {
        return m_localRotation;
    }

    return parentTransform->getWorldRotation() + m_localRotation;
}

void CTransform::setRotation(float rotation)
{
    setWorldRotation(rotation);
}

void CTransform::setLocalRotation(float rot)
{
    m_localRotation = rot;
}

void CTransform::setWorldRotation(float rot)
{
    if (!getOwner())
    {
        m_localRotation = rot;
        return;
    }

    auto parent = getOwner()->getParent();
    if (!parent)
    {
        m_localRotation = rot;
        return;
    }

    auto parentTransform = parent->getComponent<CTransform>();
    if (!parentTransform)
    {
        m_localRotation = rot;
        return;
    }

    m_localRotation = rot - parentTransform->getWorldRotation();
}
