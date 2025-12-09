#include "BarrelSpawner.h"

#include <GameEngine.h>
#include <chrono>

BarrelSpawner::BarrelSpawner(const std::string& tag, size_t id, GameEngine* engine, float minX, float maxX, float minY, float maxY, size_t count)
    : Entity(tag, id), m_engine(engine), m_minX(minX), m_maxX(maxX), m_minY(minY), m_maxY(maxY), m_barrelCount(count)
{
    auto seed = static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    m_rng     = std::mt19937(seed);
    m_distX   = std::uniform_real_distribution<float>(m_minX, m_maxX);
    m_distY   = std::uniform_real_distribution<float>(m_minY, m_maxY);
}

void BarrelSpawner::init()
{
    Entity::init();

    // Spawn all barrels during initialization
    for (size_t i = 0; i < m_barrelCount; ++i)
    {
        Vec2 pos(m_distX(m_rng), m_distY(m_rng));
        m_engine->spawn<Barrel>("barrel", pos);
    }
}

void BarrelSpawner::update(float deltaTime)
{
    Entity::update(deltaTime);
    // No update logic needed - spawning happens once in init()
}
