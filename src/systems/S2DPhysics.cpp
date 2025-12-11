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
    for (auto& pair : m_entityBodyMap)
    {
        if (b2Body_IsValid(pair.second))
        {
            b2DestroyBody(pair.second);
        }
    }
    m_entityBodyMap.clear();
    m_fixedCallbacks.clear();

    if (b2World_IsValid(m_worldId))
    {
        b2DestroyWorld(m_worldId);
    }
}

void S2DPhysics::update(float deltaTime, World& world)
{
    (void)deltaTime;

    pruneDestroyedBodies(world);

    world.view2<::Components::CTransform, ::Components::CPhysicsBody2D>([this](Entity entity,
                                                                                ::Components::CTransform&     transform,
                                                                                ::Components::CPhysicsBody2D& body)
                                                                       {
                                                                           ensureBodyForEntity(entity, transform, body);
                                                                           syncFromTransform(entity, transform);
                                                                       });

    runFixedUpdates(m_timeStep);
    b2World_Step(m_worldId, m_timeStep, m_subStepCount);

    world.view2<::Components::CTransform, ::Components::CPhysicsBody2D>([this](Entity entity,
                                                                                ::Components::CTransform&     transform,
                                                                                ::Components::CPhysicsBody2D& body)
                                                                       { syncToTransform(entity, transform); });
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
    m_fixedCallbacks.erase(entity);
}

void S2DPhysics::ensureBodyForEntity(Entity entity, ::Components::CTransform& transform, ::Components::CPhysicsBody2D& body)
{
    if (!entity.isValid())
    {
        return;
    }

    body.owner = entity;

    b2BodyId existing = getBody(entity);
    if (!b2Body_IsValid(existing))
    {
        b2BodyDef bodyDef   = b2DefaultBodyDef();
        bodyDef.position    = {transform.position.x, transform.position.y};
        bodyDef.linearDamping  = body.linearDamping;
        bodyDef.angularDamping = body.angularDamping;
        bodyDef.gravityScale   = body.gravityScale;

        switch (body.bodyType)
        {
            case Components::BodyType::Static:
                bodyDef.type = b2_staticBody;
                break;
            case Components::BodyType::Kinematic:
                bodyDef.type = b2_kinematicBody;
                break;
            case Components::BodyType::Dynamic:
                bodyDef.type = b2_dynamicBody;
                break;
        }

        existing = createBody(entity, bodyDef);
    }

    if (b2Body_IsValid(existing))
    {
        m_entityBodyMap[entity] = existing;
        b2Body_SetFixedRotation(existing, body.fixedRotation);
        b2Body_SetLinearDamping(existing, body.linearDamping);
        b2Body_SetAngularDamping(existing, body.angularDamping);
        b2Body_SetGravityScale(existing, body.gravityScale);
    }
}

void S2DPhysics::pruneDestroyedBodies(const World& world)
{
    for (auto it = m_entityBodyMap.begin(); it != m_entityBodyMap.end();)
    {
        const Entity  entity = it->first;
        const b2BodyId body  = it->second;

        const bool deadEntity        = !world.isAlive(entity);
        const bool missingComponents = deadEntity || !world.has<::Components::CPhysicsBody2D>(entity) ||
                                       !world.has<::Components::CTransform>(entity);
        const bool deadBody = !b2Body_IsValid(body);

        if (missingComponents || deadBody)
        {
            if (!deadBody)
            {
                b2DestroyBody(body);
            }
            m_fixedCallbacks.erase(entity);
            it = m_entityBodyMap.erase(it);
            continue;
        }

        ++it;
    }
}

b2BodyId S2DPhysics::getBody(Entity entity) const
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

void S2DPhysics::setFixedUpdateCallback(Entity entity, std::function<void(float)> callback)
{
    if (!entity.isValid())
    {
        return;
    }

    if (!callback)
    {
        m_fixedCallbacks.erase(entity);
        return;
    }

    m_fixedCallbacks[entity] = std::move(callback);
}

void S2DPhysics::clearFixedUpdateCallback(Entity entity)
{
    m_fixedCallbacks.erase(entity);
}

void S2DPhysics::syncFromTransform(Entity entity, const ::Components::CTransform& transform)
{
    const b2BodyId bodyId = getBody(entity);
    if (!b2Body_IsValid(bodyId))
    {
        return;
    }

    b2Body_SetTransform(bodyId, {transform.position.x, transform.position.y}, b2MakeRot(transform.rotation));
    b2Body_SetLinearVelocity(bodyId, {transform.velocity.x, transform.velocity.y});
}

void S2DPhysics::syncToTransform(Entity entity, ::Components::CTransform& transform)
{
    const b2BodyId bodyId = getBody(entity);
    if (!b2Body_IsValid(bodyId))
    {
        return;
    }

    b2Vec2 pos   = b2Body_GetPosition(bodyId);
    b2Rot  rot   = b2Body_GetRotation(bodyId);
    float  angle = b2Rot_GetAngle(rot);
    b2Vec2 vel   = b2Body_GetLinearVelocity(bodyId);

    transform.position = {pos.x, pos.y};
    transform.rotation = angle;
    transform.velocity = {vel.x, vel.y};
}

