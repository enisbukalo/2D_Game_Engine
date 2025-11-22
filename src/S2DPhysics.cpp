#include "systems/S2DPhysics.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include "EntityManager.h"
#include "components/CBoxCollider.h"
#include "components/CCircleCollider.h"
#include "components/CCollider.h"
#include "components/CForceDebug.h"
#include "components/CGravity.h"
#include "components/CRigidBody2D.h"
#include "components/CTransform.h"

S2DPhysics& S2DPhysics::instance()
{
    static S2DPhysics instance;
    return instance;
}

void S2DPhysics::update(float deltaTime)
{
    handleGravity(deltaTime);
    clearForces();  // Clear after accumulating forces for this frame (saves to totalForce for visualization)
    applyDrag(deltaTime);
    integratePositions(deltaTime);
    updateQuadtree();
    checkCollisions();

    // Debug: print all entity velocities after physics update
    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("S2DPhysics::update complete. Entity velocities:");
    auto& entityManager = EntityManager::instance();
    auto  entities      = entityManager.getEntitiesWithComponent<CTransform>();
    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        Vec2 vel       = transform->getVelocity();
        if (auto logger = spdlog::get("GameEngine"))
            logger->debug("  {}: velocity=({},{})", entity->getTag(), vel.x, vel.y);
    }
}

S2DPhysics::S2DPhysics()
    : m_worldBounds(Vec2(0, 0), Vec2(1000, 1000)),
      m_globalGravity(Vec2(0, 981))  // Default world size and gravity (9.81 m/s² at 100px/m scale)
{
    m_quadtree = std::make_unique<Quadtree>(0, m_worldBounds);
}

void S2DPhysics::setWorldBounds(const Vec2& center, const Vec2& size)
{
    m_worldBounds = AABB(center, size);
    m_quadtree    = std::make_unique<Quadtree>(0, m_worldBounds);
}

const Quadtree* S2DPhysics::getQuadtree() const
{
    return m_quadtree.get();
}

void S2DPhysics::setGlobalGravity(const Vec2& gravity)
{
    m_globalGravity = gravity;
}

Vec2 S2DPhysics::getGlobalGravity() const
{
    return m_globalGravity;
}

void S2DPhysics::clearForces()
{
    auto& entityManager = EntityManager::instance();
    auto rigidBodies = entityManager.getEntitiesWithComponent<CRigidBody2D>();

    for (auto* entity : rigidBodies)
    {
        auto rigidBody = entity->getComponent<CRigidBody2D>();
        if (rigidBody && rigidBody->isActive())
        {
            rigidBody->clearForces();
        }
    }
}

void S2DPhysics::applyDrag(float deltaTime)
{
    auto& entityManager = EntityManager::instance();
    auto rigidBodies = entityManager.getEntitiesWithComponent<CRigidBody2D>();

    for (auto* entity : rigidBodies)
    {
        auto rigidBody = entity->getComponent<CRigidBody2D>();
        auto transform = entity->getComponent<CTransform>();

        if (!rigidBody || !transform || !rigidBody->isActive())
            continue;

        // Skip kinematic bodies (they don't respond to drag)
        if (rigidBody->isKinematic())
            continue;

        // Get current velocity
        Vec2 velocity = transform->getVelocity();
        float velocityMagnitude = velocity.length();

        // Skip if velocity is negligible to avoid division by zero
        if (velocityMagnitude < 0.001f)
            continue;

        // Apply linear drag: F_drag = -drag * velocity
        // This creates an exponential decay: v(t) = v0 * e^(-drag * t)
        // We use the approximation: v_new = v_old * (1 - drag * dt)
        float linearDrag = rigidBody->getLinearDrag();
        float dragFactor = 1.0f - (linearDrag * deltaTime);

        // Clamp to prevent negative velocities from drag
        dragFactor = std::max(0.0f, dragFactor);

        // Apply drag to velocity
        Vec2 newVelocity = velocity * dragFactor;
        transform->setVelocity(newVelocity);
    }
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

    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("updateQuadtree: Found {} entities with transform", entities.size());
    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("World bounds: center({}, {}) halfSize({}, {})",
                      m_worldBounds.position.x,
                      m_worldBounds.position.y,
                      m_worldBounds.halfSize.x,
                      m_worldBounds.halfSize.y);

    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        Vec2 pos       = transform->getPosition();

        if (auto logger = spdlog::get("GameEngine"))
            logger->debug("  Entity {} at ({}, {})", entity->getTag(), pos.x, pos.y);

        // Only insert entities that are within world bounds
        if (m_worldBounds.contains(pos))
        {
            if (auto logger = spdlog::get("GameEngine"))
                logger->debug(" - inserting into quadtree");
            m_quadtree->insert(entity);
        }
        else
        {
            if (auto logger = spdlog::get("GameEngine"))
                logger->debug(" - OUT OF BOUNDS, clamping...");
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

    // Apply gravity force to entities with CRigidBody2D
    auto rigidBodies = entityManager.getEntitiesWithComponent<CRigidBody2D>();
    for (auto* entity : rigidBodies)
    {
        auto transform = entity->getComponent<CTransform>();
        auto rigidBody = entity->getComponent<CRigidBody2D>();

        if (transform && rigidBody && rigidBody->isActive())
        {
            // Only apply gravity if the body uses gravity and is not kinematic
            if (rigidBody->getUseGravity() && !rigidBody->isKinematic())
            {
                // Apply global gravity multiplied by entity's gravity scale: v = v0 + (g * scale) * dt
                Vec2 currentVelocity = transform->getVelocity();
                Vec2 force           = m_globalGravity * rigidBody->getGravityScale();
                Vec2 newVelocity     = currentVelocity + (force * deltaTime);

                // Store force in rigid body for visualization
                rigidBody->addForce(force);

                transform->setVelocity(newVelocity);
            }
        }
    }

    // Backward compatibility: Apply gravity force to entities with CGravity (deprecated)
    auto entities = entityManager.getEntitiesWithComponent<CGravity>();
    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        auto gravity   = entity->getComponent<CGravity>();

        if (transform && gravity && gravity->isActive())
        {
            // Apply global gravity multiplied by entity's multiplier: v = v0 + (g * m) * dt
            Vec2 currentVelocity = transform->getVelocity();
            Vec2 force           = m_globalGravity * gravity->getMultiplier();
            Vec2 newVelocity     = currentVelocity + (force * deltaTime);

            // Store force in debug component for visualization (if entity has one)
            auto forceDebug = entity->getComponent<CForceDebug>();
            if (forceDebug)
            {
                forceDebug->setGravityForce(force);
            }

            transform->setVelocity(newVelocity);
        }
    }
}

