#include "systems/S2DPhysics.h"
#include <algorithm>
#include <iostream>
#include "EntityManager.h"
#include "components/CBoxCollider.h"
#include "components/CCircleCollider.h"
#include "components/CCollider.h"
#include "components/CGravity.h"
#include "components/CTransform.h"

S2DPhysics& S2DPhysics::instance()
{
    static S2DPhysics instance;
    return instance;
}

void S2DPhysics::update(float deltaTime)
{
    handleGravity(deltaTime);
    integratePositions(deltaTime);
    updateQuadtree();
    checkCollisions();

#ifdef DEBUG
    // Debug: print all entity velocities after physics update
    std::cout << "[DEBUG] S2DPhysics::update complete. Entity velocities:" << std::endl;
    auto& entityManager = EntityManager::instance();
    auto  entities      = entityManager.getEntitiesWithComponent<CTransform>();
    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        Vec2 vel       = transform->getVelocity();
        std::cout << "[DEBUG]   " << entity->getTag() << ": velocity=(" << vel.x << "," << vel.y << ")" << std::endl;
    }
#endif
}

S2DPhysics::S2DPhysics() : m_worldBounds(Vec2(0, 0), Vec2(1000, 1000))  // Default world size
{
    m_quadtree = std::make_unique<Quadtree>(0, m_worldBounds);
}

void S2DPhysics::setWorldBounds(const Vec2& center, const Vec2& size)
{
    m_worldBounds = AABB(center, size * 0.5f);
    m_quadtree    = std::make_unique<Quadtree>(0, m_worldBounds);
}

void S2DPhysics::updateQuadtree()
{
    // TODO: Implement Way For Reference To Move With Camera. (Options)
    //      - Move Grid (Spatial Hashing).
    //      - Multiple Grids (Chunking).
    //      - Infinite Grid (Relative Coordinates).

    m_quadtree->clear();

    auto& entityManager = EntityManager::instance();
    auto  entities      = entityManager.getEntitiesWithComponent<CTransform>();

#ifdef DEBUG
    std::cout << "[DEBUG] updateQuadtree: Found " << entities.size() << " entities with transform" << std::endl;
    std::cout << "[DEBUG] World bounds: center(" << m_worldBounds.position.x << ", " << m_worldBounds.position.y << ")"
              << " halfSize(" << m_worldBounds.halfSize.x << ", " << m_worldBounds.halfSize.y << ")" << std::endl;
#endif

    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        Vec2 pos       = transform->getPosition();

#ifdef DEBUG
        std::cout << "[DEBUG]   Entity " << entity->getTag() << " at (" << pos.x << ", " << pos.y << ")";
#endif

        // Only insert entities that are within world bounds
        if (m_worldBounds.contains(pos))
        {
#ifdef DEBUG
            std::cout << " - inserting into quadtree" << std::endl;
#endif
            m_quadtree->insert(entity);
        }
        else
        {
#ifdef DEBUG
            std::cout << " - OUT OF BOUNDS, clamping..." << std::endl;
#endif
            // TODO: Implement Out Of Bounds Handling. (Options)
            //      - Wrap around to the other side
            //      - Bounce off the boundaries
            //      - Destroy the entity
            //      - Teleport to nearest valid position

            Vec2 newPos = pos;
            newPos.x    = std::clamp(pos.x,
                                  m_worldBounds.position.x - m_worldBounds.halfSize.x,
                                  m_worldBounds.position.x + m_worldBounds.halfSize.x);
            newPos.y    = std::clamp(pos.y,
                                  m_worldBounds.position.y - m_worldBounds.halfSize.y,
                                  m_worldBounds.position.y + m_worldBounds.halfSize.y);

            transform->setPosition(newPos);
            m_quadtree->insert(entity);
        }
    }
}