void S2DPhysics::applyForce(Entity entity, const b2Vec2& force, const b2Vec2& point)
{
    const b2BodyId bodyId = getBody(entity);
    if (b2Body_IsValid(bodyId))
    {
        b2Body_ApplyForce(bodyId, force, point, true);
    }
}

void S2DPhysics::applyForceToCenter(Entity entity, const b2Vec2& force)
{
    const b2BodyId bodyId = getBody(entity);
    if (b2Body_IsValid(bodyId))
    {
        b2Body_ApplyForceToCenter(bodyId, force, true);
    }
}

void S2DPhysics::applyLinearImpulse(Entity entity, const b2Vec2& impulse, const b2Vec2& point)
{
    const b2BodyId bodyId = getBody(entity);
    if (b2Body_IsValid(bodyId))
    {
        b2Body_ApplyLinearImpulse(bodyId, impulse, point, true);
    }
}

void S2DPhysics::applyLinearImpulseToCenter(Entity entity, const b2Vec2& impulse)
{
    const b2BodyId bodyId = getBody(entity);
    if (b2Body_IsValid(bodyId))
    {
        b2Body_ApplyLinearImpulseToCenter(bodyId, impulse, true);
    }
}

void S2DPhysics::applyAngularImpulse(Entity entity, float impulse)
{
    const b2BodyId bodyId = getBody(entity);
    if (b2Body_IsValid(bodyId))
    {
        b2Body_ApplyAngularImpulse(bodyId, impulse, true);
    }
}

void S2DPhysics::applyTorque(Entity entity, float torque)
{
    const b2BodyId bodyId = getBody(entity);
    if (b2Body_IsValid(bodyId))
    {
        b2Body_ApplyTorque(bodyId, torque, true);
    }
}

void S2DPhysics::setLinearVelocity(Entity entity, const b2Vec2& velocity)
{
    const b2BodyId bodyId = getBody(entity);
    if (b2Body_IsValid(bodyId))
    {
        b2Body_SetLinearVelocity(bodyId, velocity);
    }
}

void S2DPhysics::setAngularVelocity(Entity entity, float omega)
{
    const b2BodyId bodyId = getBody(entity);
    if (b2Body_IsValid(bodyId))
    {
        b2Body_SetAngularVelocity(bodyId, omega);
    }
}

b2Vec2 S2DPhysics::getLinearVelocity(Entity entity) const
{
    const b2BodyId bodyId = getBody(entity);
    if (b2Body_IsValid(bodyId))
    {
        return b2Body_GetLinearVelocity(bodyId);
    }
    return {0.0f, 0.0f};
}

float S2DPhysics::getAngularVelocity(Entity entity) const
{
    const b2BodyId bodyId = getBody(entity);
    if (b2Body_IsValid(bodyId))
    {
        return b2Body_GetAngularVelocity(bodyId);
    }
    return 0.0f;
}

b2Vec2 S2DPhysics::getPosition(Entity entity) const
{
    const b2BodyId bodyId = getBody(entity);
    if (b2Body_IsValid(bodyId))
    {
        return b2Body_GetPosition(bodyId);
    }
    return {0.0f, 0.0f};
}

float S2DPhysics::getRotation(Entity entity) const
{
    const b2BodyId bodyId = getBody(entity);
    if (b2Body_IsValid(bodyId))
    {
        b2Rot rot = b2Body_GetRotation(bodyId);
        return b2Rot_GetAngle(rot);
    }
    return 0.0f;
}

b2Vec2 S2DPhysics::getForwardVector(Entity entity) const
{
    const b2BodyId bodyId = getBody(entity);
    if (b2Body_IsValid(bodyId))
    {
        b2Rot rot = b2Body_GetRotation(bodyId);
        return b2Rot_GetYAxis(rot);
    }
    return {0.0f, 1.0f};
}

b2Vec2 S2DPhysics::getRightVector(Entity entity) const
{
    const b2BodyId bodyId = getBody(entity);
    if (b2Body_IsValid(bodyId))
    {
        b2Rot rot = b2Body_GetRotation(bodyId);
        return b2Rot_GetXAxis(rot);
    }
    return {1.0f, 0.0f};
}

void S2DPhysics::runFixedUpdates(float timeStep)
{
    for (auto it = m_fixedCallbacks.begin(); it != m_fixedCallbacks.end();)
    {
        const Entity entity = it->first;
        const b2BodyId body = getBody(entity);
        if (!b2Body_IsValid(body))
        {
            it = m_fixedCallbacks.erase(it);
            continue;
        }

        if (it->second)
        {
            it->second(timeStep);
        }
        ++it;
    }
}

}  // namespace Systems
