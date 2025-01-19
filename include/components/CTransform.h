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
#pragma region Variables
    Vec2  position = Vec2(0.0f, 0.0f);  ///< Current position in world space
    Vec2  velocity = Vec2(0.0f, 0.0f);  ///< Current velocity vector
    Vec2  scale    = Vec2(1.0f, 1.0f);  ///< Scale factor for x and y dimensions
    float rotation = 0.0f;              ///< Rotation angle in radians
#pragma endregion

#pragma region Override Methods
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
    void serialize() const override;

    /**
     * @brief Deserializes transform data from binary data
     */
    void deserialize() override;
#pragma endregion
};

#endif  // CTRANSFORM_H