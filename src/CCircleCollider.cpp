#include "components/CCircleCollider.h"
#include "components/CTransform.h"
#include "physics/CollisionDetector.h"

CCircleCollider::CCircleCollider(float radius) : m_radius(radius)
{
    m_isTrigger = false;
}

AABB CCircleCollider::getBounds() const
{
    auto pos = getOwner()->getComponent<CTransform>()->getPosition();
    return AABB(pos, Vec2(m_radius * 2, m_radius * 2));
}

bool CCircleCollider::intersects(const CCollider* other) const
{
    // Delegate to centralized collision detector
    return CollisionDetector::intersects(this, other);
}

float CCircleCollider::getRadius() const
{
    return m_radius;
}

void CCircleCollider::setRadius(float radius)
{
    m_radius = radius;
}

void CCircleCollider::serialize(JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cCircleCollider");
    builder.beginObject();
    builder.addKey("radius");
    builder.addNumber(m_radius);
    builder.addKey("trigger");
    builder.addBool(m_isTrigger);
    builder.endObject();
    builder.endObject();
}

void CCircleCollider::deserialize(const JsonValue& value)
{
    const auto& circleComp = value["cCircleCollider"];
    m_radius               = circleComp["radius"].getNumber();
    m_isTrigger            = circleComp["trigger"].getBool();
}

std::string CCircleCollider::getType() const
{
    return "CircleCollider";
}
