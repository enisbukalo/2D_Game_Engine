#include "GameEngine.h"

GameEngine::GameEngine(sf::RenderWindow *window, sf::Vector2f gravity, uint8_t subStepCount, float timeStep)
    : m_window(window), m_gravity(gravity), m_subStepCount(subStepCount), m_timeStep(timeStep)
{
    m_window->setFramerateLimit(144);
    m_gameRunning = true;
}

GameEngine::~GameEngine() {}

void GameEngine::readInputs()
{
    for (sf::Event event = sf::Event(); m_window->pollEvent(event);)
    {
        if (event.type == sf::Event::Closed)
        {
            m_window->close();
            m_gameRunning = false;
        }
    }
}

void GameEngine::update()
{
    // Update game logic here
}

void GameEngine::render()
{
    m_window->clear();
    // Render game objects here
    m_window->display();
}