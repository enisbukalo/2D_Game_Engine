#include "physics/CollisionDetector.h"
#include <algorithm>
#include <cmath>
#include "components/CBoxCollider.h"
#include "components/CCircleCollider.h"
#include "components/CCollider.h"
#include "components/CTransform.h"

bool CollisionDetector::intersects(const CCollider* a, const CCollider* b)
{
    CollisionManifold manifold = getManifold(a, b);
    return manifold.hasCollision;
}

CollisionManifold CollisionDetector::getManifold(const CCollider* a, const CCollider* b)
{
    if (!a || !b)
        return CollisionManifold();

    // Try to cast to specific collider types
    auto* circleA = dynamic_cast<const CCircleCollider*>(a);
    auto* circleB = dynamic_cast<const CCircleCollider*>(b);
    auto* boxA    = dynamic_cast<const CBoxCollider*>(a);
    auto* boxB    = dynamic_cast<const CBoxCollider*>(b);

    // Circle vs Circle
    if (circleA && circleB)
        return circleVsCircle(circleA, circleB);

    // Circle vs Box (order doesn't matter)
    if (circleA && boxB)
        return circleVsBox(circleA, boxB);
    if (boxA && circleB) {
        // Flip the normal when order is reversed
        CollisionManifold manifold = circleVsBox(circleB, boxA);
        manifold.normal = manifold.normal * -1.0f;
        return manifold;
    }

    // Box vs Box
    if (boxA && boxB)
        return boxVsBox(boxA, boxB);

    // Unsupported collision type
    return CollisionManifold();
}

CollisionManifold CollisionDetector::circleVsCircle(const CCircleCollider* a, const CCircleCollider* b)
{
    Vec2 posA = a->getOwner()->getComponent<CTransform>()->getPosition();
    Vec2 posB = b->getOwner()->getComponent<CTransform>()->getPosition();

    float radiusA = a->getRadius();
    float radiusB = b->getRadius();
    float radiusSum = radiusA + radiusB;

    // Calculate distance between centers
    Vec2 delta = posB - posA;
    float distSq = delta.lengthSquared();
    float radiusSumSq = radiusSum * radiusSum;

    // No collision if circles are too far apart
    if (distSq > radiusSumSq)
        return CollisionManifold();

    float distance = std::sqrt(distSq);

    // Handle perfectly overlapping circles
    if (distance < 0.0001f) {
        // Arbitrary separation direction
        Vec2 normal(1.0f, 0.0f);
        float penetration = radiusA; // Arbitrary, both circles overlap completely

        // Contact point in the middle
        Vec2 contactPoint = posA;
        return CollisionManifold(true, normal, penetration, {contactPoint});
    }

    // Calculate collision normal (from A to B)
    Vec2 normal = delta / distance;

    // Penetration depth
    float penetration = radiusSum - distance;

    // Contact point is on the edge of circle A (surface point)
    // This is the point where circle A's surface touches circle B
    Vec2 contactPoint = posA + normal * radiusA;

    return CollisionManifold(true, normal, penetration, {contactPoint});
}

