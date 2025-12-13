#include "BarrelEntity.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace Components;
using namespace Entity;

namespace
{
constexpr float kBarrelRadius              = 0.1f;
constexpr float kBarrelColliderDensity     = 0.5f;
constexpr float kBarrelColliderFriction    = 0.3f;
constexpr float kBarrelColliderRestitution = 0.0f;
constexpr float kBarrelDensity             = 1.0f;
constexpr float kBarrelFriction            = 0.3f;
constexpr float kBarrelRestitution         = 0.15f;
constexpr float kBarrelLinearDamping       = 1.5f;
constexpr float kBarrelAngularDamping      = 2.0f;
constexpr float kBarrelGravityScale        = 1.0f;
constexpr int   kBarrelZIndex              = 10;
constexpr int   kBarrelSprayZIndex         = 9;

// Barrel spray emitter defaults (matches JSON)
const Vec2  kSprayDirection(0.0f, 1.0f);
const float kSpraySpread       = 0.5f;
const float kSprayMinSpeed     = 0.15f;
const float kSprayMaxSpeed     = 0.5f;
const float kSprayMinLifetime  = 0.5f;
const float kSprayMaxLifetime  = 2.0f;
const float kSprayMinSize      = 0.006f;
const float kSprayMaxSize      = 0.02f;
const float kSprayEmissionRate = 0.0f;
const float kSprayStartAlpha   = 0.9f;
const float kSprayEndAlpha     = 0.0f;
const int   kSprayMaxParticles = 1250;
const float kSprayShrinkEnd    = 0.1f;

void configurePhysics(CPhysicsBody2D* body, const Vec2& position)
{
    if (!body)
        return;
    body->setBodyType(BodyType::Dynamic);
    body->setDensity(kBarrelDensity);
    body->setFriction(kBarrelFriction);
    body->setRestitution(kBarrelRestitution);
    body->setFixedRotation(false);
    body->setLinearDamping(kBarrelLinearDamping);
    body->setAngularDamping(kBarrelAngularDamping);
    body->setGravityScale(kBarrelGravityScale);
    body->initialize(b2Vec2{position.x, position.y}, BodyType::Dynamic);
}

void configureCollider(CCollider2D* collider)
{
    if (!collider)
        return;
    collider->setIsSensor(false);
    collider->setDensity(kBarrelColliderDensity);
    collider->setFriction(kBarrelColliderFriction);
    collider->setRestitution(kBarrelColliderRestitution);
    collider->createCircle(kBarrelRadius, b2Vec2{0.0f, 0.0f});
}

void configureEmitter(CParticleEmitter* emitter, sf::Texture* texture)
{
    if (!emitter)
        return;
    emitter->setDirection(kSprayDirection);
    emitter->setSpreadAngle(kSpraySpread);
    emitter->setMinSpeed(kSprayMinSpeed);
    emitter->setMaxSpeed(kSprayMaxSpeed);
    emitter->setMinLifetime(kSprayMinLifetime);
    emitter->setMaxLifetime(kSprayMaxLifetime);
    emitter->setMinSize(kSprayMinSize);
    emitter->setMaxSize(kSprayMaxSize);
    emitter->setEmissionRate(kSprayEmissionRate);
    emitter->setStartColor(Color(220, 240, 255, 255));
    emitter->setEndColor(Color(255, 255, 255, 255));
    emitter->setStartAlpha(kSprayStartAlpha);
    emitter->setEndAlpha(kSprayEndAlpha);
    emitter->setGravity(Vec2(0.0f, 0.0f));
    emitter->setMinRotationSpeed(-3.0f);
    emitter->setMaxRotationSpeed(3.0f);
    emitter->setFadeOut(true);
    emitter->setShrink(true);
    emitter->setShrinkEndScale(kSprayShrinkEnd);
    emitter->setMaxParticles(kSprayMaxParticles);
    emitter->setZIndex(kBarrelSprayZIndex);
    emitter->setEmissionShape(EmissionShape::Circle);
    emitter->setShapeRadius(kBarrelRadius);
    emitter->setEmitFromEdge(true);
    emitter->setEmitOutward(true);
    emitter->setLineStart(Vec2(-0.5f, 0.0f));
    emitter->setLineEnd(Vec2(0.5f, 0.0f));
    if (texture)
    {
        texture->setSmooth(true);
        emitter->setTexture(texture);
    }
}

}  // namespace

Barrel::Barrel(const std::string& tag, size_t id, const Vec2& position) : Entity(tag, id), m_spawnPosition(position)
{
    // Load spray texture internally
    if (!m_sprayTexture.loadFromFile("assets/textures/bubble.png"))
    {
        std::cout << "Warning: Failed to load spray texture for Barrel" << std::endl;
    }
    else
    {
        m_sprayTexture.setSmooth(true);
    }
}

void Barrel::init()
{
    configureBarrel();
}

void Barrel::configureBarrel()
{
    m_transform   = addComponent<CTransform>(m_spawnPosition, Vec2(1.0f, 1.0f), 0.0f);
    auto* texture = addComponent<CTexture>("assets/textures/barrel.png");
    texture->setSmooth(true);
    addComponent<CRenderable>(VisualType::Sprite, Color::White, kBarrelZIndex, true);
    addComponent<CMaterial>(Color::White, BlendMode::Alpha, 1.0f);

    m_physics = addComponent<CPhysicsBody2D>();
    configurePhysics(m_physics, m_spawnPosition);

    auto* collider = addComponent<CCollider2D>();
    configureCollider(collider);

    m_emitter = addComponent<CParticleEmitter>();
    configureEmitter(m_emitter, &m_sprayTexture);
}

void Barrel::updateSprayForSpeed(float speed)
{
    if (!m_emitter)
        return;

    const float MIN_SPEED_FOR_SPRAY = 0.05f;    // Start spraying at this speed (m/s)
    const float MAX_SPEED_FOR_SPRAY = 2.0f;     // Maximum spray at this speed (m/s)
    const float MIN_EMISSION_RATE   = 0.0f;     // Emission rate at minimum speed
    const float MAX_EMISSION_RATE   = 1250.0f;  // Emission rate at maximum speed

    float emissionRate = 0.0f;
    if (speed > MIN_SPEED_FOR_SPRAY)
    {
        float normalizedSpeed = (speed - MIN_SPEED_FOR_SPRAY) / (MAX_SPEED_FOR_SPRAY - MIN_SPEED_FOR_SPRAY);
        normalizedSpeed       = std::min(1.0f, std::max(0.0f, normalizedSpeed));

        emissionRate = MIN_EMISSION_RATE + (MAX_EMISSION_RATE - MIN_EMISSION_RATE) * (normalizedSpeed * normalizedSpeed);

        float speedMultiplier = 0.5f + (normalizedSpeed * 0.5f);
        m_emitter->setMinSpeed(0.15f * speedMultiplier);
        m_emitter->setMaxSpeed(0.5f * speedMultiplier);
    }

    m_emitter->setEmissionRate(emissionRate);
}

void Barrel::update(float /*deltaTime*/)
{
    if (!m_physics || !m_physics->isInitialized())
        return;

    b2Vec2 velocity = m_physics->getLinearVelocity();
    float  speed    = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);

    updateSprayForSpeed(speed);
}
