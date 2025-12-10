#pragma once

#include <GameEngine.h>
#include <memory>
#include "Components.h"

// Barrel is a concrete entity that owns its own emitter and physics.
class Barrel : public Entity::Entity
{
protected:
    friend class Systems::SEntity;

    Barrel(const std::string& tag, size_t id, const Vec2& position);

    void init() override;
    void update(float deltaTime) override;

    Components::CPhysicsBody2D* getPhysicsBody() const
    {
        return m_physics;
    }

private:
    void configureBarrel();
    void updateSprayForSpeed(float speed);

    Vec2        m_spawnPosition;
    sf::Texture m_sprayTexture;

    Components::CTransform*       m_transform = nullptr;
    Components::CPhysicsBody2D*   m_physics   = nullptr;
    Components::CParticleEmitter* m_emitter   = nullptr;
};
