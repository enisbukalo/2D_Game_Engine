#include "BarrelSpawner.h"

#include "BarrelEntity.h"

#include <World.h>

namespace Example
{

BarrelSpawnerScript::BarrelSpawnerScript(float minX, float maxX, float minY, float maxY, size_t count)
    : m_minX(minX),
      m_maxX(maxX),
      m_minY(minY),
      m_maxY(maxY),
      m_barrelCount(count),
      m_rng(std::random_device{}()),
      m_distX(minX, maxX),
      m_distY(minY, maxY)
{
}

void BarrelSpawnerScript::onCreate(Entity /*self*/, World& world)
{
    for (size_t i = 0; i < m_barrelCount; ++i)
    {
        const Vec2 pos{m_distX(m_rng), m_distY(m_rng)};
        (void)spawnBarrel(world, pos);
    }
}

void BarrelSpawnerScript::onUpdate(float /*deltaTime*/, Entity /*self*/, World& /*world*/) {}

}  // namespace Example
