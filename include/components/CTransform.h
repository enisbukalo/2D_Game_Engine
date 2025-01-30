#ifndef CTRANSFORM_H
#define CTRANSFORM_H

#include "Component.h"
#include "Vec2.h"

/**
 * @brief Component for handling entity position, movement, and transformation
 *
 * @description
 * CTransform is a component that manages an entity's spatial properties including
 * position, velocity, scale, and rotation. It provides the basic functionality
 * for moving and transforming entities in the game world. The component is
 * updated each frame to apply velocity-based movement.
 */
struct CTransform : public Component
{
public:
    /** @brief Default constructor */
    CTransform() = default;

    /**
     * @brief Constructs a transform with initial position, scale, and rotation
     * @param pos Initial position
     * @param scl Initial scale
     * @param rot Initial rotation in radians
     */
    CTransform(const Vec2& pos, const Vec2& scl, float rot) : m_position(pos), m_scale(scl), m_rotation(rot) {}

    /**
     * @brief Updates the transform based on velocity
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime) override;

    /**
     * @brief Gets the type identifier for this component
     * @return String "Transform"
     */
    std::string getType() const override;

    /**
     * @brief Serializes the transform data to binary data
     */
    void serialize(JsonBuilder& builder) const override;

    /**
     * @brief Deserializes transform data from binary data
     */
    void deserialize(const JsonValue& value) override;

    /**
     * @brief Gets the position of the transform
     * @return Position as a Vec2
     */
    Vec2 getPosition() const;

    /**
     * @brief Gets the velocity of the transform
     * @return Velocity as a Vec2
     */
    Vec2 getVelocity() const;

    /**
     * @brief Gets the scale of the transform
     * @return Scale as a Vec2
     */
    Vec2 getScale() const;

    /**
     * @brief Gets the rotation of the transform
     * @return Rotation as a float
     */
    float getRotation() const;

    /**
     * @brief Sets the position of the transform
     * @param pos New position as a Vec2
     */
    void setPosition(const Vec2& pos);

    /**
     * @brief Sets the velocity of the transform
     * @param vel New velocity as a Vec2
     */
    void setVelocity(const Vec2& vel);

    /**
     * @brief Sets the scale of the transform
     * @param scl New scale as a Vec2
     */
    void setScale(const Vec2& scl);

    /**
     * @brief Sets the rotation of the transform
     * @param rot New rotation as a float
     */
    void setRotation(float rot);

private:
    Vec2  m_position = Vec2(0.0f, 0.0f);  ///< Current position in world space
    Vec2  m_velocity = Vec2(0.0f, 0.0f);  ///< Current velocity vector
    Vec2  m_scale    = Vec2(1.0f, 1.0f);  ///< Scale factor for x and y dimensions
    float m_rotation = 0.0f;              ///< Rotation angle in radians
};

#endif  // CTRANSFORM_H