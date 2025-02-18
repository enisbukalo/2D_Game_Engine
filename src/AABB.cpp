#include "physics/Quadtree.h"

AABB::AABB(const Vec2& pos, const Vec2& size) : position(pos), halfSize(size) {}

bool AABB::contains(const Vec2& point) const
{
    return (point.x >= position.x - halfSize.x && point.x <= position.x + halfSize.x
            && point.y >= position.y - halfSize.y && point.y <= position.y + halfSize.y);
}

bool AABB::intersects(const AABB& other) const
{
    return !(position.x + halfSize.x < other.position.x - other.halfSize.x
             || position.x - halfSize.x > other.position.x + other.halfSize.x
             || position.y + halfSize.y < other.position.y - other.halfSize.y
             || position.y - halfSize.y > other.position.y + other.halfSize.y);
}