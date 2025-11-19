#include <cmath>  // for std::abs
#include "physics/Quadtree.h"

AABB::AABB(const Vec2& pos, const Vec2& size) : position(pos), halfSize(Vec2(std::abs(size.x) * 0.5f, std::abs(size.y) * 0.5f)) {}

bool AABB::contains(const Vec2& point) const
{
    return (point.x >= position.x - std::abs(halfSize.x) && point.x <= position.x + std::abs(halfSize.x)
            && point.y >= position.y - std::abs(halfSize.y) && point.y <= position.y + std::abs(halfSize.y));
}

bool AABB::intersects(const AABB& other) const
{
    float thisHalfX  = std::abs(halfSize.x);
    float thisHalfY  = std::abs(halfSize.y);
    float otherHalfX = std::abs(other.halfSize.x);
    float otherHalfY = std::abs(other.halfSize.y);

    return !(position.x + thisHalfX < other.position.x - otherHalfX || position.x - thisHalfX > other.position.x + otherHalfX
             || position.y + thisHalfY < other.position.y - otherHalfY || position.y - thisHalfY > other.position.y + otherHalfY);
}