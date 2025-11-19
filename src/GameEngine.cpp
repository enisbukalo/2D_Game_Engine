#include "GameEngine.h"
#include "EntityManager.h"
#include "systems/S2DPhysics.h"
#include "utility/Logger.h"

GameEngine::GameEngine(sf::RenderWindow* window, sf::Vector2f gravity, uint8_t subStepCount, float timeStep)
    : m_window(window), m_gravity(gravity), m_subStepCount(subStepCount), m_timeStep(timeStep)
{
    // Initialize logger
    Logger::instance().init("game_engine.log");
    LOG_INFO("GameEngine initialized");
    LOG_INFO_STREAM("Window size: " << window->getSize().x << "x" << window->getSize().y);
    LOG_INFO_STREAM("SubSteps: " << (int)subStepCount << ", TimeStep: " << timeStep);

    m_gameRunning = true;

    // Set up physics world bounds based on window size
    sf::Vector2u windowSize = window->getSize();
    Vec2         center(windowSize.x / 2.0f, windowSize.y / 2.0f);
    Vec2         size(windowSize.x, windowSize.y);
    S2DPhysics::instance().setWorldBounds(center, size);
}

GameEngine::~GameEngine()
{
    LOG_INFO("GameEngine shutting down");
    Logger::instance().shutdown();
}

void GameEngine::readInputs()
{
    sf::Event event;
    while (m_window->pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            m_window->close();
            m_gameRunning = false;
        }
    }
}

void GameEngine::update(float deltaTime)
{
    // Accumulate time for fixed timestep updates
    m_accumulator += deltaTime;

    // Run physics updates with fixed timestep (prevents spiral of death)
    const float maxAccumulator = m_timeStep * 10.0f;  // Cap to prevent spiral of death
    if (m_accumulator > maxAccumulator)
    {
        m_accumulator = maxAccumulator;
    }

    // Process fixed timestep updates
    while (m_accumulator >= m_timeStep)
    {
        // Run physics substeps
        for (uint8_t i = 0; i < m_subStepCount; ++i)
        {
            S2DPhysics::instance().update(m_timeStep / m_subStepCount);
        }

        m_accumulator -= m_timeStep;
    }

    // Update entity manager with the actual frame delta time
    EntityManager::instance().update(deltaTime);
}

void GameEngine::render()
{
    m_window->clear();
    // Render game objects
    m_window->display();
}

bool GameEngine::is_running() const
{
    return m_gameRunning;
}