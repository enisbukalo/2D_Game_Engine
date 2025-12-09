#pragma once

#include <GameEngine.h>
#include <memory>
#include <random>
#include <vector>
#include "BarrelEntity.h"

class BarrelSpawner
{
public:
    BarrelSpawner(GameEngine& engine, float minX, float maxX, float minY, float maxY);

    /**
     * @brief Spawn the requested number of barrels at random positions within the bounds.
     * @param count Number of barrels to spawn.
     * @return Vector of spawned Barrel entities.
     */
    std::vector<std::shared_ptr<Barrel>> spawn(size_t count);

private:
    GameEngine&                           m_engine;
    float                                 m_minX;
    float                                 m_maxX;
    float                                 m_minY;
    float                                 m_maxY;
    std::mt19937                          m_rng;
    std::uniform_real_distribution<float> m_distX;
    std::uniform_real_distribution<float> m_distY;
};
