#ifndef CTRANSFORM_H
#define CTRANSFORM_H

#include "Component.h"
#include "Vec2.h"

struct CTransform : public Component
{
#pragma region Variables
    Vec2  position = Vec2(0.0f, 0.0f);
    Vec2  velocity = Vec2(0.0f, 0.0f);
    Vec2  scale    = Vec2(1.0f, 1.0f);
    float rotation = 0.0f;
#pragma endregion

#pragma region Override Methods
    void        update(float deltaTime) override;
    std::string getType() const override;
    json        serialize() const override;
    void        deserialize(const json& data) override;
#pragma endregion
};

#endif  // CTRANSFORM_H