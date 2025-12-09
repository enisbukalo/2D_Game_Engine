#pragma once

#include <GameEngine.h>
#include <memory>
#include "components/CParticleEmitter.h"
#include "components/CPhysicsBody2D.h"
#include "components/CTransform.h"
#include "entities/EntityFactory.h"

// Barrel is a concrete entity that owns its own emitter and physics.
class Barrel : public Entity::Entity
{
public:
    /**
     * @brief Convenience factory for creating Barrel entities
     * @param position Initial position in world space (meters)
     * @return Shared pointer to the created Barrel entity
     */
    static std::shared_ptr<Barrel> spawn(const Vec2& position)
    {
        return Entity::create<Barrel>("barrel", position);
    }

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
