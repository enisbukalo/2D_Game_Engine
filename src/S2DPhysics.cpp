#include "S2DPhysics.h"
#include "EntityManager.h"
#include <algorithm>  // Add at top of file

S2DPhysics& S2DPhysics::instance()
{
    static S2DPhysics instance;
    return instance;
}

S2DPhysics::S2DPhysics()
    : m_worldBounds(Vec2(0, 0), Vec2(1000, 1000))  // Default world size
{
    m_quadtree = std::make_unique<Quadtree>(0, m_worldBounds);
}

void S2DPhysics::setWorldBounds(const Vec2& center, const Vec2& size)
{
    m_worldBounds = AABB(center, size * 0.5f);
    m_quadtree = std::make_unique<Quadtree>(0, m_worldBounds);
}

void S2DPhysics::updateQuadtree()
{
    // TODO: Implement Way For Reference To Move With Camera.
    //      - Move Grid (Spatial Hashing).
    //      - Multiple Grids (Chunking).
    //      - Infinite Grid (Relative Coordinates).

    m_quadtree->clear();

    auto& entityManager = EntityManager::instance();
    auto entities = entityManager.getEntitiesWithComponent<CTransform>();

    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        Vec2 pos = transform->getPosition();

        // Only insert entities that are within world bounds
        if (m_worldBounds.contains(pos))
        {
            m_quadtree->insert(entity);
        }
        else
        {
            // TODO: Handle out-of-bounds entities (optional)
            // You could:
            // 1. Wrap around to the other side
            // 2. Bounce off the boundaries
            // 3. Destroy the entity
            // 4. Teleport to nearest valid position

            Vec2 newPos = pos;
            newPos.x = std::clamp(pos.x,
                m_worldBounds.position.x - m_worldBounds.halfSize.x,
                m_worldBounds.position.x + m_worldBounds.halfSize.x);
            newPos.y = std::clamp(pos.y,
                m_worldBounds.position.y - m_worldBounds.halfSize.y,
                m_worldBounds.position.y + m_worldBounds.halfSize.y);

            transform->setPosition(newPos);
            m_quadtree->insert(entity);
        }
    }
}

void S2DPhysics::update(float deltaTime)
{
    handleGravity(deltaTime);

    // Then update the quadtree with new positions
    updateQuadtree();

    // Now you can use m_quadtree->query() for collision detection
    // For each entity that needs collision checking:
    // 1. Create an AABB around it
    // 2. Query the quadtree for potential collisions
    // 3. Do detailed collision checks only with returned entities
}

void S2DPhysics::handleGravity(float deltaTime)
{
    auto& entityManager = EntityManager::instance();

    // First update positions based on gravity
    auto entities = entityManager.getEntitiesWithComponent<CGravity>();
    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        auto gravity = entity->getComponent<CGravity>();

        if (transform && gravity && gravity->isActive())
        {
            // Update physics as before...
            Vec2 initialVelocity = transform->getVelocity();
            Vec2 position = transform->getPosition();
            Vec2 force = gravity->getForce();

            Vec2 newVelocity = initialVelocity + (force * deltaTime);
            transform->setVelocity(newVelocity);

            position += initialVelocity * deltaTime + (force * deltaTime * deltaTime * 0.5f);
            transform->setPosition(position);
        }
    }
}
