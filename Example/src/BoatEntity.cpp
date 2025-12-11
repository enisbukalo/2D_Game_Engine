#include "BoatEntity.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>

namespace
{
constexpr float kBoatPosX = 9.20209f;
constexpr float kBoatPosY = 7.90827f;
constexpr float kBoatRot  = 1.73084f;

// Movement and steering constants
constexpr float kPlayerForce           = 5.0f;   // Force applied for player movement
constexpr float kPlayerTurningForce    = 0.5f;   // Base torque/force multiplier for player rotation
constexpr float kRudderOffsetMeters    = 0.35f;  // Distance from center to stern (meters) where rudder force is applied
constexpr float kRudderForceMultiplier = 1.0f;   // Multiplier for lateral rudder force
constexpr float kRudderSmoothK       = 0.18f;  // Smooth parameter to scale rudder effectiveness with speed (soft clamp)
constexpr float kMinSpeedForSteering = 0.15f;  // Minimum speed (m/s) required for steering effectiveness (coasting)
constexpr float kRudderMinEffectiveScale = 0.025f;  // Minimum rudder effect scale applied at MIN_SPEED_FOR_STEERING

// Motor audio constants
constexpr float kMotorFadeDuration = 2.0f;   // 2 second fade-in & fade-out
constexpr float kMotorMaxVolume    = 0.45f;  // 45% max volume

constexpr float kBoatDensity             = 2.0f;
constexpr float kBoatFriction            = 0.3f;
constexpr float kBoatRestitution         = 0.15f;
constexpr float kBoatLinearDamping       = 0.75f;
constexpr float kBoatAngularDamping      = 0.75f;
constexpr float kBoatGravityScale        = 1.0f;
constexpr float kBoatColliderDensity     = 5.0f;
constexpr float kBoatColliderFriction    = 0.5f;
constexpr float kBoatColliderRestitution = 0.125f;

// Bubble trail emitter constants
const Vec2  kBubbleDirection(0.0f, -1.0f);  // Emit backward (stern direction in local space)
const float kBubbleSpread       = 1.2f;
const float kBubbleMinSpeed     = 0.05f;
const float kBubbleMaxSpeed     = 0.2f;
const float kBubbleMinLifetime  = 3.0f;
const float kBubbleMaxLifetime  = 3.0f;
const float kBubbleMinSize      = 0.005f;
const float kBubbleMaxSize      = 0.025f;
const float kBubbleEmissionRate = 300.0f;
const float kBubbleStartAlpha   = 1.0f;
const float kBubbleEndAlpha     = 0.5f;
const int   kBubbleMaxParticles = 1000;
const int   kBubbleZIndex       = 5;
const Vec2  kBubbleOffset(0.0f, -0.65625f);

// Hull spray emitter constants
const Vec2      kSprayDirection(0.0f, 1.0f);  // Emit forward (bow direction in local space)
const float     kSpraySpread           = 0.4f;
constexpr float kSprayMinSpeed         = 0.122925f;
constexpr float kSprayMaxSpeed         = 0.4917f;
constexpr float kSprayMinLifetime      = 0.5f;
constexpr float kSprayMaxLifetime      = 2.4f;
constexpr float kSprayMinSize          = 0.006f;
constexpr float kSprayMaxSize          = 0.02f;
constexpr float kSprayEmissionRate     = 938.808f;
constexpr float kSprayStartAlpha       = 0.9f;
constexpr float kSprayEndAlpha         = 0.0f;
constexpr float kSprayMinRotationSpeed = -3.0f;
constexpr float kSprayMaxRotationSpeed = 3.0f;
constexpr float kSprayShrinkEnd        = 0.1f;
constexpr int   kSprayMaxParticles     = 7500;
constexpr int   kSprayZIndex           = 5;

// Boat hull polygon (matches JSON fixtures for collider and spray polygon)
const std::vector<std::vector<b2Vec2>> kBoatHullFixtures = {
    {{0.225f, 0.0f}, {-0.225f, 0.0f}, {-0.225f, -0.0875f}, {-0.1575f, -0.39375f}, {0.1575f, -0.39375f}, {0.225f, -0.0875f}},
    {{-0.225f, 0.0f}, {0.225f, 0.0f}, {0.223438f, 0.0401042f}, {-0.223438f, 0.0401042f}},
    {{-0.223438f, 0.0401042f}, {0.223438f, 0.0401042f}, {0.21875f, 0.0802083f}, {-0.21875f, 0.0802083f}},
    {{-0.21875f, 0.0802083f}, {0.21875f, 0.0802083f}, {0.210938f, 0.120313f}, {-0.210938f, 0.120313f}},
    {{-0.210938f, 0.120313f}, {0.210938f, 0.120313f}, {0.2f, 0.160417f}, {-0.2f, 0.160417f}},
    {{-0.2f, 0.160417f}, {0.2f, 0.160417f}, {0.185937f, 0.200521f}, {-0.185937f, 0.200521f}},
    {{-0.185937f, 0.200521f}, {0.185937f, 0.200521f}, {0.16875f, 0.240625f}, {-0.16875f, 0.240625f}},
    {{-0.16875f, 0.240625f}, {0.16875f, 0.240625f}, {0.148438f, 0.280729f}, {-0.148438f, 0.280729f}},
    {{-0.148438f, 0.280729f}, {0.148438f, 0.280729f}, {0.125f, 0.320833f}, {-0.125f, 0.320833f}},
    {{-0.125f, 0.320833f}, {0.125f, 0.320833f}, {0.0984375f, 0.360938f}, {-0.0984375f, 0.360938f}},
    {{-0.0984375f, 0.360938f}, {0.0984375f, 0.360938f}, {0.06875f, 0.401042f}, {-0.06875f, 0.401042f}},
    {{-0.06875f, 0.401042f}, {0.06875f, 0.401042f}, {0.0359375f, 0.441146f}, {-0.0359375f, 0.441146f}},
    {{-0.0359375f, 0.441146f}, {0.0359375f, 0.441146f}, {0.0f, 0.48125f}}};

const std::vector<Vec2> kHullSprayPolygon = {
    {-0.1575f, -0.39375f},    {0.1575f, -0.39375f},    {0.225f, -0.0875f},       {0.225f, 0.0f},
    {0.223438f, 0.0401042f},  {0.21875f, 0.0802083f},  {0.210938f, 0.120313f},   {0.2f, 0.160417f},
    {0.185937f, 0.200521f},   {0.16875f, 0.240625f},   {0.148438f, 0.280729f},   {0.125f, 0.320833f},
    {0.0984375f, 0.360938f},  {0.06875f, 0.401042f},   {0.0359375f, 0.441146f},  {0.0f, 0.48125f},
    {-0.0359375f, 0.441146f}, {-0.06875f, 0.401042f},  {-0.0984375f, 0.360938f}, {-0.125f, 0.320833f},
    {-0.148438f, 0.280729f},  {-0.16875f, 0.240625f},  {-0.185937f, 0.200521f},  {-0.2f, 0.160417f},
    {-0.210938f, 0.120313f},  {-0.21875f, 0.0802083f}, {-0.223438f, 0.0401042f}, {-0.225f, 0.0f},
    {-0.225f, -0.0875f}};

void addHullCollider(Components::CCollider2D* collider)
{
    if (!collider)
        return;

    collider->setIsSensor(false);
    collider->setDensity(kBoatColliderDensity);
    collider->setFriction(kBoatColliderFriction);
    collider->setRestitution(kBoatColliderRestitution);

    if (!kBoatHullFixtures.empty())
    {
        const auto& first = kBoatHullFixtures.front();
        collider->createPolygon(first.data(), static_cast<int>(first.size()), 0.02f);
        for (size_t i = 1; i < kBoatHullFixtures.size(); ++i)
        {
            const auto& fixture = kBoatHullFixtures[i];
            collider->addPolygon(fixture.data(), static_cast<int>(fixture.size()), 0.02f);
        }
    }
}

void configureBoatPhysics(Components::CPhysicsBody2D* body)
{
    if (!body)
        return;
    body->setBodyType(Components::BodyType::Dynamic);
    body->setDensity(kBoatDensity);
    body->setFriction(kBoatFriction);
    body->setRestitution(kBoatRestitution);
    body->setFixedRotation(false);
    body->setLinearDamping(kBoatLinearDamping);
    body->setAngularDamping(kBoatAngularDamping);
    body->setGravityScale(kBoatGravityScale);
}

void configureBubbleEmitter(Components::CParticleEmitter* emitter, sf::Texture* texture)
{
    if (!emitter)
        return;
    emitter->setDirection(kBubbleDirection);
    emitter->setSpreadAngle(kBubbleSpread);
    emitter->setMinSpeed(kBubbleMinSpeed);
    emitter->setMaxSpeed(kBubbleMaxSpeed);
    emitter->setMinLifetime(kBubbleMinLifetime);
    emitter->setMaxLifetime(kBubbleMaxLifetime);
    emitter->setMinSize(kBubbleMinSize);
    emitter->setMaxSize(kBubbleMaxSize);
    emitter->setEmissionRate(kBubbleEmissionRate);
    emitter->setStartAlpha(kBubbleStartAlpha);
    emitter->setEndAlpha(kBubbleEndAlpha);
    emitter->setGravity(Vec2(0.0f, 0.0f));
    emitter->setMaxParticles(kBubbleMaxParticles);
    emitter->setZIndex(kBubbleZIndex);
    emitter->setPositionOffset(kBubbleOffset);
    emitter->setEmissionShape(Components::EmissionShape::Point);
    emitter->setLineStart(Vec2(-0.5f, 0.0f));
    emitter->setLineEnd(Vec2(0.5f, 0.0f));
    emitter->setEmitFromEdge(true);
    emitter->setEmitOutward(false);
    if (texture)
    {
        texture->setSmooth(true);
        emitter->setTexture(texture);
    }
}

void configureHullSprayEmitter(Components::CParticleEmitter* emitter, sf::Texture* texture)
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
    emitter->setGravity(Vec2(0.0f, 0.0f));
    emitter->setStartAlpha(kSprayStartAlpha);
    emitter->setEndAlpha(kSprayEndAlpha);
    emitter->setMinRotationSpeed(kSprayMinRotationSpeed);
    emitter->setMaxRotationSpeed(kSprayMaxRotationSpeed);
    emitter->setFadeOut(true);
    emitter->setShrink(true);
    emitter->setShrinkEndScale(kSprayShrinkEnd);
    emitter->setMaxParticles(kSprayMaxParticles);
    emitter->setZIndex(kSprayZIndex);
    emitter->setEmissionShape(Components::EmissionShape::Polygon);
    emitter->setEmitFromEdge(true);
    emitter->setEmitOutward(true);
    emitter->setLineStart(Vec2(-0.5f, 0.0f));
    emitter->setLineEnd(Vec2(0.5f, 0.0f));
    emitter->setPolygonVertices(kHullSprayPolygon);
    if (texture)
    {
        texture->setSmooth(true);
        emitter->setTexture(texture);
    }
}

}  // namespace

