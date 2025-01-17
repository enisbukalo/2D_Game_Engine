#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <SFML/Graphics.hpp>

class GameEngine
{
public:
    GameEngine(sf::RenderWindow* window, sf::Vector2f gravity, uint8_t subStepCount, float timeStep);
    ~GameEngine();

    void readInputs();
    void update();
    void render();
    bool is_running() const;

private:
    const uint8_t m_subStepCount;
    const float   m_timeStep;

    bool m_gameRunning = false;

    sf::RenderWindow* m_window;
    sf::Vector2f      m_gravity;
};

#endif  // GAMEENGINE_H