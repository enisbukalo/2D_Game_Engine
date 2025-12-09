#include "BarrelSpawner.h"

#include <chrono>

BarrelSpawner::BarrelSpawner(GameEngine& engine, float minX, float maxX, float minY, float maxY)
    : m_engine(engine), m_minX(minX), m_maxX(maxX), m_minY(minY), m_maxY(maxY)
{
    auto seed = static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    m_rng     = std::mt19937(seed);
    m_distX   = std::uniform_real_distribution<float>(m_minX, m_maxX);
    m_distY   = std::uniform_real_distribution<float>(m_minY, m_maxY);
}

std::vector<std::shared_ptr<Barrel>> BarrelSpawner::spawn(size_t count)
{
    std::vector<std::shared_ptr<Barrel>> result;
    result.reserve(count);
    for (size_t i = 0; i < count; ++i)
    {
        Vec2 pos(m_distX(m_rng), m_distY(m_rng));
        result.emplace_back(Barrel::spawn(pos));
    }
    return result;
}
