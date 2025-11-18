#ifndef COLLISIONDETECTOR_H
#define COLLISIONDETECTOR_H

#include "Vec2.h"
#include <vector>

class CCollider;
class CCircleCollider;
class CBoxCollider;

/**
 * @brief Collision manifold containing detailed collision information
 *
 * Stores contact points on the edges/surfaces of colliding shapes,
 * collision normal, and penetration depth for physics resolution.
 */
struct CollisionManifold
{
    bool hasCollision = false;          // Whether a collision occurred
    Vec2 normal;                         // Collision normal (points from A to B)
    float penetrationDepth = 0.0f;       // How deep the shapes overlap
    std::vector<Vec2> contactPoints;     // Contact points on shape edges (1-2 points)

    CollisionManifold() = default;

    CollisionManifold(bool collision, const Vec2& norm, float depth, const std::vector<Vec2>& points)
        : hasCollision(collision), normal(norm), penetrationDepth(depth), contactPoints(points)
    {}
};

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
     * @brief Main collision detection entry point (legacy boolean version)
     * @param a First collider
     * @param b Second collider
     * @return true if colliders are intersecting
     */
    static bool intersects(const CCollider* a, const CCollider* b);

    /**
     * @brief Main collision detection with manifold
     * @param a First collider
     * @param b Second collider
     * @return CollisionManifold with contact points and collision details
     */
    static CollisionManifold getManifold(const CCollider* a, const CCollider* b);

    /**
     * @brief Circle vs Circle collision detection with edge contact points
     * @return CollisionManifold with contact point on circle surfaces
     */
    static CollisionManifold circleVsCircle(const CCircleCollider* a, const CCircleCollider* b);

    /**
     * @brief Circle vs Box collision detection with edge contact points
     * @return CollisionManifold with contact points on circle and box edges
     */
    static CollisionManifold circleVsBox(const CCircleCollider* circle, const CBoxCollider* box);

    /**
     * @brief Box vs Box collision detection with edge contact points
     * @return CollisionManifold with contact points on box edges
     */
    static CollisionManifold boxVsBox(const CBoxCollider* a, const CBoxCollider* b);
};

#endif  // COLLISIONDETECTOR_H
