#include "GameEngine.h"

GameEngine::GameEngine(sf::RenderWindow *window, b2Vec2 gravity, uint8_t subStepCount, float timeStep)
    : m_window(window), m_gravity(gravity), m_subStepCount(subStepCount), m_timeStep(timeStep)
{
    m_window->setFramerateLimit(144);

    m_worldDef         = b2DefaultWorldDef();
    m_worldDef.gravity = m_gravity;
    m_worldId          = b2CreateWorld(&m_worldDef);

    m_gameRunning = true;
}

GameEngine::~GameEngine()
{
    b2DestroyWorld(m_worldId);
}

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
    b2World_Step(m_worldId, m_timeStep, m_subStepCount);
}

void GameEngine::render()
{
    m_window->clear();
    //   for (sf::Shape *shape : shapes) {
    //     Game::window.draw(*shape);
    //   }

    //   Game::window.draw(lines);
    m_window->display();
}