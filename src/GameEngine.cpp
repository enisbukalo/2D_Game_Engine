#include "GameEngine.h"

GameEngine::GameEngine(sf::RenderWindow* window, sf::Vector2f gravity, uint8_t subStepCount, float timeStep)
    : m_window(window), m_gravity(gravity), m_subStepCount(subStepCount), m_timeStep(timeStep)
{
    m_gameRunning = true;
}

GameEngine::~GameEngine()
{
    // Cleanup resources if needed
}

void GameEngine::readInputs()
{
    while (const std::optional<sf::Event> event = m_window->pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            m_window->close();
            m_gameRunning = false;
        }
    }
}

void GameEngine::update()
{
    // Update game state
    // This will be expanded as we add more systems
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