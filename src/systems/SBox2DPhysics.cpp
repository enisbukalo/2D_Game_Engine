#include "SBox2DPhysics.h"
#include "CPhysicsBody2D.h"
#include "CTransform.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Vec2.h"

SBox2DPhysics* SBox2DPhysics::s_instance = nullptr;

SBox2DPhysics::SBox2DPhysics() : m_timeStep(1.0f / 60.0f), m_subStepCount(6)
{
    // Create Box2D world with default gravity (Y-up: negative Y = downward)
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity    = {0.0f, -10.0f};

    // Increase substep count for better stability with many bodies
    // Box2D v3 uses substeps internally during b2World_Step
    // Note: we also pass subStepCount to b2World_Step for additional substeps

    m_worldId = b2CreateWorld(&worldDef);
}

SBox2DPhysics::~SBox2DPhysics()
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

    s_instance = nullptr;
}

SBox2DPhysics& SBox2DPhysics::instance()
{
    if (!s_instance)
    {
        s_instance = new SBox2DPhysics();
    }
    return *s_instance;
}

void SBox2DPhysics::update(float deltaTime)
{
    // Step the Box2D world with fixed timestep
    b2World_Step(m_worldId, m_timeStep, m_subStepCount);

    // Sync Box2D bodies back to CTransform components
    for (auto& pair : m_entityBodyMap)
    {
        Entity* entity = reinterpret_cast<Entity*>(b2Body_GetUserData(pair.second));
        if (!entity)
            continue;

        auto physicsBody = entity->getComponent<CPhysicsBody2D>();
        auto transform   = entity->getComponent<CTransform>();

        if (physicsBody && transform && physicsBody->isInitialized())
        {
            physicsBody->syncToTransform(transform);
        }
    }
}

void SBox2DPhysics::setGravity(const b2Vec2& gravity)
{
    b2World_SetGravity(m_worldId, gravity);
}

b2Vec2 SBox2DPhysics::getGravity() const
{
    return b2World_GetGravity(m_worldId);
}

b2BodyId SBox2DPhysics::createBody(Entity* entity, const b2BodyDef& bodyDef)
{
    if (!entity)
    {
        return b2_nullBodyId;
    }

    // Check if entity already has a body
    auto it = m_entityBodyMap.find(entity->getId());
    if (it != m_entityBodyMap.end() && b2Body_IsValid(it->second))
    {
        // Body already exists, destroy it first
        b2DestroyBody(it->second);
    }

    // Create new body
    b2BodyId bodyId = b2CreateBody(m_worldId, &bodyDef);

    // Find the physics root owner (highest ancestor with CPhysicsBody2D)
    Entity* physicsRoot = CPhysicsBody2D::getPhysicsRootOwner(entity);

    // Store physics root owner pointer in body user data
    b2Body_SetUserData(bodyId, physicsRoot);

    // Map entity to body
    m_entityBodyMap[entity->getId()] = bodyId;

    return bodyId;
}

void SBox2DPhysics::destroyBody(const Entity* entity)
{
    if (!entity)
    {
        return;
    }

    auto it = m_entityBodyMap.find(entity->getId());
    if (it != m_entityBodyMap.end())
    {
        if (b2Body_IsValid(it->second))
        {
            b2DestroyBody(it->second);
        }
        m_entityBodyMap.erase(it);
    }
}

b2BodyId SBox2DPhysics::getBody(const Entity* entity)
{
    if (!entity)
    {
        return b2_nullBodyId;
    }

    auto it = m_entityBodyMap.find(entity->getId());
    if (it != m_entityBodyMap.end())
    {
        return it->second;
    }

    return b2_nullBodyId;
}

void SBox2DPhysics::updatePhysicsRootOwners(Entity* entity)
{
    if (!entity)
    {
        return;
    }

    // Update this entity's body if it has one
    if (entity->hasComponent<CPhysicsBody2D>())
    {
        auto bodyId = getBody(entity);
        if (b2Body_IsValid(bodyId))
        {
            Entity* physicsRoot = CPhysicsBody2D::getPhysicsRootOwner(entity);
            b2Body_SetUserData(bodyId, physicsRoot);
        }
    }

    // Recursively update all children
    for (auto child : entity->getChildren())
    {
        updatePhysicsRootOwners(child.get());
    }
}

void SBox2DPhysics::queryAABB(const b2AABB& aabb, b2OverlapResultFcn* callback, void* context)
{
    b2QueryFilter filter = b2DefaultQueryFilter();
    b2World_OverlapAABB(m_worldId, aabb, filter, callback, context);
}

void SBox2DPhysics::rayCast(const b2Vec2& origin, const b2Vec2& translation, b2CastResultFcn* callback, void* context)
{
    b2QueryFilter filter = b2DefaultQueryFilter();
    b2World_CastRay(m_worldId, origin, translation, filter, callback, context);
}
