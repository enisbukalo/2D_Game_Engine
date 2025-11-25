#include "CPhysicsBody2D.h"
#include "CTransform.h"
#include "SBox2DPhysics.h"
#include "Entity.h"
#include "Vec2.h"

CPhysicsBody2D::CPhysicsBody2D()
    : m_bodyId(b2_nullBodyId)
    , m_bodyType(BodyType::Dynamic)
    , m_density(1.0f)
    , m_friction(0.3f)
    , m_restitution(0.0f)
    , m_fixedRotation(false)
    , m_linearDamping(0.0f)
    , m_angularDamping(0.0f)
    , m_gravityScale(1.0f)
    , m_initialized(false)
{
}

CPhysicsBody2D::~CPhysicsBody2D() {
    if (m_initialized && b2Body_IsValid(m_bodyId) && getOwner()) {
        SBox2DPhysics::instance().destroyBody(getOwner());
        m_bodyId = b2_nullBodyId;
        m_initialized = false;
    }
}

void CPhysicsBody2D::initialize(const b2Vec2& position, BodyType type) {
    if (m_initialized) {
        // Already initialized, destroy old body first
        if (b2Body_IsValid(m_bodyId) && getOwner()) {
            SBox2DPhysics::instance().destroyBody(getOwner());
        }
    }

    m_bodyType = type;

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.position = position;
    // Note: fixedRotation is set after body creation in Box2D v3
    bodyDef.linearDamping = m_linearDamping;
    bodyDef.angularDamping = m_angularDamping;
    bodyDef.gravityScale = m_gravityScale;

    switch (type) {
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

    m_bodyId = SBox2DPhysics::instance().createBody(getOwner(), bodyDef);
    m_initialized = true;

    // TODO: Box2D v3 doesn't support fixedRotation in the same way as v2
    // This will need to be implemented differently or removed
}

void CPhysicsBody2D::setBodyType(BodyType type) {
    m_bodyType = type;
    if (m_initialized && b2Body_IsValid(m_bodyId)) {
        switch (type) {
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

void CPhysicsBody2D::setFixedRotation(bool fixed) {
    m_fixedRotation = fixed;
    // TODO: Box2D v3 doesn't support fixedRotation in the same way as v2
    // This will need to be implemented differently or removed
}

void CPhysicsBody2D::setLinearDamping(float damping) {
    m_linearDamping = damping;
    if (m_initialized && b2Body_IsValid(m_bodyId)) {
        b2Body_SetLinearDamping(m_bodyId, damping);
    }
}

void CPhysicsBody2D::setAngularDamping(float damping) {
    m_angularDamping = damping;
    if (m_initialized && b2Body_IsValid(m_bodyId)) {
        b2Body_SetAngularDamping(m_bodyId, damping);
    }
}

void CPhysicsBody2D::setGravityScale(float scale) {
    m_gravityScale = scale;
    if (m_initialized && b2Body_IsValid(m_bodyId)) {
        b2Body_SetGravityScale(m_bodyId, scale);
    }
}

void CPhysicsBody2D::applyForce(const b2Vec2& force, const b2Vec2& point) {
    if (m_initialized && b2Body_IsValid(m_bodyId)) {
        b2Body_ApplyForce(m_bodyId, force, point, true);
    }
}

void CPhysicsBody2D::applyForceToCenter(const b2Vec2& force) {
    if (m_initialized && b2Body_IsValid(m_bodyId)) {
        b2Body_ApplyForceToCenter(m_bodyId, force, true);
    }
}

void CPhysicsBody2D::applyLinearImpulse(const b2Vec2& impulse, const b2Vec2& point) {
    if (m_initialized && b2Body_IsValid(m_bodyId)) {
        b2Body_ApplyLinearImpulse(m_bodyId, impulse, point, true);
    }
}

void CPhysicsBody2D::applyLinearImpulseToCenter(const b2Vec2& impulse) {
    if (m_initialized && b2Body_IsValid(m_bodyId)) {
        b2Body_ApplyLinearImpulseToCenter(m_bodyId, impulse, true);
    }
}

void CPhysicsBody2D::applyAngularImpulse(float impulse) {
    if (m_initialized && b2Body_IsValid(m_bodyId)) {
        b2Body_ApplyAngularImpulse(m_bodyId, impulse, true);
    }
}

void CPhysicsBody2D::setLinearVelocity(const b2Vec2& velocity) {
    if (m_initialized && b2Body_IsValid(m_bodyId)) {
        b2Body_SetLinearVelocity(m_bodyId, velocity);
    }
}

b2Vec2 CPhysicsBody2D::getLinearVelocity() const {
    if (m_initialized && b2Body_IsValid(m_bodyId)) {
        return b2Body_GetLinearVelocity(m_bodyId);
    }
    return {0.0f, 0.0f};
}

void CPhysicsBody2D::setAngularVelocity(float omega) {
    if (m_initialized && b2Body_IsValid(m_bodyId)) {
        b2Body_SetAngularVelocity(m_bodyId, omega);
    }
}

float CPhysicsBody2D::getAngularVelocity() const {
    if (m_initialized && b2Body_IsValid(m_bodyId)) {
        return b2Body_GetAngularVelocity(m_bodyId);
    }
    return 0.0f;
}

b2Vec2 CPhysicsBody2D::getPosition() const {
    if (m_initialized && b2Body_IsValid(m_bodyId)) {
        return b2Body_GetPosition(m_bodyId);
    }
    return {0.0f, 0.0f};
}

float CPhysicsBody2D::getRotation() const {
    if (m_initialized && b2Body_IsValid(m_bodyId)) {
        b2Rot rot = b2Body_GetRotation(m_bodyId);
        return b2Rot_GetAngle(rot);
    }
    return 0.0f;
}

void CPhysicsBody2D::syncToTransform(CTransform* transform) {
    if (!m_initialized || !b2Body_IsValid(m_bodyId) || !transform) {
        return;
    }

    b2Vec2 pos = b2Body_GetPosition(m_bodyId);
    b2Rot rot = b2Body_GetRotation(m_bodyId);
    float angle = b2Rot_GetAngle(rot);
    b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);

    transform->setPosition(Vec2(pos.x, pos.y));
    transform->setRotation(angle);
    transform->setVelocity(Vec2(vel.x, vel.y));
}

void CPhysicsBody2D::syncFromTransform(const CTransform* transform) {
    if (!m_initialized || !b2Body_IsValid(m_bodyId) || !transform) {
        return;
    }

    Vec2 pos = transform->getPosition();
    float angle = transform->getRotation();
    Vec2 vel = transform->getVelocity();

    b2Body_SetTransform(m_bodyId, {pos.x, pos.y}, b2MakeRot(angle));
    b2Body_SetLinearVelocity(m_bodyId, {vel.x, vel.y});
}

void CPhysicsBody2D::serialize(JsonBuilder& builder) const {
    // TODO: Implement full JSON serialization
    builder.beginObject();
    builder.addKey("cPhysicsBody2D");
    builder.beginObject();
    builder.endObject();
    builder.endObject();
}

void CPhysicsBody2D::deserialize(const JsonValue& value) {
    // TODO: Implement JSON deserialization
}

std::string CPhysicsBody2D::getType() const {
    return "CPhysicsBody2D";
}
