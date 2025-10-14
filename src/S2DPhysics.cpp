#include "systems/S2DPhysics.h"
#include <algorithm>
#include <iostream>
#include "EntityManager.h"
#include "components/CCollider.h"
#include "components/CCircleCollider.h"
#include "components/CBoxCollider.h"
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
    updateQuadtree();
    checkCollisions();

    // Debug: print all entity velocities after physics update
    std::cout << "[DEBUG] S2DPhysics::update complete. Entity velocities:" << std::endl;
    auto& entityManager = EntityManager::instance();
    auto entities = entityManager.getEntitiesWithComponent<CTransform>();
    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        Vec2 vel = transform->getVelocity();
        std::cout << "[DEBUG]   " << entity->getTag() << ": velocity=(" << vel.x << "," << vel.y << ")" << std::endl;
    }
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

    std::cout << "[DEBUG] updateQuadtree: Found " << entities.size() << " entities with transform" << std::endl;
    std::cout << "[DEBUG] World bounds: center(" << m_worldBounds.position.x << ", " << m_worldBounds.position.y << ")"
              << " halfSize(" << m_worldBounds.halfSize.x << ", " << m_worldBounds.halfSize.y << ")" << std::endl;

    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        Vec2 pos       = transform->getPosition();

        std::cout << "[DEBUG]   Entity " << entity->getTag() << " at (" << pos.x << ", " << pos.y << ")";

        // Only insert entities that are within world bounds
        if (m_worldBounds.contains(pos))
        {
            std::cout << " - inserting into quadtree" << std::endl;
            m_quadtree->insert(entity);
        }
        else
        {
            std::cout << " - OUT OF BOUNDS, clamping..." << std::endl;
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

void S2DPhysics::handleGravity(float deltaTime)
{
    auto& entityManager = EntityManager::instance();

    // First update positions based on gravity
    auto entities = entityManager.getEntitiesWithComponent<CGravity>();
    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        auto gravity   = entity->getComponent<CGravity>();

        if (transform && gravity && gravity->isActive())
        {
            // Update physics as before...
            Vec2 initialVelocity = transform->getVelocity();
            Vec2 position        = transform->getPosition();
            Vec2 force           = gravity->getForce();

            Vec2 newVelocity = initialVelocity + (force * deltaTime);
            transform->setVelocity(newVelocity);

            position += initialVelocity * deltaTime + (force * deltaTime * deltaTime * 0.5f);
            transform->setPosition(position);
        }
    }
}

void S2DPhysics::checkCollisions()
{
    auto& entityManager = EntityManager::instance();
    // Use getEntitiesWithComponentDerived to find all entities with CCollider or derived types
    auto  entities      = entityManager.getEntitiesWithComponentDerived<CCollider>();

    std::cout << "[DEBUG] checkCollisions: Found " << entities.size() << " entities with colliders" << std::endl;

    // Broad phase: Use quadtree to get potential collisions
    for (auto* entity : entities)
    {
        std::cout << "[DEBUG] Starting loop iteration for entity..." << std::endl;
        if (!entity || !entity->isAlive())
        {
            std::cout << "[DEBUG]   Entity is null or not alive, skipping" << std::endl;
            continue;
        }

        auto collider = entity->getComponentDerived<CCollider>();
        if (!collider)
        {
            std::cout << "[DEBUG]   Could not get collider component, skipping" << std::endl;
            continue;
        }

        auto transform = entity->getComponent<CTransform>();
        auto bounds = collider->getBounds();

        std::cout << "[DEBUG] Entity " << entity->getTag()
                  << " at (" << transform->getPosition().x << ", " << transform->getPosition().y << ")"
                  << " bounds: center(" << bounds.position.x << ", " << bounds.position.y << ")"
                  << " halfSize(" << bounds.halfSize.x << ", " << bounds.halfSize.y << ")" << std::endl;

        // Query quadtree for potential collisions
        std::vector<Entity*> potentialCollisions = m_quadtree->query(bounds);

        std::cout << "[DEBUG]   Quadtree query for bounds center(" << bounds.position.x << "," << bounds.position.y << ")"
                  << " halfSize(" << bounds.halfSize.x << "," << bounds.halfSize.y << ")"
                  << " returned " << potentialCollisions.size() << " potential collisions" << std::endl;

        // Narrow phase: Detailed collision checks
        for (auto* other : potentialCollisions)
        {
            std::cout << "[DEBUG]   Checking potential collision..." << std::endl;
            if (!other || !other->isAlive() || entity == other)
            {
                std::cout << "[DEBUG]     Skipping (null=" << (!other) << " alive=" << (other && other->isAlive())
                          << " self=" << (other && entity == other) << ")" << std::endl;
                continue;
            }

            // Skip if we've already processed this pair (avoid duplicate processing)
            if (entity->getId() >= other->getId())
            {
                std::cout << "[DEBUG]     Skipping (already processed this pair)" << std::endl;
                continue;
            }

            auto otherCollider = other->getComponentDerived<CCollider>();
            if (!otherCollider)
            {
                std::cout << "[DEBUG]     No collider on other entity, skipping" << std::endl;
                continue;
            }

            std::cout << "[DEBUG]   Checking collision with " << other->getTag() << std::endl;

            // Detailed collision check
            if (collider->intersects(otherCollider))
            {
                std::cout << "[DEBUG]   *** COLLISION DETECTED between " << entity->getTag()
                          << " and " << other->getTag() << " ***" << std::endl;
                std::cout << "[DEBUG]   Calling handleCollision..." << std::endl;
                handleCollision(entity, other);
                std::cout << "[DEBUG]   handleCollision returned successfully" << std::endl;
            }
        }
    }
}

