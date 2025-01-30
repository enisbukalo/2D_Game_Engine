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
     * @brief Gets the force of the gravity
     * @return Force as a Vec2
     */
    Vec2 getForce() const;

    /**
     * @brief Sets the force of the gravity
     * @param force New force as a Vec2
     */
    void setForce(const Vec2& force);

private:
    Vec2 m_force = Vec2(0.0f, -9.81f);  ///< Gravity force vector (default: Earth gravity)
};

#endif  // CGRAVITY_H