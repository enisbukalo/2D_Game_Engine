#include "../include/components/CRigidBody2D.h"
#include <JsonBuilder.h>
#include <JsonValue.h>

CRigidBody2D::CRigidBody2D()
    : m_mass(1.0f),
      m_inverseMass(1.0f)  // 1/1.0 = 1.0
      ,
      m_restitution(0.5f)  // Medium bounce (wood/plastic feel)
      ,
      m_friction(0.3f)  // Medium friction
      ,
      m_linearDrag(0.25f)  // Moderate air resistance (25% per second)
      ,
      m_angularDrag(0.05f)  // Slight rotational dampening
      ,
      m_useGravity(true)  // Most objects affected by gravity
      ,
      m_gravityScale(1.0f)  // Normal gravity strength
      ,
      m_isKinematic(false)  // Most objects are dynamic
      ,
      m_freezePositionX(false),
      m_freezePositionY(false),
      m_freezeRotation(false),
      m_accumulatedForce(0.0f, 0.0f),
      m_totalForce(0.0f, 0.0f)
{
}

void CRigidBody2D::serialize(JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cRigidBody2D");
    builder.beginObject();

    builder.addKey("mass");
    builder.addNumber(m_mass);
    builder.addKey("restitution");
    builder.addNumber(m_restitution);
    builder.addKey("friction");
    builder.addNumber(m_friction);
    builder.addKey("linearDrag");
    builder.addNumber(m_linearDrag);
    builder.addKey("angularDrag");
    builder.addNumber(m_angularDrag);
    builder.addKey("useGravity");
    builder.addBool(m_useGravity);
    builder.addKey("gravityScale");
    builder.addNumber(m_gravityScale);
    builder.addKey("isKinematic");
    builder.addBool(m_isKinematic);
    builder.addKey("freezePositionX");
    builder.addBool(m_freezePositionX);
    builder.addKey("freezePositionY");
    builder.addBool(m_freezePositionY);
    builder.addKey("freezeRotation");
    builder.addBool(m_freezeRotation);

    builder.endObject();
    builder.endObject();
}

void CRigidBody2D::deserialize(const JsonValue& value)
{
    const auto& rb = value["cRigidBody2D"];

    m_mass            = static_cast<float>(rb["mass"].getNumber());
    m_restitution     = static_cast<float>(rb["restitution"].getNumber());
    m_friction        = static_cast<float>(rb["friction"].getNumber());
    m_linearDrag      = static_cast<float>(rb["linearDrag"].getNumber());
    m_angularDrag     = static_cast<float>(rb["angularDrag"].getNumber());
    m_useGravity      = rb["useGravity"].getBool();
    m_gravityScale    = static_cast<float>(rb["gravityScale"].getNumber());
    m_isKinematic     = rb["isKinematic"].getBool();
    m_freezePositionX = rb["freezePositionX"].getBool();
    m_freezePositionY = rb["freezePositionY"].getBool();
    m_freezeRotation  = rb["freezeRotation"].getBool();

    // Recalculate inverse mass after deserialization
    if (m_mass > 0.0f)
    {
        m_inverseMass = 1.0f / m_mass;
    }
    else
    {
        m_inverseMass = 0.0f;  // Infinite mass (immovable)
    }
}

void CRigidBody2D::addForce(const Vec2& force)
{
    if (m_isKinematic)
    {
        return;  // Kinematic bodies don't respond to forces
    }
    m_accumulatedForce = m_accumulatedForce + force;
}

void CRigidBody2D::addImpulse(const Vec2& impulse)
{
    if (m_isKinematic)
    {
        return;  // Kinematic bodies don't respond to impulses
    }
    // Impulses are applied immediately to velocity by the physics system
    // This method can be used for instant velocity changes
    m_accumulatedForce = m_accumulatedForce + impulse;
}

void CRigidBody2D::clearForces()
{
    m_totalForce       = m_accumulatedForce;  // Save for visualization
    m_accumulatedForce = Vec2(0.0f, 0.0f);
}

void CRigidBody2D::setMass(float mass)
{
    if (mass < 0.0f)
    {
        mass = 0.0f;  // Clamp to zero minimum
    }

    m_mass = mass;

    if (mass > 0.0f)
    {
        m_inverseMass = 1.0f / mass;
    }
    else
    {
        m_inverseMass = 0.0f;  // Infinite mass (immovable object)
    }
}
