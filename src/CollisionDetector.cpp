#include "physics/CollisionDetector.h"
#include "components/CCollider.h"
#include "components/CCircleCollider.h"
#include "components/CBoxCollider.h"
#include "components/CTransform.h"
#include <algorithm>
#include <cmath>

bool CollisionDetector::intersects(const CCollider* a, const CCollider* b)
{
    if (!a || !b)
        return false;

    // Try to cast to specific collider types
    auto* circleA = dynamic_cast<const CCircleCollider*>(a);
    auto* circleB = dynamic_cast<const CCircleCollider*>(b);
    auto* boxA = dynamic_cast<const CBoxCollider*>(a);
    auto* boxB = dynamic_cast<const CBoxCollider*>(b);

    // Circle vs Circle
    if (circleA && circleB)
        return circleVsCircle(circleA, circleB);

    // Circle vs Box (order doesn't matter)
    if (circleA && boxB)
        return circleVsBox(circleA, boxB);
    if (boxA && circleB)
        return circleVsBox(circleB, boxA);

    // Box vs Box
    if (boxA && boxB)
        return boxVsBox(boxA, boxB);

    // Unsupported collision type
    return false;
}

bool CollisionDetector::circleVsCircle(const CCircleCollider* a, const CCircleCollider* b)
{
    Vec2 posA = a->getOwner()->getComponent<CTransform>()->getPosition();
    Vec2 posB = b->getOwner()->getComponent<CTransform>()->getPosition();

    float radiusSum = a->getRadius() + b->getRadius();
    float distSq = posA.distanceSquared(posB);

    return distSq <= (radiusSum * radiusSum);
}

bool CollisionDetector::circleVsBox(const CCircleCollider* circle, const CBoxCollider* box)
{
    Vec2 circlePos = circle->getOwner()->getComponent<CTransform>()->getPosition();
    Vec2 boxPos = box->getOwner()->getComponent<CTransform>()->getPosition();

    Vec2 boxSize = box->getSize();
    Vec2 halfSize = boxSize * 0.5f;

    // Find the closest point on the box to the circle center
    float closestX = std::max(boxPos.x - halfSize.x, std::min(circlePos.x, boxPos.x + halfSize.x));
    float closestY = std::max(boxPos.y - halfSize.y, std::min(circlePos.y, boxPos.y + halfSize.y));

    // Calculate distance between circle center and closest point
    float distX = circlePos.x - closestX;
    float distY = circlePos.y - closestY;
    float distSq = distX * distX + distY * distY;

    // Check if distance is less than circle radius
    float radius = circle->getRadius();
    return distSq <= (radius * radius);
}

bool CollisionDetector::boxVsBox(const CBoxCollider* a, const CBoxCollider* b)
{
    Vec2 posA = a->getOwner()->getComponent<CTransform>()->getPosition();
    Vec2 posB = b->getOwner()->getComponent<CTransform>()->getPosition();

    Vec2 halfSizeA = a->getSize() * 0.5f;
    Vec2 halfSizeB = b->getSize() * 0.5f;

    // AABB collision detection
    bool overlapX = std::abs(posA.x - posB.x) <= (halfSizeA.x + halfSizeB.x);
    bool overlapY = std::abs(posA.y - posB.y) <= (halfSizeA.y + halfSizeB.y);

    return overlapX && overlapY;
}

