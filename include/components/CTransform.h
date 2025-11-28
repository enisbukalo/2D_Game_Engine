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
     * @brief Gets the local position of the transform (relative to parent)
     * @return Local position as a Vec2
     */
    Vec2 getLocalPosition() const;

    /**
     * @brief Gets the world position of the transform (computed on demand)
     * @return World position as a Vec2
     */
    Vec2 getWorldPosition() const;

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
     * @brief Gets the local scale of the transform (relative to parent)
     * @return Local scale as a Vec2
     */
    Vec2 getLocalScale() const;

    /**
     * @brief Gets the world scale of the transform (computed on demand)
     * @return World scale as a Vec2
     */
    Vec2 getWorldScale() const;

    /**
     * @brief Gets the rotation of the transform
     * @return Rotation as a float
     */
    float getRotation() const;

    /**
     * @brief Gets the local rotation of the transform (relative to parent)
     * @return Local rotation as a float
     */
    float getLocalRotation() const;

    /**
     * @brief Gets the world rotation of the transform (computed on demand)
     * @return World rotation as a float
     */
    float getWorldRotation() const;

    /**
     * @brief Sets the position of the transform
     * @param pos New position as a Vec2
     */
    void setPosition(const Vec2& pos);

    /**
     * @brief Sets the local position of the transform (relative to parent)
     * @param pos New local position as a Vec2
     */
    void setLocalPosition(const Vec2& pos);

    /**
     * @brief Sets the world position of the transform
     * @param pos New world position as a Vec2
     */
    void setWorldPosition(const Vec2& pos);

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
     * @brief Sets the local scale of the transform (relative to parent)
     * @param scl New local scale as a Vec2
     */
    void setLocalScale(const Vec2& scl);

    /**
     * @brief Sets the world scale of the transform
     * @param scl New world scale as a Vec2
     */
    void setWorldScale(const Vec2& scl);

    /**
     * @brief Sets the rotation of the transform
     * @param rot New rotation as a float
     */
    void setRotation(float rot);

    /**
     * @brief Sets the local rotation of the transform (relative to parent)
     * @param rot New local rotation as a float
     */
    void setLocalRotation(float rot);

    /**
     * @brief Sets the world rotation of the transform
     * @param rot New world rotation as a float
     */
    void setWorldRotation(float rot);

private:
    Vec2  m_localPosition = Vec2(0.0f, 0.0f);  ///< Position relative to parent (or world if no parent)
    Vec2  m_velocity      = Vec2(0.0f, 0.0f);  ///< Current velocity vector
    Vec2  m_localScale    = Vec2(1.0f, 1.0f);  ///< Scale relative to parent (or world if no parent)
    float m_localRotation = 0.0f;              ///< Rotation relative to parent (or world if no parent)

    // Legacy members kept for backward compatibility - now alias to local transforms
    Vec2& m_position = m_localPosition;  ///< Alias to m_localPosition for backward compatibility
    Vec2& m_scale    = m_localScale;     ///< Alias to m_localScale for backward compatibility
    float& m_rotation = m_localRotation; ///< Alias to m_localRotation for backward compatibility
};

#endif  // CTRANSFORM_H