#include "S2DPhysics.h"
#include "EntityManager.h"

S2DPhysics& S2DPhysics::instance()
{
    static S2DPhysics instance;
    return instance;
}

void S2DPhysics::update(float deltaTime)
{
    auto& entityManager = EntityManager::instance();

    // Get all entities with both Transform and Gravity components
    auto entities = entityManager.getEntitiesWithComponent<CGravity>();

    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        auto gravity   = entity->getComponent<CGravity>();

        if (transform && gravity && gravity->isActive())
        {
            // Get current state
            Vec2 initialVelocity = transform->getVelocity();
            Vec2 position        = transform->getPosition();
            Vec2 force           = gravity->getForce();

            // Physics system is the authoritative source for position and velocity updates
            // First update velocity: v = v0 + at
            Vec2 newVelocity = initialVelocity + (force * deltaTime);
            transform->setVelocity(newVelocity);

            // Then update position: p = p0 + v0t + (1/2)at^2
            // Note: We use initialVelocity here, not newVelocity
            position += initialVelocity * deltaTime + (force * deltaTime * deltaTime * 0.5f);
            transform->setPosition(position);
        }
    }
}
