#ifndef COMPONENT_H
#define COMPONENT_H

#include <box2d/box2d.h>

class Component {
public:
  Component();
  ~Component();
};

struct TransformComponent : Component {
  b2Vec2 position;
  b2Rot rotation;
};

struct PhysicsComponent : Component {
  b2Vec2 velocity;
  float angularVelocity;
};

struct InputComponent : Component {};

struct SpriteComponent : Component {};

#endif // COMPONENT_H