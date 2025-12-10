#pragma once

#include <Entity.h>
#include <memory>
#include <random>
#include <vector>
#include "BarrelEntity.h"

class GameEngine;

class BarrelSpawner : public Entity::Entity
{
protected:
    friend class Systems::SEntity;

    BarrelSpawner(const std::string& tag, size_t id, GameEngine* engine, float minX, float maxX, float minY, float maxY, size_t count);

    void init() override;
    void update(float deltaTime) override;

private:
    GameEngine*                           m_engine;
    float                                 m_minX;
    float                                 m_maxX;
    float                                 m_minY;
    float                                 m_maxY;
    size_t                                m_barrelCount;
    std::mt19937                          m_rng;
    std::uniform_real_distribution<float> m_distX;
    std::uniform_real_distribution<float> m_distY;
};
