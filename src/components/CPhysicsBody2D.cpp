#include "CPhysicsBody2D.h"
#include "CTransform.h"
#include "S2DPhysics.h"
#include "SystemLocator.h"
#include "Vec2.h"

namespace Components
{

CPhysicsBody2D::CPhysicsBody2D()
    : m_bodyId(b2_nullBodyId),
      m_bodyType(BodyType::Dynamic),
      m_density(1.0f),
      m_friction(0.3f),
      m_restitution(0.15f),
      m_fixedRotation(false),
      m_linearDamping(0.25f),
      m_angularDamping(0.10f),
      m_gravityScale(1.0f),
    m_entity(Entity::null()),
      m_initialized(false)
{
}

CPhysicsBody2D::~CPhysicsBody2D()
{
    if (m_initialized)
    {
        ::Systems::SystemLocator::physics().unregisterBody(this);

        if (b2Body_IsValid(m_bodyId))
        {
            if (m_entity.isValid())
            {
                ::Systems::SystemLocator::physics().destroyBody(m_entity);
            }
            else
            {
                b2DestroyBody(m_bodyId);
            }
        }

        m_bodyId      = b2_nullBodyId;
        m_initialized = false;
    }
}

void CPhysicsBody2D::initialize(const b2Vec2& position, BodyType type)
{
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        if (m_entity.isValid())
        {
            ::Systems::SystemLocator::physics().destroyBody(m_entity);
        }
        else
        {
            b2DestroyBody(m_bodyId);
        }
        m_bodyId      = b2_nullBodyId;
        m_initialized = false;
    }

    m_bodyType = type;

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.position  = position;
    // Note: fixedRotation is set after body creation in Box2D v3
    bodyDef.linearDamping  = m_linearDamping;
    bodyDef.angularDamping = m_angularDamping;
    bodyDef.gravityScale   = m_gravityScale;

    switch (type)
    {
        case BodyType::Static:
            bodyDef.type = b2_staticBody;
            break;
        case BodyType::Kinematic:
            bodyDef.type = b2_kinematicBody;
            break;
        case BodyType::Dynamic:
            bodyDef.type = b2_dynamicBody;
            break;
    }

    m_bodyId      = ::Systems::SystemLocator::physics().createBody(m_entity, bodyDef);
    m_initialized = b2Body_IsValid(m_bodyId);

    // Apply persisted properties after body creation
    if (b2Body_IsValid(m_bodyId))
    {
        if (m_fixedRotation)
        {
            b2Body_SetFixedRotation(m_bodyId, true);
        }
    }

    // Auto-register this body for fixed-update callbacks
    ::Systems::SystemLocator::physics().registerBody(this);
}

void CPhysicsBody2D::setBodyType(BodyType type)
{
    m_bodyType = type;
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        switch (type)
        {
            case BodyType::Static:
                b2Body_SetType(m_bodyId, b2_staticBody);
                break;
            case BodyType::Kinematic:
                b2Body_SetType(m_bodyId, b2_kinematicBody);
                break;
            case BodyType::Dynamic:
                b2Body_SetType(m_bodyId, b2_dynamicBody);
                break;
        }
    }
}

void CPhysicsBody2D::setFixedRotation(bool fixed)
{
    m_fixedRotation = fixed;
    if (b2Body_IsValid(m_bodyId))
    {
        b2Body_SetFixedRotation(m_bodyId, fixed);
    }
}

void CPhysicsBody2D::setLinearDamping(float damping)
{
    m_linearDamping = damping;
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        b2Body_SetLinearDamping(m_bodyId, damping);
    }
}

void CPhysicsBody2D::setAngularDamping(float damping)
{
    m_angularDamping = damping;
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        b2Body_SetAngularDamping(m_bodyId, damping);
    }
}

void CPhysicsBody2D::setGravityScale(float scale)
{
    m_gravityScale = scale;
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        b2Body_SetGravityScale(m_bodyId, scale);
    }
}

void CPhysicsBody2D::applyForce(const b2Vec2& force, const b2Vec2& point)
{
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        b2Body_ApplyForce(m_bodyId, force, point, true);
    }
}

