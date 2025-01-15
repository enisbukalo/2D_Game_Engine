#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>

class GameEngine
{
public:
    GameEngine(sf::RenderWindow *window, b2Vec2 gravity, uint8_t subStepCount, float timeStep);
    ~GameEngine();

    void readInputs();
    void update();
    void render();

    bool is_running()
    {
        return m_gameRunning;
    };

private:
    const uint8_t m_subStepCount;
    const float   m_timeStep;

    bool m_gameRunning = false;

    sf::RenderWindow *m_window;
    b2Vec2            m_gravity;
    b2WorldDef        m_worldDef;
    b2WorldId         m_worldId;
};

#endif  // GAMEENGINE_H