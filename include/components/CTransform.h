#ifndef CTRANSFORM_H
#define CTRANSFORM_H

#include <string>
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

    inline Vec2 getPosition() const
    {
        return position;
    }
    inline Vec2 getVelocity() const
    {
        return velocity;
    }
    inline Vec2 getScale() const
    {
        return scale;
    }
    inline float getRotation() const
    {
        return rotation;
    }

    inline void setPosition(const Vec2& pos)
    {
        position = pos;
    }
    inline void setVelocity(const Vec2& vel)
    {
        velocity = vel;
    }
    inline void setScale(const Vec2& scl)
    {
        scale = scl;
    }
    inline void setRotation(float rot)
    {
        rotation = rot;
    }

    Vec2  position = Vec2(0.0f, 0.0f);
    Vec2  velocity = Vec2(0.0f, 0.0f);
    Vec2  scale    = Vec2(1.0f, 1.0f);
    float rotation = 0.0f;
};

}  // namespace Components

#endif  // CTRANSFORM_H