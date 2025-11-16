#ifndef CBOXCOLLIDER_H
#define CBOXCOLLIDER_H

#include "CCollider.h"

/**
 * @brief Box (AABB) collision shape component
 *
 * @description
 * CBoxCollider implements an axis-aligned bounding box collision shape. It provides
 * efficient box vs box, box vs circle collision detection and can be used for both
 * trigger and solid collisions. The box is defined by its width and height, and the
 * position comes from the entity's transform component.
 */
class CBoxCollider : public CCollider
{
public:
    /**
     * @brief Constructs a box collider with default size (1x1)
     */
    CBoxCollider() : m_size(1.0f, 1.0f) {}

    /**
     * @brief Constructs a box collider with specified width and height
     * @param width The width of the box in world units
     * @param height The height of the box in world units
     */
    CBoxCollider(float width, float height);

    /**
     * @brief Constructs a box collider with specified size vector
     * @param size The size (width, height) of the box in world units
     */
    explicit CBoxCollider(const Vec2& size);

    /**
     * @brief Gets the bounding box for this collider
     * @return AABB representing this box collider
     */
    AABB getBounds() const override;

    /**
     * @brief Checks for collision with another collider
     * @param other The other collider to check against
     * @return true if colliding, false otherwise
     *
     * Supports box vs box, box vs circle collision detection
     */
    bool intersects(const CCollider* other) const override;

    /**
     * @brief Gets the width of the box
     * @return The box's width in world units
     */
    float getWidth() const;

    /**
     * @brief Gets the height of the box
     * @return The box's height in world units
     */
    float getHeight() const;

    /**
     * @brief Gets the size of the box as a vector
     * @return Vec2 containing (width, height)
     */
    Vec2 getSize() const;

    /**
     * @brief Sets the size of the box
     * @param width New width in world units
     * @param height New height in world units
     */
    void setSize(float width, float height);

    /**
     * @brief Sets the size of the box
     * @param size New size (width, height) in world units
     */
    void setSize(const Vec2& size);

    /**
     * @brief Gets the type identifier for this component
     * @return String "BoxCollider"
     */
    std::string getType() const override;

    // Override Component serialization methods
    void serialize(JsonBuilder& builder) const override;
    void deserialize(const JsonValue& value) override;

private:
    Vec2 m_size;  ///< Size of the box (width, height) in world units
};

#endif  // CBOXCOLLIDER_H