void CPhysicsBody2D::applyForceToCenter(const b2Vec2& force)
{
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        b2Body_ApplyForceToCenter(m_bodyId, force, true);
    }
}

void CPhysicsBody2D::applyLinearImpulse(const b2Vec2& impulse, const b2Vec2& point)
{
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        b2Body_ApplyLinearImpulse(m_bodyId, impulse, point, true);
    }
}

void CPhysicsBody2D::applyLinearImpulseToCenter(const b2Vec2& impulse)
{
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        b2Body_ApplyLinearImpulseToCenter(m_bodyId, impulse, true);
    }
}

void CPhysicsBody2D::applyAngularImpulse(float impulse)
{
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        b2Body_ApplyAngularImpulse(m_bodyId, impulse, true);
    }
}

void CPhysicsBody2D::applyTorque(float torque)
{
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        b2Body_ApplyTorque(m_bodyId, torque, true);
    }
}

void CPhysicsBody2D::setLinearVelocity(const b2Vec2& velocity)
{
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        b2Body_SetLinearVelocity(m_bodyId, velocity);
    }
}

b2Vec2 CPhysicsBody2D::getLinearVelocity() const
{
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        return b2Body_GetLinearVelocity(m_bodyId);
    }
    return {0.0f, 0.0f};
}

void CPhysicsBody2D::setAngularVelocity(float omega)
{
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        b2Body_SetAngularVelocity(m_bodyId, omega);
    }
}

float CPhysicsBody2D::getAngularVelocity() const
{
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        return b2Body_GetAngularVelocity(m_bodyId);
    }
    return 0.0f;
}

b2Vec2 CPhysicsBody2D::getPosition() const
{
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        return b2Body_GetPosition(m_bodyId);
    }
    return {0.0f, 0.0f};
}

float CPhysicsBody2D::getRotation() const
{
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        b2Rot rot = b2Body_GetRotation(m_bodyId);
        return b2Rot_GetAngle(rot);
    }
    return 0.0f;
}

b2Vec2 CPhysicsBody2D::getForwardVector() const
{
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        b2Rot rot = b2Body_GetRotation(m_bodyId);
        return b2Rot_GetYAxis(rot);  // Y-axis is forward in Box2D
    }
    return {0.0f, 1.0f};  // Default forward is up
}

b2Vec2 CPhysicsBody2D::getRightVector() const
{
    if (m_initialized && b2Body_IsValid(m_bodyId))
    {
        b2Rot rot = b2Body_GetRotation(m_bodyId);
        return b2Rot_GetXAxis(rot);  // X-axis is right in Box2D
    }
    return {1.0f, 0.0f};  // Default right is to the right
}

void CPhysicsBody2D::syncToTransform(CTransform* transform)
{
    if (!m_initialized || !b2Body_IsValid(m_bodyId) || !transform)
    {
        return;
    }

    b2Vec2 pos   = b2Body_GetPosition(m_bodyId);
    b2Rot  rot   = b2Body_GetRotation(m_bodyId);
    float  angle = b2Rot_GetAngle(rot);
    b2Vec2 vel   = b2Body_GetLinearVelocity(m_bodyId);

    transform->setPosition(Vec2(pos.x, pos.y));
    transform->setRotation(angle);
    transform->setVelocity(Vec2(vel.x, vel.y));
}

void CPhysicsBody2D::syncFromTransform(const CTransform* transform)
{
    if (!m_initialized || !b2Body_IsValid(m_bodyId) || !transform)
    {
        return;
    }

    Vec2  pos   = transform->getPosition();
    float angle = transform->getRotation();
    Vec2  vel   = transform->getVelocity();

    b2Body_SetTransform(m_bodyId, {pos.x, pos.y}, b2MakeRot(angle));
    b2Body_SetLinearVelocity(m_bodyId, {vel.x, vel.y});
}

void CPhysicsBody2D::init() {}

void CPhysicsBody2D::setEntity(Entity entity)
{
    m_entity = entity;
}

}  // namespace Components