Boat::Boat(const std::string& tag, size_t id, Systems::SInput* inputManager, Systems::SAudio* audioSystem)
    : Entity(tag, id), m_inputManager(inputManager), m_audioSystem(audioSystem), m_motorBoatHandle(AudioHandle::invalid())
{
    // Load textures internally
    if (!m_bubbleTexture.loadFromFile("assets/textures/bubble.png"))
    {
        std::cout << "Warning: Failed to load bubble texture for Boat" << std::endl;
    }
    else
    {
        m_bubbleTexture.setSmooth(true);
    }

    if (!m_sprayTexture.loadFromFile("assets/textures/bubble.png"))
    {
        std::cout << "Warning: Failed to load spray texture for Boat" << std::endl;
    }
    else
    {
        m_sprayTexture.setSmooth(true);
    }
}

void Boat::init()
{
    configureBoatBody();
    configureBubbleTrail();
    configureHullSpray();
    bindInputCallbacks();
    setupFixedUpdate();
}

void Boat::configureBoatBody()
{
    m_transform   = addComponent<Components::CTransform>(Vec2(kBoatPosX, kBoatPosY), Vec2(1.0f, 1.0f), kBoatRot);
    auto* texture = addComponent<Components::CTexture>("assets/textures/boat.png");
    addComponent<Components::CRenderable>(Components::VisualType::Sprite, Color::White, 10, true);
    addComponent<Components::CMaterial>(Color::White, Components::BlendMode::Alpha, 1.0f);

    m_physicsBody = addComponent<Components::CPhysicsBody2D>();
    configureBoatPhysics(m_physicsBody);
    m_physicsBody->initialize(b2Vec2{kBoatPosX, kBoatPosY}, Components::BodyType::Dynamic);

    auto* collider = addComponent<Components::CCollider2D>();
    addHullCollider(collider);

    m_input = addComponent<Components::CInputController>();
}

