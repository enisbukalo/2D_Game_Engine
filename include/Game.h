#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

class Game {
public:
  Game(sf::RenderWindow *window, b2Vec2 gravity, uint8_t subStepCount, float timeStep);
  ~Game();

  void readInputs();
  void update();
  void render();

  bool is_running() { return m_gameRunning; };

private:
  const uint8_t m_subStepCount;
  const float m_timeStep;

  bool m_gameRunning = false;

  sf::RenderWindow *m_window;
  b2Vec2 m_gravity;
  b2WorldDef m_worldDef;
  b2WorldId m_worldId;
};

#endif // GAME_H