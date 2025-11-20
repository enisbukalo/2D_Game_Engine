#ifndef CGRAVITY_H
#define CGRAVITY_H

#include "Component.h"
#include "Vec2.h"

/**
 * @brief Component for applying gravity forces to entities
 *
 * @description
 * CGravity is a component that applies a multiplier to the global gravity force.
 * The multiplier is applied to the global gravity defined in S2DPhysics during
 * the physics update step. The default multiplier is 1.0, which applies the global
 * gravity as-is. A multiplier of 0.0 means no gravity, 2.0 means double gravity,
 * and -1.0 means reverse gravity.
 */
struct CGravity : public Component
{
public:
    /**
     * @brief Applies gravity force to the entity's transform
     * @param deltaTime Time elapsed since last update
     */
    // cppcheck-suppress uselessOverride
    // cppcheck-suppress missingOverride
    void update(float deltaTime) override;

    /**
     * @brief Gets the type identifier for this component
     * @return String "Gravity"
     */
    std::string getType() const override;

    /**
     * @brief Serializes the gravity data to binary data
     * @param builder The JSON builder to serialize to
     */
    void serialize(JsonBuilder& builder) const override;

    /**
     * @brief Deserializes gravity data from binary data
     * @param value The JSON value to deserialize
     */
    void deserialize(const JsonValue& value) override;

    /**
     * @brief Gets the gravity multiplier
     * @return Multiplier value (1.0 = normal gravity, 0.0 = no gravity, 2.0 = double gravity, etc.)
     */
    float getMultiplier() const;

    /**
     * @brief Sets the gravity multiplier
     * @param multiplier New multiplier value
     */
    void setMultiplier(float multiplier);

private:
    float m_multiplier = 1.0f;  ///< Gravity multiplier (default: 1.0 for normal gravity)
};

#endif  // CGRAVITY_H