void Boat::configureBubbleTrail()
{
    m_bubbleTrail = Systems::SEntity::instance().addEntity<::Entity::Entity>("bubble_trail");
    m_bubbleTrail->addComponent<Components::CTransform>(Vec2(kBoatPosX, kBoatPosY), Vec2(1.0f, 1.0f), kBoatRot);
    m_bubbleEmitter = m_bubbleTrail->addComponent<Components::CParticleEmitter>();
    configureBubbleEmitter(m_bubbleEmitter, &m_bubbleTexture);
}

void Boat::configureHullSpray()
{
    m_hullSpray = Systems::SEntity::instance().addEntity<::Entity::Entity>("hull_spray");
    m_hullSpray->addComponent<Components::CTransform>(Vec2(kBoatPosX, kBoatPosY), Vec2(1.0f, 1.0f), kBoatRot);
    m_hullEmitter = m_hullSpray->addComponent<Components::CParticleEmitter>();
    configureHullSprayEmitter(m_hullEmitter, &m_sprayTexture);
}

void Boat::syncEmittersToBoat()
{
    if (!m_transform)
        return;

    Vec2  pos      = m_transform->getPosition();
    float rotation = m_transform->getRotation();

    if (m_bubbleTrail && m_bubbleTrail->isAlive())
    {
        if (auto* t = m_bubbleTrail->getComponent<Components::CTransform>())
        {
            t->setPosition(pos);
            t->setRotation(rotation);
        }
    }

    if (m_hullSpray && m_hullSpray->isAlive())
    {
        if (auto* t = m_hullSpray->getComponent<Components::CTransform>())
        {
            t->setPosition(pos);
            t->setRotation(rotation);
        }
    }
}