void S2DPhysics::checkCollisions()
{
    auto& entityManager = EntityManager::instance();
    // Use getEntitiesWithComponentDerived to find all entities with CCollider or derived types
    auto entities = entityManager.getEntitiesWithComponentDerived<CCollider>();

    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("checkCollisions: Found {} entities with colliders", entities.size());

    // Broad phase: Use quadtree to get potential collisions
    for (auto* entity : entities)
    {
        if (auto logger = spdlog::get("GameEngine"))
            logger->debug("Starting loop iteration for entity...");
        if (!entity || !entity->isAlive())
        {
            if (auto logger = spdlog::get("GameEngine"))
                logger->debug("  Entity is null or not alive, skipping");
            continue;
        }

        auto collider = entity->getComponentDerived<CCollider>();
        if (!collider)
        {
            if (auto logger = spdlog::get("GameEngine"))
                logger->debug("  Could not get collider component, skipping");
            continue;
        }

        auto transform = entity->getComponent<CTransform>();
        auto bounds    = collider->getBounds();

        if (auto logger = spdlog::get("GameEngine"))
            logger->debug("Entity {} at ({}, {}) bounds: center({}, {}) halfSize({}, {})",
                          entity->getTag(),
                          transform->getPosition().x,
                          transform->getPosition().y,
                          bounds.position.x,
                          bounds.position.y,
                          bounds.halfSize.x,
                          bounds.halfSize.y);

        // Query quadtree for potential collisions
        std::vector<Entity*> potentialCollisions = m_quadtree->query(bounds);

        if (auto logger = spdlog::get("GameEngine"))
            logger->debug("  Quadtree query for bounds center({},{}) halfSize({},{}) returned {} potential collisions",
                          bounds.position.x,
                          bounds.position.y,
                          bounds.halfSize.x,
                          bounds.halfSize.y,
                          potentialCollisions.size());

        // Narrow phase: Detailed collision checks
        for (auto* other : potentialCollisions)
        {
            if (auto logger = spdlog::get("GameEngine"))
                logger->debug("  Checking potential collision...");
            if (!other || !other->isAlive() || entity == other)
            {
                if (auto logger = spdlog::get("GameEngine"))
                    logger->debug("    Skipping (null={} alive={} self={})", (!other), (other && other->isAlive()), (other && entity == other));
                continue;
            }

            // Skip if we've already processed this pair (avoid duplicate processing)
            if (entity->getId() >= other->getId())
            {
                if (auto logger = spdlog::get("GameEngine"))
                    logger->debug("    Skipping (already processed this pair)");
                continue;
            }

            auto otherCollider = other->getComponentDerived<CCollider>();
            if (!otherCollider)
            {
                if (auto logger = spdlog::get("GameEngine"))
                    logger->debug("    No collider on other entity, skipping");
                continue;
            }

            if (auto logger = spdlog::get("GameEngine"))
                logger->debug("  Checking collision with {}", other->getTag());

            // Detailed collision check
            if (collider->intersects(otherCollider))
            {
                if (auto logger = spdlog::get("GameEngine"))
                    logger->debug("  *** COLLISION DETECTED between {} and {} ***", entity->getTag(), other->getTag());
                if (auto logger = spdlog::get("GameEngine"))
                    logger->debug("  Calling handleCollision...");
                handleCollision(entity, other);
                if (auto logger = spdlog::get("GameEngine"))
                    logger->debug("  handleCollision returned successfully");
            }
        }
    }
}