void S2DPhysics::integratePositions(float deltaTime)
{
    auto& entityManager = EntityManager::instance();
    auto  entities      = entityManager.getEntitiesWithComponent<CTransform>();

    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        if (!transform)
            continue;

        // Update position based on velocity: position += velocity * deltaTime
        Vec2 currentPos = transform->getPosition();
        Vec2 velocity   = transform->getVelocity();
        Vec2 newPos     = currentPos + (velocity * deltaTime);

        transform->setPosition(newPos);
    }
}

void S2DPhysics::handleGravity(float deltaTime)
{
    auto& entityManager = EntityManager::instance();

    // Apply gravity force to velocities
    auto entities = entityManager.getEntitiesWithComponent<CGravity>();
    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        auto gravity   = entity->getComponent<CGravity>();

        if (transform && gravity && gravity->isActive())
        {
            // Apply gravity to velocity: v = v0 + a*dt
            Vec2 currentVelocity = transform->getVelocity();
            Vec2 force           = gravity->getForce();
            Vec2 newVelocity     = currentVelocity + (force * deltaTime);

            transform->setVelocity(newVelocity);
        }
    }
}

void S2DPhysics::checkCollisions()
{
    auto& entityManager = EntityManager::instance();
    // Use getEntitiesWithComponentDerived to find all entities with CCollider or derived types
    auto entities = entityManager.getEntitiesWithComponentDerived<CCollider>();

#ifdef DEBUG
    std::cout << "[DEBUG] checkCollisions: Found " << entities.size() << " entities with colliders" << std::endl;
#endif

    // Broad phase: Use quadtree to get potential collisions
    for (auto* entity : entities)
    {
#ifdef DEBUG
        std::cout << "[DEBUG] Starting loop iteration for entity..." << std::endl;
#endif
        if (!entity || !entity->isAlive())
        {
#ifdef DEBUG
            std::cout << "[DEBUG]   Entity is null or not alive, skipping" << std::endl;
#endif
            continue;
        }

        auto collider = entity->getComponentDerived<CCollider>();
        if (!collider)
        {
#ifdef DEBUG
            std::cout << "[DEBUG]   Could not get collider component, skipping" << std::endl;
#endif
            continue;
        }

        auto transform = entity->getComponent<CTransform>();
        auto bounds    = collider->getBounds();

#ifdef DEBUG
        std::cout << "[DEBUG] Entity " << entity->getTag() << " at (" << transform->getPosition().x << ", "
                  << transform->getPosition().y << ")" << " bounds: center(" << bounds.position.x << ", "
                  << bounds.position.y << ")" << " halfSize(" << bounds.halfSize.x << ", " << bounds.halfSize.y << ")"
                  << std::endl;
#endif

        // Query quadtree for potential collisions
        std::vector<Entity*> potentialCollisions = m_quadtree->query(bounds);

#ifdef DEBUG
        std::cout << "[DEBUG]   Quadtree query for bounds center(" << bounds.position.x << "," << bounds.position.y
                  << ")" << " halfSize(" << bounds.halfSize.x << "," << bounds.halfSize.y << ")" << " returned "
                  << potentialCollisions.size() << " potential collisions" << std::endl;
#endif

        // Narrow phase: Detailed collision checks
        for (auto* other : potentialCollisions)
        {
#ifdef DEBUG
            std::cout << "[DEBUG]   Checking potential collision..." << std::endl;
#endif
            if (!other || !other->isAlive() || entity == other)
            {
#ifdef DEBUG
                std::cout << "[DEBUG]     Skipping (null=" << (!other) << " alive=" << (other && other->isAlive())
                          << " self=" << (other && entity == other) << ")" << std::endl;
#endif
                continue;
            }

            // Skip if we've already processed this pair (avoid duplicate processing)
            if (entity->getId() >= other->getId())
            {
#ifdef DEBUG
                std::cout << "[DEBUG]     Skipping (already processed this pair)" << std::endl;
#endif
                continue;
            }

            auto otherCollider = other->getComponentDerived<CCollider>();
            if (!otherCollider)
            {
#ifdef DEBUG
                std::cout << "[DEBUG]     No collider on other entity, skipping" << std::endl;
#endif
                continue;
            }

#ifdef DEBUG
            std::cout << "[DEBUG]   Checking collision with " << other->getTag() << std::endl;
#endif

            // Detailed collision check
            if (collider->intersects(otherCollider))
            {
#ifdef DEBUG
                std::cout << "[DEBUG]   *** COLLISION DETECTED between " << entity->getTag() << " and "
                          << other->getTag() << " ***" << std::endl;
                std::cout << "[DEBUG]   Calling handleCollision..." << std::endl;
#endif
                handleCollision(entity, other);
#ifdef DEBUG
                std::cout << "[DEBUG]   handleCollision returned successfully" << std::endl;
#endif
            }
        }
    }
}