void Boat::updateHullSprayForSpeed(float speed)
{
    if (!m_hullEmitter)
        return;

    // Define speed thresholds
    const float MIN_SPEED_FOR_SPRAY = 0.05f;    // Start spraying at this speed (m/s)
    const float MAX_SPEED_FOR_SPRAY = 2.0f;     // Maximum spray at this speed (m/s)
    const float MIN_EMISSION_RATE   = 0.0f;     // Emission rate at minimum speed
    const float MAX_EMISSION_RATE   = 5000.0f;  // Emission rate at maximum speed

    float emissionRate = 0.0f;
    if (speed > MIN_SPEED_FOR_SPRAY)
    {
        float normalizedSpeed = (speed - MIN_SPEED_FOR_SPRAY) / (MAX_SPEED_FOR_SPRAY - MIN_SPEED_FOR_SPRAY);
        normalizedSpeed       = std::min(1.0f, std::max(0.0f, normalizedSpeed));
        emissionRate = MIN_EMISSION_RATE + (MAX_EMISSION_RATE - MIN_EMISSION_RATE) * (normalizedSpeed * normalizedSpeed);

        float speedMultiplier = 1.0f + (speed / MAX_SPEED_FOR_SPRAY) * 0.5f;
        m_hullEmitter->setMinSpeed(0.1f * speedMultiplier);
        m_hullEmitter->setMaxSpeed(0.4f * speedMultiplier);
    }

    m_hullEmitter->setEmissionRate(emissionRate);
}

