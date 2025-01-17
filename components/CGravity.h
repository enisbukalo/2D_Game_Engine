#ifndef CGRAVITY_H
#define CGRAVITY_H

#include "Component.h"
#include "Vec2.h"

struct CGravity : public Component
{
    Vec2 force = Vec2(0.0f, -9.81f);

    void        update(float deltaTime) override;
    std::string getType() const override;
    json        serialize() const override;
    void        deserialize(const json& data) override;
};

#endif  // CGRAVITY_H