void S2DPhysics::handleCollision(Entity* a, Entity* b)
{
#ifdef DEBUG
    std::cout << "[DEBUG] handleCollision: Getting colliders..." << std::endl;
#endif
    auto colliderA = a->getComponentDerived<CCollider>();
    auto colliderB = b->getComponentDerived<CCollider>();
#ifdef DEBUG
    std::cout << "[DEBUG] handleCollision: Got colliders (A=" << (colliderA != nullptr)
              << ", B=" << (colliderB != nullptr) << ")" << std::endl;
#endif

    // If either is a trigger, just notify
    if (colliderA->isTrigger() || colliderB->isTrigger())
    {
        // TODO: Emit collision event
        return;
    }

    // Otherwise, resolve the collision physically
    resolveCollision(a, b, colliderA, colliderB);
}

void S2DPhysics::resolveCollision(Entity* a, Entity* b, const CCollider* colliderA, const CCollider* colliderB)
{
#ifdef DEBUG
    std::cout << "[DEBUG] resolveCollision: START" << std::endl;
#endif
    // Get transforms (need non-const access to update velocities)
    auto transformA = a->getComponent<CTransform>();
    auto transformB = b->getComponent<CTransform>();
#ifdef DEBUG
    std::cout << "[DEBUG] resolveCollision: Got transforms (A=" << (transformA != nullptr)
              << ", B=" << (transformB != nullptr) << ")" << std::endl;
#endif

    if (!transformA || !transformB)
        return;

    // Get collision manifold with edge-based contact points
    CollisionManifold manifold = CollisionDetector::getManifold(colliderA, colliderB);
    if (!manifold.hasCollision)
        return;

    // Check if objects are static (immovable) based on their collider flag
    bool aIsStatic = colliderA->isStatic();
    bool bIsStatic = colliderB->isStatic();

    // Cast to specific collider types
    auto* circleA = dynamic_cast<const CCircleCollider*>(colliderA);
    auto* circleB = dynamic_cast<const CCircleCollider*>(colliderB);
    auto* boxA    = dynamic_cast<const CBoxCollider*>(colliderA);
    auto* boxB    = dynamic_cast<const CBoxCollider*>(colliderB);

    // Dispatch to appropriate collision resolver with manifold
    if (circleA && circleB)
    {
        resolveCircleVsCircle(transformA, transformB, circleA, circleB, aIsStatic, bIsStatic, manifold);
    }
    else if ((circleA && boxB) || (boxA && circleB))
    {
        resolveCircleVsBox(transformA, transformB, circleA, boxA, circleB, boxB, aIsStatic, bIsStatic, manifold);
    }
    else if (boxA && boxB)
    {
        resolveBoxVsBox(transformA, transformB, boxA, boxB, aIsStatic, bIsStatic, manifold);
    }
}

