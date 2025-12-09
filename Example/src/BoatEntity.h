#pragma once

#include <GameEngine.h>
#include <Vec2.h>
#include <memory>
#include "components/CInputController.h"
#include "components/CParticleEmitter.h"
#include "components/CPhysicsBody2D.h"
#include "components/CTransform.h"
#include "entities/EntityFactory.h"
#include "systems/AudioTypes.h"

using EntityPtr = std::shared_ptr<::Entity::Entity>;

namespace Systems
{
class SAudio;
class SInput;
}  // namespace Systems

// Boat is a concrete entity that owns the player boat plus its emitters.
class Boat : public Entity::Entity
{
public:
    /**
     * @brief Convenience factory for creating Boat entities
     * @param inputManager Input system for player controls
     * @param audioSystem Audio system for motor sounds
     * @return Shared pointer to the created Boat entity
     */
    static std::shared_ptr<Boat> spawn(Systems::SInput* inputManager, Systems::SAudio* audioSystem)
    {
        return Entity::create<Boat>("player", inputManager, audioSystem);
    }

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
    EntityPtr getBubbleTrailEntity() const
    {
        return m_bubbleTrail;
    }
    EntityPtr getHullSprayEntity() const
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

    EntityPtr                     m_bubbleTrail;
    Components::CParticleEmitter* m_bubbleEmitter = nullptr;

    EntityPtr                     m_hullSpray;
    Components::CParticleEmitter* m_hullEmitter = nullptr;
};