void Boat::update(float /*deltaTime*/)
{
    if (!m_physicsBody || !m_physicsBody->isInitialized())
        return;

    // Compute boat speed for spray modulation
    b2Vec2 velocity = m_physicsBody->getLinearVelocity();
    float  speed    = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);

    updateHullSprayForSpeed(speed);
    syncEmittersToBoat();
    checkStopMotorBoat();
}

void Boat::bindInputCallbacks()
{
    if (!m_input || !m_physicsBody || !m_inputManager)
        return;

    // Register boat input actions with the input system
    ActionBinding moveForward;
    moveForward.keys.push_back(KeyCode::W);
    moveForward.trigger = ActionTrigger::Held;
    m_inputManager->bindAction("MoveForward", moveForward);

    ActionBinding moveBackward;
    moveBackward.keys.push_back(KeyCode::S);
    moveBackward.trigger = ActionTrigger::Held;
    m_inputManager->bindAction("MoveBackward", moveBackward);

    ActionBinding rotateLeft;
    rotateLeft.keys.push_back(KeyCode::A);
    rotateLeft.trigger = ActionTrigger::Held;
    m_inputManager->bindAction("RotateLeft", rotateLeft);

    ActionBinding rotateRight;
    rotateRight.keys.push_back(KeyCode::D);
    rotateRight.trigger = ActionTrigger::Held;
    m_inputManager->bindAction("RotateRight", rotateRight);

    // Set callbacks for movement actions - store input intent instead of applying forces
    m_input->setActionCallback("MoveForward",
                               [this](ActionState state)
                               {
                                   if (state == ActionState::Held || state == ActionState::Pressed)
                                   {
                                       m_wantsForward = true;
                                       startMotorBoat();
                                   }
                                   else if (state == ActionState::Released)
                                   {
                                       m_wantsForward = false;
                                   }
                               });

    m_input->setActionCallback("MoveBackward",
                               [this](ActionState state)
                               {
                                   if (state == ActionState::Held || state == ActionState::Pressed)
                                   {
                                       m_wantsBackward = true;
                                       startMotorBoat();
                                   }
                                   else if (state == ActionState::Released)
                                   {
                                       m_wantsBackward = false;
                                   }
                               });

    m_input->setActionCallback("RotateLeft",
                               [this](ActionState state)
                               {
                                   if (state == ActionState::Held || state == ActionState::Pressed)
                                   {
                                       m_wantsLeft = true;
                                   }
                                   else if (state == ActionState::Released)
                                   {
                                       m_wantsLeft = false;
                                   }
                               });

    m_input->setActionCallback("RotateRight",
                               [this](ActionState state)
                               {
                                   if (state == ActionState::Held || state == ActionState::Pressed)
                                   {
                                       m_wantsRight = true;
                                   }
                                   else if (state == ActionState::Released)
                                   {
                                       m_wantsRight = false;
                                   }
                               });
}

void Boat::startMotorBoat()
{
    if (!m_audioSystem)
        return;

    // Check if motor boat is already playing
    if (m_audioSystem->isPlayingSFX(m_motorBoatHandle))
    {
        // If fading out, fade back in to target volume
        FadeConfig fadeIn = FadeConfig::linear(kMotorFadeDuration, true);
        m_audioSystem->fadeSFX(m_motorBoatHandle, kMotorMaxVolume, fadeIn);
        return;
    }

    // Start motor boat with fade-in
    FadeConfig fadeIn = FadeConfig::linear(kMotorFadeDuration, true);
    m_motorBoatHandle = m_audioSystem->playSFXWithFade("motor_boat", kMotorMaxVolume, 1.0f, true, fadeIn);
}