void S2DPhysics::resolveCircleVsCircle(CTransform*              transformA,
                                       CTransform*              transformB,
                                       const CCircleCollider*   circleA,
                                       const CCircleCollider*   circleB,
                                       bool                     aIsStatic,
                                       bool                     bIsStatic,
                                       const CollisionManifold& manifold)
{
    // Get positions and velocities
    Vec2 posA = transformA->getPosition();
    Vec2 posB = transformB->getPosition();
    Vec2 velA = transformA->getVelocity();
    Vec2 velB = transformB->getVelocity();

    // Use collision normal and penetration from manifold (edge-based)
    Vec2  normal      = manifold.normal;
    float penetration = manifold.penetrationDepth;

    // Calculate relative velocity
    Vec2  relativeVel    = velA - velB;
    float velAlongNormal = relativeVel.dot(normal);

#ifdef DEBUG
    std::cout << "[DEBUG] Circle vs Circle: normal=(" << normal.x << "," << normal.y << ")" << std::endl;
    std::cout << "[DEBUG] Circle vs Circle: velAlongNormal=" << velAlongNormal << std::endl;
    std::cout << "[DEBUG] Circle vs Circle: contactPoint=(" << manifold.contactPoints[0].x << ","
              << manifold.contactPoints[0].y << ")" << std::endl;
#endif

    // Only apply velocity changes if objects are approaching
    if (velAlongNormal > 0)
    {
        float restitution      = 0.8f;
        float impulseMagnitude = -(1.0f + restitution) * velAlongNormal;

        // Apply impulse based on static/dynamic state
        if (bIsStatic && !aIsStatic)
        {
            Vec2 impulse = normal * impulseMagnitude;
            transformA->setVelocity(velA + impulse);
        }
        else if (aIsStatic && !bIsStatic)
        {
            Vec2 impulse = normal * impulseMagnitude;
            transformB->setVelocity(velB - impulse);
        }
        else if (!aIsStatic && !bIsStatic)
        {
            Vec2 impulse = normal * (impulseMagnitude / 2.0f);
            transformA->setVelocity(velA + impulse);
            transformB->setVelocity(velB - impulse);
        }
    }

    // Positional correction using edge-based penetration depth
    if (penetration > 0.0f)
    {
        float correctionPercent = 0.8f;
        Vec2  correction        = normal * (penetration * correctionPercent);

        if (bIsStatic && !aIsStatic)
        {
            transformA->setPosition(posA - correction);
        }
        else if (aIsStatic && !bIsStatic)
        {
            transformB->setPosition(posB + correction);
        }
        else if (!aIsStatic && !bIsStatic)
        {
            transformA->setPosition(posA - correction * 0.5f);
            transformB->setPosition(posB + correction * 0.5f);
        }
    }
}

void S2DPhysics::resolveCircleVsBox(CTransform*              transformA,
                                    CTransform*              transformB,
                                    const CCircleCollider*   circleA,
                                    const CBoxCollider*      boxA,
                                    const CCircleCollider*   circleB,
                                    const CBoxCollider*      boxB,
                                    bool                     aIsStatic,
                                    bool                     bIsStatic,
                                    const CollisionManifold& manifold)
{
    // Get positions and velocities
    Vec2 posA = transformA->getPosition();
    Vec2 posB = transformB->getPosition();
    Vec2 velA = transformA->getVelocity();
    Vec2 velB = transformB->getVelocity();

    // Use collision normal and penetration from manifold (edge-based)
    Vec2  normal      = manifold.normal;
    float penetration = manifold.penetrationDepth;

    // Calculate relative velocity
    Vec2  relativeVel    = velA - velB;
    float velAlongNormal = relativeVel.dot(normal);

#ifdef DEBUG
    std::cout << "[DEBUG] Circle vs Box: normal=(" << normal.x << "," << normal.y << ")" << std::endl;
    std::cout << "[DEBUG] Circle vs Box: velA=(" << velA.x << "," << velA.y << ") velB=(" << velB.x << "," << velB.y
              << ")" << std::endl;
    std::cout << "[DEBUG] Circle vs Box: velAlongNormal=" << velAlongNormal << std::endl;
    std::cout << "[DEBUG] Circle vs Box: penetration=" << penetration << std::endl;
    std::cout << "[DEBUG] Circle vs Box: contactPoint=(" << manifold.contactPoints[0].x << ","
              << manifold.contactPoints[0].y << ")" << std::endl;
#endif

    // Only apply velocity changes if objects are approaching
    if (velAlongNormal > 0)
    {
        float restitution      = 0.8f;
        float impulseMagnitude = -(1.0f + restitution) * velAlongNormal;

        // Apply impulse based on static/dynamic state
        if (bIsStatic && !aIsStatic)
        {
            Vec2 impulse = normal * impulseMagnitude;
            transformA->setVelocity(velA + impulse);
        }
        else if (aIsStatic && !bIsStatic)
        {
            Vec2 impulse = normal * impulseMagnitude;
            transformB->setVelocity(velB - impulse);
        }
        else if (!aIsStatic && !bIsStatic)
        {
            Vec2 impulse = normal * (impulseMagnitude / 2.0f);
            transformA->setVelocity(velA + impulse);
            transformB->setVelocity(velB - impulse);
        }
    }

    // Positional correction using edge-based penetration depth
    if (penetration > 0.0f)
    {
        float correctionPercent = 0.8f;
        Vec2  correction        = normal * (penetration * correctionPercent);

        if (bIsStatic && !aIsStatic)
        {
            transformA->setPosition(posA - correction);
        }
        else if (aIsStatic && !bIsStatic)
        {
            transformB->setPosition(posB + correction);
        }
        else if (!aIsStatic && !bIsStatic)
        {
            transformA->setPosition(posA - correction * 0.5f);
            transformB->setPosition(posB + correction * 0.5f);
        }
    }
}