void S2DPhysics::handleCollision(Entity* a, Entity* b)
{
    std::cout << "[DEBUG] handleCollision: Getting colliders..." << std::endl;
    auto colliderA = a->getComponentDerived<CCollider>();
    auto colliderB = b->getComponentDerived<CCollider>();
    std::cout << "[DEBUG] handleCollision: Got colliders (A=" << (colliderA != nullptr) << ", B=" << (colliderB != nullptr) << ")" << std::endl;

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
    std::cout << "[DEBUG] resolveCollision: START" << std::endl;
    // Get transforms (need non-const access to update velocities)
    auto transformA = a->getComponent<CTransform>();
    auto transformB = b->getComponent<CTransform>();
    std::cout << "[DEBUG] resolveCollision: Got transforms (A=" << (transformA != nullptr) << ", B=" << (transformB != nullptr) << ")" << std::endl;

    if (!transformA || !transformB)
        return;

    // Check if objects are static (immovable) based on their collider flag
    bool aIsStatic = colliderA->isStatic();
    bool bIsStatic = colliderB->isStatic();

    // Cast to specific collider types
    auto* circleA = dynamic_cast<const CCircleCollider*>(colliderA);
    auto* circleB = dynamic_cast<const CCircleCollider*>(colliderB);
    auto* boxA = dynamic_cast<const CBoxCollider*>(colliderA);
    auto* boxB = dynamic_cast<const CBoxCollider*>(colliderB);

    // Dispatch to appropriate collision resolver
    if (circleA && circleB)
    {
        resolveCircleVsCircle(transformA, transformB, circleA, circleB, aIsStatic, bIsStatic);
    }
    else if ((circleA && boxB) || (boxA && circleB))
    {
        resolveCircleVsBox(transformA, transformB, circleA, boxA, circleB, boxB, aIsStatic, bIsStatic);
    }
    else if (boxA && boxB)
    {
        resolveBoxVsBox(transformA, transformB, boxA, boxB, aIsStatic, bIsStatic);
    }
}

