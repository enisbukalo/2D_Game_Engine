#ifndef COMPONENT_H
#define COMPONENT_H

#include <box2d/box2d.h>
#include <string>

#include "Vec2.h"

class Component {
public:
  Component();
  ~Component();
};

struct CName : Component {
  std::string name;
};

struct CTransform : Component {
  Vec2 position;
  b2Rot rotation;
};

struct CGravity : Component {
  Vec2 acceleration;
};

struct CInput : Component {};

struct CSprite : Component {};

#endif // COMPONENT_H