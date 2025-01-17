#ifndef CGRAVITY_H
#define CGRAVITY_H

#include "Component.h"
#include "Vec2.h"

/**
 * @brief Component for applying gravity forces to entities
 *
 * @description
 * CGravity is a component that applies a constant force (typically downward)
 * to an entity's transform component. The force is applied each frame during
 * the physics update step. The default gravity force is set to -9.81 m/s²
 * in the Y direction, but can be modified for different gravity effects.
 */
struct CGravity : public Component
{
#pragma region Variables
    Vec2 force = Vec2(0.0f, -9.81f);  ///< Gravity force vector (default: Earth gravity)
#pragma endregion

#pragma region Override Methods
    /**
     * @brief Applies gravity force to the entity's transform
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime) override;

    /**
     * @brief Gets the type identifier for this component
     * @return String "Gravity"
     */
    std::string getType() const override;

    /**
     * @brief Serializes the gravity data to JSON
     * @return JSON object containing the force vector
     */
    json serialize() const override;

    /**
     * @brief Deserializes gravity data from JSON
     * @param data JSON object containing gravity data
     */
    void deserialize(const json& data) override;
#pragma endregion
};

#endif  // CGRAVITY_H