void S2DPhysics::resolveBoxVsBox(CTransform*              transformA,
                                 CTransform*              transformB,
                                 const CBoxCollider*      boxA,
                                 const CBoxCollider*      boxB,
                                 bool                     aIsStatic,
                                 bool                     bIsStatic,
                                 const CollisionManifold& manifold)
{
    // Get positions and velocities
    Vec2 posA = transformA->getPosition();
    Vec2 posB = transformB->getPosition();
    Vec2 velA = transformA->getVelocity();
    Vec2 velB = transformB->getVelocity();

    // Use collision normal and penetration from manifold (edge-based)
    Vec2  normal      = manifold.normal;
    float penetration = manifold.penetrationDepth;

    // Calculate relative velocity
    Vec2  relativeVel    = velA - velB;
    float velAlongNormal = relativeVel.dot(normal);

#ifdef DEBUG
    std::cout << "[DEBUG] Box vs Box: normal=(" << normal.x << "," << normal.y << ")" << std::endl;
    std::cout << "[DEBUG] Box vs Box: velAlongNormal=" << velAlongNormal << std::endl;
    std::cout << "[DEBUG] Box vs Box: contactPoint=(" << manifold.contactPoints[0].x << ","
              << manifold.contactPoints[0].y << ")" << std::endl;
#endif

    // Only apply velocity changes if objects are approaching
    if (velAlongNormal > 0)
    {
        float restitution      = 0.8f;
        float impulseMagnitude = -(1.0f + restitution) * velAlongNormal;

        // Apply impulse based on static/dynamic state
        if (bIsStatic && !aIsStatic)
        {
            Vec2 impulse = normal * impulseMagnitude;
            transformA->setVelocity(velA + impulse);
        }
        else if (aIsStatic && !bIsStatic)
        {
            Vec2 impulse = normal * impulseMagnitude;
            transformB->setVelocity(velB - impulse);
        }
        else if (!aIsStatic && !bIsStatic)
        {
            Vec2 impulse = normal * (impulseMagnitude / 2.0f);
            transformA->setVelocity(velA + impulse);
            transformB->setVelocity(velB - impulse);
        }
    }

    // Positional correction using edge-based penetration depth
    if (penetration > 0.0f)
    {
        float correctionPercent = 0.8f;
        Vec2  correction        = normal * (penetration * correctionPercent);

        if (bIsStatic && !aIsStatic)
        {
            transformA->setPosition(posA - correction);
        }
        else if (aIsStatic && !bIsStatic)
        {
            transformB->setPosition(posB + correction);
        }
        else if (!aIsStatic && !bIsStatic)
        {
            transformA->setPosition(posA - correction * 0.5f);
            transformB->setPosition(posB + correction * 0.5f);
        }
    }
}
