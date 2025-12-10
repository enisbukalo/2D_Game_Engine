#pragma once

#include <GameEngine.h>
#include <Vec2.h>
#include <memory>
#include "AudioTypes.h"
#include "Components.h"

namespace Systems
{
class SAudio;
class SInput;
}  // namespace Systems

// Boat is a concrete entity that owns the player boat plus its emitters.
class Boat : public Entity::Entity
{
protected:
    friend class Systems::SEntity;

    Boat(const std::string& tag, size_t id, Systems::SInput* inputManager, Systems::SAudio* audioSystem);

    void init() override;
    void update(float deltaTime) override;

    Components::CPhysicsBody2D* getPhysicsBody() const
    {
        return m_physicsBody;
    }
    Components::CInputController* getInputController() const
    {
        return m_input;
    }
    std::shared_ptr<::Entity::Entity> getBubbleTrailEntity() const
    {
        return m_bubbleTrail;
    }
    std::shared_ptr<::Entity::Entity> getHullSprayEntity() const
    {
        return m_hullSpray;
    }
    Components::CParticleEmitter* getHullEmitter() const
    {
        return m_hullEmitter;
    }

private:
    void configureBoatBody();
    void configureBubbleTrail();
    void configureHullSpray();
    void bindInputCallbacks();
    void setupFixedUpdate();
    void syncEmittersToBoat();
    void updateHullSprayForSpeed(float speed);
    void startMotorBoat();
    void checkStopMotorBoat();

    sf::Texture m_bubbleTexture;
    sf::Texture m_sprayTexture;

    Systems::SInput* m_inputManager = nullptr;
    Systems::SAudio* m_audioSystem  = nullptr;
    AudioHandle      m_motorBoatHandle;

    Components::CTransform*       m_transform   = nullptr;
    Components::CPhysicsBody2D*   m_physicsBody = nullptr;
    Components::CInputController* m_input       = nullptr;

    // Input intent state (set by input callbacks, consumed by fixed update)
    bool m_wantsForward  = false;
    bool m_wantsBackward = false;
    bool m_wantsLeft     = false;
    bool m_wantsRight    = false;

    std::shared_ptr<::Entity::Entity> m_bubbleTrail;
    Components::CParticleEmitter*     m_bubbleEmitter = nullptr;

    std::shared_ptr<::Entity::Entity> m_hullSpray;
    Components::CParticleEmitter*     m_hullEmitter = nullptr;
};