CollisionManifold CollisionDetector::circleVsBox(const CCircleCollider* circle, const CBoxCollider* box)
{
    Vec2 circlePos = circle->getOwner()->getComponent<CTransform>()->getPosition();
    Vec2 boxPos    = box->getOwner()->getComponent<CTransform>()->getPosition();

    Vec2 boxSize  = box->getSize();
    Vec2 halfSize = boxSize * 0.5f;

    // Find the closest point on the box edge to the circle center
    float closestX = std::max(boxPos.x - halfSize.x, std::min(circlePos.x, boxPos.x + halfSize.x));
    float closestY = std::max(boxPos.y - halfSize.y, std::min(circlePos.y, boxPos.y + halfSize.y));
    Vec2 closestPoint(closestX, closestY);

    // Calculate vector from closest point on box edge to circle center
    Vec2 delta = circlePos - closestPoint;
    float distSq = delta.lengthSquared();

    float radius = circle->getRadius();
    float radiusSq = radius * radius;

    // No collision if circle is too far from box edge
    if (distSq > radiusSq)
        return CollisionManifold();

    float distance = std::sqrt(distSq);

    // Handle case where circle center is inside the box
    if (distance < 0.0001f) {
        // Circle center is inside box, find shortest distance to edge
        float distToLeft   = circlePos.x - (boxPos.x - halfSize.x);
        float distToRight  = (boxPos.x + halfSize.x) - circlePos.x;
        float distToBottom = circlePos.y - (boxPos.y - halfSize.y);
        float distToTop    = (boxPos.y + halfSize.y) - circlePos.y;

        float minDist = std::min({distToLeft, distToRight, distToBottom, distToTop});

        Vec2 normal;
        if (minDist == distToLeft)
            normal = Vec2(-1.0f, 0.0f);  // Push circle left (circle->box direction)
        else if (minDist == distToRight)
            normal = Vec2(1.0f, 0.0f);   // Push circle right
        else if (minDist == distToBottom)
            normal = Vec2(0.0f, -1.0f);  // Push circle down
        else
            normal = Vec2(0.0f, 1.0f);   // Push circle up

        float penetration = radius + minDist;
        Vec2 contactPoint = circlePos + normal * radius; // Point on circle edge in push direction

        return CollisionManifold(true, normal, penetration, {contactPoint});
    }

    // Normal points from circle toward box (away from circle center toward closest box point)
    Vec2 normal = delta / distance * -1.0f;

    // Penetration depth
    float penetration = radius - distance;

    // Contact point on circle edge (surface) - pointing toward the box
    Vec2 contactPointOnCircle = circlePos + normal * radius;

    return CollisionManifold(true, normal, penetration, {contactPointOnCircle});
}

CollisionManifold CollisionDetector::boxVsBox(const CBoxCollider* a, const CBoxCollider* b)
{
    Vec2 posA = a->getOwner()->getComponent<CTransform>()->getPosition();
    Vec2 posB = b->getOwner()->getComponent<CTransform>()->getPosition();

    Vec2 halfSizeA = a->getSize() * 0.5f;
    Vec2 halfSizeB = b->getSize() * 0.5f;

    // Calculate overlap on each axis
    Vec2 delta = posB - posA;
    float overlapX = (halfSizeA.x + halfSizeB.x) - std::abs(delta.x);
    float overlapY = (halfSizeA.y + halfSizeB.y) - std::abs(delta.y);

    // No collision if no overlap on either axis (touching counts as collision)
    if (overlapX < 0.0f || overlapY < 0.0f)
        return CollisionManifold();

    // Find the axis of least penetration (this is the collision edge direction)
    Vec2 normal;
    float penetration;
    Vec2 contactPoint;

    if (overlapX < overlapY) {
        // Collision on the X axis (left or right edge)
        penetration = overlapX;
        normal = (delta.x > 0.0f) ? Vec2(1.0f, 0.0f) : Vec2(-1.0f, 0.0f);

        // Contact point is on the edge of box A
        float contactX = posA.x + (normal.x > 0.0f ? halfSizeA.x : -halfSizeA.x);

        // Y coordinate is clamped to the overlapping region
        float minY = std::max(posA.y - halfSizeA.y, posB.y - halfSizeB.y);
        float maxY = std::min(posA.y + halfSizeA.y, posB.y + halfSizeB.y);
        float contactY = (minY + maxY) * 0.5f; // Midpoint of overlap

        contactPoint = Vec2(contactX, contactY);
    } else {
        // Collision on the Y axis (top or bottom edge)
        penetration = overlapY;
        normal = (delta.y > 0.0f) ? Vec2(0.0f, 1.0f) : Vec2(0.0f, -1.0f);

        // Contact point is on the edge of box A
        float contactY = posA.y + (normal.y > 0.0f ? halfSizeA.y : -halfSizeA.y);

        // X coordinate is clamped to the overlapping region
        float minX = std::max(posA.x - halfSizeA.x, posB.x - halfSizeB.x);
        float maxX = std::min(posA.x + halfSizeA.x, posB.x + halfSizeB.x);
        float contactX = (minX + maxX) * 0.5f; // Midpoint of overlap

        contactPoint = Vec2(contactX, contactY);
    }

    return CollisionManifold(true, normal, penetration, {contactPoint});
}
