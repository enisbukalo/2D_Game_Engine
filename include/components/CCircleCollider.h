#ifndef CCIRCLECOLLIDER_H
#define CCIRCLECOLLIDER_H

#include "CCollider.h"

/**
 * @brief Circular collision shape component
 *
 * @description
 * CCircleCollider implements a circular collision shape. It provides efficient
 * circle vs circle collision detection and can be used for both trigger and
 * solid collisions. The circle is defined by its radius and the position comes
 * from the entity's transform component.
 */
class CCircleCollider : public CCollider
{
public:
    /**
     * @brief Constructs a circle collider with default radius
     */
    CCircleCollider() : m_radius(1.0f) {}

    /**
     * @brief Constructs a circle collider with specified radius
     * @param radius The radius of the circle in world units
     */
    explicit CCircleCollider(float radius);

    /**
     * @brief Gets the bounding box that contains this circle
     * @return AABB that fully encloses the circle
     */
    AABB getBounds() const override;

    /**
     * @brief Checks for collision with another collider
     * @param other The other collider to check against
     * @return true if colliding, false otherwise
     *
     * Currently only supports circle vs circle collision
     */
    bool intersects(const CCollider* other) const override;

    /**
     * @brief Gets the radius of the circle
     * @return The circle's radius in world units
     */
    float getRadius() const;

    /**
     * @brief Sets the radius of the circle
     * @param radius New radius in world units
     */
    void setRadius(float radius);

    /**
     * @brief Gets the type identifier for this component
     * @return String "CircleCollider"
     */
    std::string getType() const override;

    // Override Component serialization methods
    void serialize(JsonBuilder& builder) const override;
    void deserialize(const JsonValue& value) override;

private:
    float m_radius;  ///< Radius of the circle in world units
};

#endif