void Boat::checkStopMotorBoat()
{
    if (!m_audioSystem || !m_input)
        return;

    // Check if any movement action is still active using action states
    bool anyMovementActive = m_input->isActionDown("MoveForward") || m_input->isActionDown("MoveBackward");

    if (!anyMovementActive && m_audioSystem->isPlayingSFX(m_motorBoatHandle))
    {
        // Stop with fade-out
        FadeConfig fadeOut = FadeConfig::linear(kMotorFadeDuration, true);
        m_audioSystem->stopSFXWithFade(m_motorBoatHandle, fadeOut);
    }
}

void Boat::setupFixedUpdate()
{
    if (!m_physicsBody)
        return;

    // Set the fixed-update callback that runs once per physics step (60Hz)
    // This ensures frame-rate independent movement
    m_physicsBody->setFixedUpdateCallback(
        [this](float /*timeStep*/)
        {
            if (!m_physicsBody || !m_physicsBody->isInitialized())
                return;

            // Apply forward/backward thrust
            if (m_wantsForward)
            {
                b2Vec2 forward = m_physicsBody->getForwardVector();
                b2Vec2 force   = {forward.x * kPlayerForce, forward.y * kPlayerForce};
                m_physicsBody->applyForceToCenter(force);
            }
            else if (m_wantsBackward)
            {
                b2Vec2 forward = m_physicsBody->getForwardVector();
                b2Vec2 force   = {-forward.x * (kPlayerForce / 2), -forward.y * (kPlayerForce / 2)};
                m_physicsBody->applyForceToCenter(force);
            }

            // Apply rudder steering (left)
            if (m_wantsLeft)
            {
                b2Vec2 forward = m_physicsBody->getForwardVector();
                b2Vec2 right   = m_physicsBody->getRightVector();
                b2Vec2 vel     = m_physicsBody->getLinearVelocity();

                float forwardVelSigned = forward.x * vel.x + forward.y * vel.y;
                float absForwardVel    = std::fabs(forwardVelSigned);

                if (absForwardVel >= kMinSpeedForSteering)
                {
                    b2Vec2 stern   = m_physicsBody->getPosition() - forward * kRudderOffsetMeters;
                    b2Vec2 lateral = (forwardVelSigned >= 0.0f) ? right : b2Vec2{-right.x, -right.y};

                    float speedEffective = std::max(0.0f, absForwardVel - kMinSpeedForSteering);
                    float normalized     = speedEffective / (speedEffective + kRudderSmoothK);
                    float speedFactor    = kRudderMinEffectiveScale + normalized * (1.0f - kRudderMinEffectiveScale);

                    float  forceMag = kPlayerTurningForce * kRudderForceMultiplier * speedFactor;
                    b2Vec2 force{lateral.x * forceMag, lateral.y * forceMag};

                    m_physicsBody->applyForce(force, stern);
                }
            }

            // Apply rudder steering (right)
            if (m_wantsRight)
            {
                b2Vec2 forward = m_physicsBody->getForwardVector();
                b2Vec2 right   = m_physicsBody->getRightVector();
                b2Vec2 vel     = m_physicsBody->getLinearVelocity();

                float forwardVelSigned = forward.x * vel.x + forward.y * vel.y;
                float absForwardVel    = std::fabs(forwardVelSigned);

                if (absForwardVel >= kMinSpeedForSteering)
                {
                    b2Vec2 stern   = m_physicsBody->getPosition() - forward * kRudderOffsetMeters;
                    b2Vec2 lateral = (forwardVelSigned >= 0.0f) ? b2Vec2{-right.x, -right.y} : right;

                    float speedEffective = std::max(0.0f, absForwardVel - kMinSpeedForSteering);
                    float normalized     = speedEffective / (speedEffective + kRudderSmoothK);
                    float speedFactor    = kRudderMinEffectiveScale + normalized * (1.0f - kRudderMinEffectiveScale);

                    float  forceMag = kPlayerTurningForce * kRudderForceMultiplier * speedFactor;
                    b2Vec2 force{lateral.x * forceMag, lateral.y * forceMag};

                    m_physicsBody->applyForce(force, stern);
                }
            }
        });
}