void S2DPhysics::handleCollision(Entity* a, Entity* b)
{
    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("handleCollision: Getting colliders...");
    auto colliderA = a->getComponentDerived<CCollider>();
    auto colliderB = b->getComponentDerived<CCollider>();
    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("handleCollision: Got colliders (A={}, B={})", (colliderA != nullptr), (colliderB != nullptr));

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
    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("resolveCollision: START");
    // Get transforms (need non-const access to update velocities)
    auto transformA = a->getComponent<CTransform>();
    auto transformB = b->getComponent<CTransform>();
    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("resolveCollision: Got transforms (A={}, B={})", (transformA != nullptr), (transformB != nullptr));

    if (!transformA || !transformB)
        return;

    // Get collision manifold with edge-based contact points
    CollisionManifold manifold = CollisionDetector::getManifold(colliderA, colliderB);
    if (!manifold.hasCollision)
        return;

    // Check if objects are static (immovable) based on their collider flag
    bool aIsStatic = colliderA->isStatic();
    bool bIsStatic = colliderB->isStatic();

    // Get rigid bodies (if present) to determine restitution
    auto rigidBodyA = a->getComponent<CRigidBody2D>();
    auto rigidBodyB = b->getComponent<CRigidBody2D>();

    // Calculate combined restitution (use minimum of both objects' restitution values)
    // If an entity doesn't have a rigid body, use default hardcoded value (0.8f for backward compatibility)
    float restitutionA = rigidBodyA ? rigidBodyA->getRestitution() : 0.8f;
    float restitutionB = rigidBodyB ? rigidBodyB->getRestitution() : 0.8f;
    float restitution = (restitutionA < restitutionB) ? restitutionA : restitutionB;

    // Cast to specific collider types
    auto* circleA = dynamic_cast<const CCircleCollider*>(colliderA);
    auto* circleB = dynamic_cast<const CCircleCollider*>(colliderB);
    auto* boxA    = dynamic_cast<const CBoxCollider*>(colliderA);
    auto* boxB    = dynamic_cast<const CBoxCollider*>(colliderB);

    // Dispatch to appropriate collision resolver with manifold
    if (circleA && circleB)
    {
        resolveCircleVsCircle(transformA, transformB, circleA, circleB, aIsStatic, bIsStatic, manifold, restitution);
    }
    else if ((circleA && boxB) || (boxA && circleB))
    {
        resolveCircleVsBox(transformA, transformB, circleA, boxA, circleB, boxB, aIsStatic, bIsStatic, manifold, restitution);
    }
    else if (boxA && boxB)
    {
        resolveBoxVsBox(transformA, transformB, boxA, boxB, aIsStatic, bIsStatic, manifold, restitution);
    }
}

void S2DPhysics::resolveCircleVsCircle(CTransform*              transformA,
                                       CTransform*              transformB,
                                       const CCircleCollider*   circleA,
                                       const CCircleCollider*   circleB,
                                       bool                     aIsStatic,
                                       bool                     bIsStatic,
                                       const CollisionManifold& manifold,
                                       float                    restitution)
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

    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("Circle vs Circle: normal=({},{})", normal.x, normal.y);
    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("Circle vs Circle: velAlongNormal={}", velAlongNormal);
    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("Circle vs Circle: contactPoint=({},{})", manifold.contactPoints[0].x, manifold.contactPoints[0].y);

    // Only apply velocity changes if objects are approaching
    if (velAlongNormal > 0)
    {
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
                                    const CollisionManifold& manifold,
                                    float                    restitution)
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

    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("Circle vs Box: normal=({},{})", normal.x, normal.y);
    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("Circle vs Box: velA=({},{}) velB=({},{})", velA.x, velA.y, velB.x, velB.y);
    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("Circle vs Box: velAlongNormal={}", velAlongNormal);
    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("Circle vs Box: penetration={}", penetration);
    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("Circle vs Box: contactPoint=({},{})", manifold.contactPoints[0].x, manifold.contactPoints[0].y);

    // Only apply velocity changes if objects are approaching
    if (velAlongNormal > 0)
    {
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
                                 const CollisionManifold& manifold,
                                 float                    restitution)
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

    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("Box vs Box: normal=({},{})", normal.x, normal.y);
    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("Box vs Box: velAlongNormal={}", velAlongNormal);
    if (auto logger = spdlog::get("GameEngine"))
        logger->debug("Box vs Box: contactPoint=({},{})", manifold.contactPoints[0].x, manifold.contactPoints[0].y);

    // Only apply velocity changes if objects are approaching
    if (velAlongNormal > 0)
    {
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
