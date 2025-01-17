#ifndef CGRAVITY_H
#define CGRAVITY_H

#include "Component.h"
#include "Vec2.h"

struct CGravity : public Component
{
#pragma region Variables
    Vec2 force = Vec2(0.0f, -9.81f);
#pragma endregion

#pragma region Override Methods
    void        update(float deltaTime) override;
    std::string getType() const override;
    json        serialize() const override;
    void        deserialize(const json& data) override;
#pragma endregion
};

#endif  // CGRAVITY_H