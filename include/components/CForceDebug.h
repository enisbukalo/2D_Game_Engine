#pragma once

#include "Component.h"
#include "Vec2.h"

/**
 * @brief Component for storing force data for visualization purposes
 *
 * This component stores forces applied to an entity for debugging and visualization.
 * It does not affect physics calculations - it only records forces for display.
 */
class CForceDebug : public Component
{
private:
    Vec2 m_gravityForce;  // Gravity force applied this frame

public:
    CForceDebug() : m_gravityForce(0.0f, 0.0f) {}

    /**
     * @brief Set the gravity force for this frame
     * @param force The gravity force vector
     */
    void setGravityForce(const Vec2& force)
    {
        m_gravityForce = force;
    }

    /**
     * @brief Get the gravity force for visualization
     * @return The gravity force vector
     */
    Vec2 getGravityForce() const
    {
        return m_gravityForce;
    }

    /**
     * @brief Clear all recorded forces (call at start of physics frame)
     */
    void clearForces()
    {
        m_gravityForce = Vec2(0.0f, 0.0f);
    }

    /**
     * @brief Gets the type identifier for this component
     * @return String "ForceDebug"
     */
    std::string getType() const override
    {
        return "ForceDebug";
    }

    /**
     * @brief Serializes the force debug data to JSON
     * @param builder The JSON builder to serialize to
     */
    void serialize(JsonBuilder& builder) const override
    {
        builder.beginObject();
        builder.addKey("cForceDebug");
        builder.beginObject();
        builder.addKey("gravityForceX");
        builder.addNumber(m_gravityForce.x);
        builder.addKey("gravityForceY");
        builder.addNumber(m_gravityForce.y);
        builder.endObject();
        builder.endObject();
    }

    /**
     * @brief Deserializes force debug data from JSON
     * @param value The JSON value to deserialize
     */
    void deserialize(const JsonValue& value) override
    {
        const auto& forceDebug = value["cForceDebug"];
        m_gravityForce.x       = static_cast<float>(forceDebug["gravityForceX"].getNumber());
        m_gravityForce.y       = static_cast<float>(forceDebug["gravityForceY"].getNumber());
    }
};