void S2DPhysics::resolveCircleVsCircle(CTransform* transformA, CTransform* transformB,
                                       const CCircleCollider* circleA, const CCircleCollider* circleB,
                                       bool aIsStatic, bool bIsStatic)
{
    // Get positions and velocities
    Vec2 posA = transformA->getPosition();
    Vec2 posB = transformB->getPosition();
    Vec2 velA = transformA->getVelocity();
    Vec2 velB = transformB->getVelocity();

    // Calculate collision normal and penetration
    Vec2 delta = posB - posA;
    float distance = delta.length();

    if (distance < 0.0001f)
        return;

    // Normal points from A to B
    Vec2 normal = delta / distance;
    float penetration = (circleA->getRadius() + circleB->getRadius()) - distance;

    if (penetration <= 0.0f)
        return;

    // Calculate relative velocity
    Vec2 relativeVel = velA - velB;
    float velAlongNormal = relativeVel.dot(normal);

    std::cout << "[DEBUG] Circle vs Circle: normal=(" << normal.x << "," << normal.y << ")" << std::endl;
    std::cout << "[DEBUG] Circle vs Circle: velAlongNormal=" << velAlongNormal << std::endl;

    // Only apply velocity changes if objects are approaching
    if (velAlongNormal > 0)
    {
        float restitution = 0.8f;
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

    // Positional correction
    if (penetration > 0.0f)
    {
        float correctionPercent = 0.8f;
        Vec2 correction = normal * (penetration * correctionPercent);

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

void S2DPhysics::resolveCircleVsBox(CTransform* transformA, CTransform* transformB,
                                    const CCircleCollider* circleA, const CBoxCollider* boxA,
                                    const CCircleCollider* circleB, const CBoxCollider* boxB,
                                    bool aIsStatic, bool bIsStatic)
{
    // Determine which is circle and which is box
    const CCircleCollider* circle = circleA ? circleA : circleB;
    const CBoxCollider* box = boxA ? boxA : boxB;

    // Get positions and velocities
    Vec2 posA = transformA->getPosition();
    Vec2 posB = transformB->getPosition();
    Vec2 velA = transformA->getVelocity();
    Vec2 velB = transformB->getVelocity();

    // Determine circle and box positions based on which is which
    Vec2 circlePos = circleA ? posA : posB;
    Vec2 boxPos = circleA ? posB : posA;

    Vec2 boxSize = box->getSize();
    Vec2 halfSize = boxSize * 0.5f;

    // Find closest point on box to circle
    float closestX = std::max(boxPos.x - halfSize.x, std::min(circlePos.x, boxPos.x + halfSize.x));
    float closestY = std::max(boxPos.y - halfSize.y, std::min(circlePos.y, boxPos.y + halfSize.y));
    Vec2 closest(closestX, closestY);

    Vec2 delta = circlePos - closest;
    float distance = delta.length();

    Vec2 normal;
    float penetration;

    if (distance < 0.0001f)
    {
        // Circle center is inside box - push out along shortest axis
        float overlapX = halfSize.x - std::abs(circlePos.x - boxPos.x);
        float overlapY = halfSize.y - std::abs(circlePos.y - boxPos.y);

        if (overlapX < overlapY)
        {
            // Normal points from box to circle (away from box center)
            normal = Vec2((circlePos.x > boxPos.x) ? 1.0f : -1.0f, 0.0f);
            penetration = overlapX + circle->getRadius();
        }
        else
        {
            normal = Vec2(0.0f, (circlePos.y > boxPos.y) ? 1.0f : -1.0f);
            penetration = overlapY + circle->getRadius();
        }
    }
    else
    {
        // Normal points from closest point on box to circle center
        normal = delta / distance;
        penetration = circle->getRadius() - distance;
    }

    // At this point, normal points from box to circle
    // We need normal to point from A to B
    // If A is circle and B is box: normal should point from circle to box (flip it)
    // If A is box and B is circle: normal should point from box to circle (keep it)
    if (circleA)
    {
        normal = normal * -1.0f;
    }

    if (penetration <= 0.0f)
        return;

    // Calculate relative velocity
    Vec2 relativeVel = velA - velB;
    float velAlongNormal = relativeVel.dot(normal);

    std::cout << "[DEBUG] Circle vs Box: normal=(" << normal.x << "," << normal.y << ")" << std::endl;
    std::cout << "[DEBUG] Circle vs Box: velA=(" << velA.x << "," << velA.y << ") velB=(" << velB.x << "," << velB.y << ")" << std::endl;
    std::cout << "[DEBUG] Circle vs Box: velAlongNormal=" << velAlongNormal << std::endl;
    std::cout << "[DEBUG] Circle vs Box: penetration=" << penetration << std::endl;

    // Only apply velocity changes if objects are approaching
    if (velAlongNormal > 0)
    {
        float restitution = 0.8f;
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

    // Positional correction
    if (penetration > 0.0f)
    {
        float correctionPercent = 0.8f;
        Vec2 correction = normal * (penetration * correctionPercent);

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

void S2DPhysics::resolveBoxVsBox(CTransform* transformA, CTransform* transformB,
                                 const CBoxCollider* boxA, const CBoxCollider* boxB,
                                 bool aIsStatic, bool bIsStatic)
{
    // Get positions and velocities
    Vec2 posA = transformA->getPosition();
    Vec2 posB = transformB->getPosition();
    Vec2 velA = transformA->getVelocity();
    Vec2 velB = transformB->getVelocity();

    // Calculate collision normal and penetration
    Vec2 delta = posB - posA;
    Vec2 halfSizeA = boxA->getSize() * 0.5f;
    Vec2 halfSizeB = boxB->getSize() * 0.5f;

    // Calculate overlap on each axis
    float overlapX = (halfSizeA.x + halfSizeB.x) - std::abs(delta.x);
    float overlapY = (halfSizeA.y + halfSizeB.y) - std::abs(delta.y);

    Vec2 normal;
    float penetration;

    // Separate along axis of least penetration
    if (overlapX < overlapY)
    {
        // Normal points from A to B along X axis
        normal = Vec2((delta.x > 0) ? 1.0f : -1.0f, 0.0f);
        penetration = overlapX;
    }
    else
    {
        // Normal points from A to B along Y axis
        normal = Vec2(0.0f, (delta.y > 0) ? 1.0f : -1.0f);
        penetration = overlapY;
    }

    if (penetration <= 0.0f)
        return;

    // Calculate relative velocity
    Vec2 relativeVel = velA - velB;
    float velAlongNormal = relativeVel.dot(normal);

    std::cout << "[DEBUG] Box vs Box: normal=(" << normal.x << "," << normal.y << ")" << std::endl;
    std::cout << "[DEBUG] Box vs Box: velAlongNormal=" << velAlongNormal << std::endl;

    // Only apply velocity changes if objects are approaching
    if (velAlongNormal > 0)
    {
        float restitution = 0.8f;
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

    // Positional correction
    if (penetration > 0.0f)
    {
        float correctionPercent = 0.8f;
        Vec2 correction = normal * (penetration * correctionPercent);

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
