#include "components/CBoxCollider.h"
#include "components/CTransform.h"
#include "physics/CollisionDetector.h"

CBoxCollider::CBoxCollider(float width, float height) : m_size(width, height)
{
    m_isTrigger = false;
}

CBoxCollider::CBoxCollider(const Vec2& size) : m_size(size)
{
    m_isTrigger = false;
}

AABB CBoxCollider::getBounds() const
{
    auto pos = getOwner()->getComponent<CTransform>()->getPosition();
    // AABB expects the full size (will be stored as halfSize internally)
    return AABB(pos, m_size);
}

bool CBoxCollider::intersects(const CCollider* other) const
{
    // Delegate to centralized collision detector
    return CollisionDetector::intersects(this, other);
}

float CBoxCollider::getWidth() const
{
    return m_size.x;
}

float CBoxCollider::getHeight() const
{
    return m_size.y;
}

Vec2 CBoxCollider::getSize() const
{
    return m_size;
}

void CBoxCollider::setSize(float width, float height)
{
    m_size = Vec2(width, height);
}

void CBoxCollider::setSize(const Vec2& size)
{
    m_size = size;
}

void CBoxCollider::serialize(JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cBoxCollider");
    builder.beginObject();
    builder.addKey("width");
    builder.addNumber(m_size.x);
    builder.addKey("height");
    builder.addNumber(m_size.y);
    builder.addKey("trigger");
    builder.addBool(m_isTrigger);
    builder.endObject();
    builder.endObject();
}

void CBoxCollider::deserialize(const JsonValue& value)
{
    const auto& boxComp = value["cBoxCollider"];
    m_size.x = boxComp["width"].getNumber();
    m_size.y = boxComp["height"].getNumber();
    m_isTrigger = boxComp["trigger"].getBool();
}

std::string CBoxCollider::getType() const
{
    return "BoxCollider";
}

