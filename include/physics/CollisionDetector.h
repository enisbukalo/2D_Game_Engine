#ifndef COLLISIONDETECTOR_H
#define COLLISIONDETECTOR_H

#include "Vec2.h"

class CCollider;
class CCircleCollider;
class CBoxCollider;

/**
 * @brief Centralized collision detection system
 *
 * This class provides static methods for detecting collisions between different
 * collider types. Centralizing collision logic here prevents code duplication
 * and makes it easier to maintain and extend collision detection.
 */
class CollisionDetector
{
public:
    /**
     * @brief Main collision detection entry point
     * @param a First collider
     * @param b Second collider
     * @return true if colliders are intersecting
     */
    static bool intersects(const CCollider* a, const CCollider* b);

    /**
     * @brief Circle vs Circle collision detection
     */
    static bool circleVsCircle(const CCircleCollider* a, const CCircleCollider* b);

    /**
     * @brief Circle vs Box collision detection
     */
    static bool circleVsBox(const CCircleCollider* circle, const CBoxCollider* box);

    /**
     * @brief Box vs Box collision detection
     */
    static bool boxVsBox(const CBoxCollider* a, const CBoxCollider* b);
};

#endif // COLLISIONDETECTOR_H

