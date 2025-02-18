#include "components/CCircleCollider.h"
#include "components/CTransform.h"

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
    // For now, only handle circle-circle collisions
    if (auto* circle = dynamic_cast<const CCircleCollider*>(other))
    {
        return circleVsCircle(circle);
    }
    return false;
}

float CCircleCollider::getRadius() const
{
    return m_radius;
}

void CCircleCollider::setRadius(float radius)
{
    m_radius = radius;
}

// Private helper methods for collision detection
bool CCircleCollider::circleVsCircle(const CCircleCollider* other) const
{
    Vec2 pos1 = getOwner()->getComponent<CTransform>()->getPosition();
    Vec2 pos2 = other->getOwner()->getComponent<CTransform>()->getPosition();

    float radiusSum = m_radius + other->getRadius();
    float distSq    = pos1.distanceSquared(pos2);

    return distSq <= (radiusSum * radiusSum);
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