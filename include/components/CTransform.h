#ifndef CTRANSFORM_H
#define CTRANSFORM_H

#include <string>
#include "SSerialization.h"
#include "Vec2.h"

namespace Components
{

/**
 * @brief Component for handling entity position, movement, and transformation
 *
 * @description
 * CTransform is a component that manages an entity's spatial properties including
 * position, velocity, scale, and rotation. It provides the basic functionality
 * for moving and transforming entities in the game world. The component is
 * updated each frame to apply velocity-based movement.
 */
struct CTransform
{
    CTransform() = default;
    CTransform(const Vec2& pos, const Vec2& scl, float rot) : position(pos), scale(scl), rotation(rot) {}

    std::string getType() const;
    void        serialize(Serialization::JsonBuilder& builder) const;
    void        deserialize(const Serialization::SSerialization::JsonValue& value);

    Vec2  getPosition() const;
    Vec2  getVelocity() const;
    Vec2  getScale() const;
    float getRotation() const;

    void setPosition(const Vec2& pos);
    void setVelocity(const Vec2& vel);
    void setScale(const Vec2& scl);
    void setRotation(float rot);

    Vec2  position = Vec2(0.0f, 0.0f);
    Vec2  velocity = Vec2(0.0f, 0.0f);
    Vec2  scale    = Vec2(1.0f, 1.0f);
    float rotation = 0.0f;
};

}  // namespace Components

#endif  // CTRANSFORM_H