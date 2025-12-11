#include "S2DPhysics.h"
#include "CPhysicsBody2D.h"
#include "CTransform.h"
#include "Vec2.h"

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

S2DPhysics& S2DPhysics::instance()
{
    static S2DPhysics instance;
    return instance;
}

void S2DPhysics::update(float deltaTime)
{
    // Step the Box2D world with fixed timestep
    b2World_Step(m_worldId, m_timeStep, m_subStepCount);

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
    if (!entity.isValid())
    {
        return b2_nullBodyId;
    }

    // Check if entity already has a body
    auto it = m_entityBodyMap.find(entity);
    if (it != m_entityBodyMap.end() && b2Body_IsValid(it->second))
    {
        // Body already exists, destroy it first
        b2DestroyBody(it->second);
    }

    // Create new body
    b2BodyId bodyId = b2CreateBody(m_worldId, &bodyDef);

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
