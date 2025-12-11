#include "S2DPhysics.h"
#include "CPhysicsBody2D.h"
#include "CTransform.h"
#include "Vec2.h"
#include "World.h"

namespace Systems
{

S2DPhysics::S2DPhysics() : m_timeStep(1.0f / 60.0f), m_subStepCount(6)
{
    // Create Box2D world with default gravity (Y-up: negative Y = downward)
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity    = {0.0f, -10.0f};

    // Increase substep count for better stability with many bodies
    // Box2D v3 uses substeps internally during b2World_Step
    // Note: we also pass subStepCount to b2World_Step for additional substeps

    m_worldId = b2CreateWorld(&worldDef);
}

S2DPhysics::~S2DPhysics()
{
    // Destroy all bodies
    for (auto& pair : m_entityBodyMap)
    {
        if (b2Body_IsValid(pair.second))
        {
            b2DestroyBody(pair.second);
        }
    }
    m_entityBodyMap.clear();

    // Destroy world
    if (b2World_IsValid(m_worldId))
    {
        b2DestroyWorld(m_worldId);
    }
}

void S2DPhysics::update(float deltaTime, World& world)
{
    (void)deltaTime;

    // Clean up any bodies that belong to destroyed entities
    pruneDestroyedBodies(world);

    // Ensure every entity with physics+transform has a Box2D body and sync state before stepping
    world.view2<::Components::CTransform, ::Components::CPhysicsBody2D>([this](Entity entity,
                                                                                ::Components::CTransform&     transform,
                                                                                ::Components::CPhysicsBody2D& body)
                                                                       {
                                                                           ensureBodyForEntity(entity, transform, body);
                                                                           body.syncFromTransform(&transform);
                                                                       });

    // Run fixed-step callbacks then advance the physics world
    runFixedUpdates(m_timeStep);
    b2World_Step(m_worldId, m_timeStep, m_subStepCount);

    // Sync simulation results back to transforms
    world.view2<::Components::CTransform, ::Components::CPhysicsBody2D>([](Entity /*entity*/,
                                                                           ::Components::CTransform&     transform,
                                                                           ::Components::CPhysicsBody2D& body)
                                                                       { body.syncToTransform(&transform); });

}

void S2DPhysics::setGravity(const b2Vec2& gravity)
{
    b2World_SetGravity(m_worldId, gravity);
}

b2Vec2 S2DPhysics::getGravity() const
{
    return b2World_GetGravity(m_worldId);
}

b2BodyId S2DPhysics::createBody(Entity entity, const b2BodyDef& bodyDef)
{
    if (entity.isValid())
    {
        // Check if entity already has a body
        auto it = m_entityBodyMap.find(entity);
        if (it != m_entityBodyMap.end() && b2Body_IsValid(it->second))
        {
            b2DestroyBody(it->second);
        }
    }

    b2BodyId bodyId = b2CreateBody(m_worldId, &bodyDef);

    if (!entity.isValid())
    {
        return bodyId;
    }

    // TODO: Store Entity ID in body user data when needed
    // b2Body_SetUserData(bodyId, &entity);

    // Map entity to body
    m_entityBodyMap[entity] = bodyId;

    return bodyId;
}

void S2DPhysics::destroyBody(Entity entity)
{
    if (!entity.isValid())
    {
        return;
    }

    auto it = m_entityBodyMap.find(entity);
    if (it != m_entityBodyMap.end())
    {
        if (b2Body_IsValid(it->second))
        {
            b2DestroyBody(it->second);
        }
        m_entityBodyMap.erase(it);
    }
}

void S2DPhysics::ensureBodyForEntity(Entity entity, ::Components::CTransform& transform, ::Components::CPhysicsBody2D& body)
{
    if (!entity.isValid())
    {
        return;
    }

    body.setEntity(entity);

    b2BodyId existing = getBody(entity);
    if (!b2Body_IsValid(existing))
    {
        body.initialize({transform.position.x, transform.position.y}, body.getBodyType());
        existing = getBody(entity);
    }

    // Ensure we keep the mapping in sync even if initialize was skipped elsewhere
    if (b2Body_IsValid(existing))
    {
        m_entityBodyMap[entity] = existing;
    }
}

void S2DPhysics::pruneDestroyedBodies(const World& world)
{
    for (auto it = m_entityBodyMap.begin(); it != m_entityBodyMap.end();)
    {
        const Entity entity = it->first;
        const b2BodyId body = it->second;

        const bool deadEntity    = !world.isAlive(entity);
        const bool missingComponents = deadEntity || !world.has<::Components::CPhysicsBody2D>(entity) ||
                                       !world.has<::Components::CTransform>(entity);
        const bool deadBody   = !b2Body_IsValid(body);

        if (missingComponents || deadBody)
        {
            if (deadBody)
            {
                // Body already gone; just drop the mapping
                it = m_entityBodyMap.erase(it);
                continue;
            }

            destroyBody(entity);
            it = m_entityBodyMap.erase(it);
            continue;
        }

        ++it;
    }
}

b2BodyId S2DPhysics::getBody(Entity entity)
{
    if (!entity.isValid())
    {
        return b2_nullBodyId;
    }

    auto it = m_entityBodyMap.find(entity);
    if (it != m_entityBodyMap.end())
    {
        return it->second;
    }

    return b2_nullBodyId;
}

void S2DPhysics::queryAABB(const b2AABB& aabb, b2OverlapResultFcn* callback, void* context)
{
    b2QueryFilter filter = b2DefaultQueryFilter();
    b2World_OverlapAABB(m_worldId, aabb, filter, callback, context);
}

void S2DPhysics::rayCast(const b2Vec2& origin, const b2Vec2& translation, b2CastResultFcn* callback, void* context)
{
    b2QueryFilter filter = b2DefaultQueryFilter();
    b2World_CastRay(m_worldId, origin, translation, filter, callback, context);
}

void S2DPhysics::registerBody(Components::CPhysicsBody2D* body)
{
    if (!body)
        return;

    // Check if already registered
    auto it = std::find(m_registeredBodies.begin(), m_registeredBodies.end(), body);
    if (it == m_registeredBodies.end())
    {
        m_registeredBodies.push_back(body);
    }
}

void S2DPhysics::unregisterBody(Components::CPhysicsBody2D* body)
{
    if (!body)
        return;

    auto it = std::find(m_registeredBodies.begin(), m_registeredBodies.end(), body);
    if (it != m_registeredBodies.end())
    {
        m_registeredBodies.erase(it);
    }
}

void S2DPhysics::runFixedUpdates(float timeStep)
{
    // Iterate all registered physics bodies and invoke their fixed-update callbacks
    for (auto* body : m_registeredBodies)
    {
        if (body && body->isInitialized() && body->hasFixedUpdateCallback())
        {
            body->getFixedUpdateCallback()(timeStep);
        }
    }
}

}  // namespace Systems
