#ifndef CCOLLIDER_H
#define CCOLLIDER_H

#include "Component.h"
#include "Vec2.h"
#include "physics/Quadtree.h"

/**
 * @brief Base class for all collision components
 *
 * @description
 * CCollider serves as the abstract base class for all collision components in the physics system.
 * It defines the interface for collision detection and provides common functionality like
 * trigger states. Derived classes must implement specific collision shapes and detection logic.
 */
class CCollider : public Component
{
public:
    virtual ~CCollider() override = default;

    /**
     * @brief Gets the axis-aligned bounding box for broad-phase collision detection
     * @return AABB representing the bounds of this collider
     */
    virtual AABB getBounds() const = 0;

    /**
     * @brief Performs detailed collision check against another collider
     * @param other Pointer to the other collider to check against
     * @return true if colliding, false otherwise
     */
    virtual bool intersects(const CCollider* other) const = 0;

    /**
     * @brief Checks if this collider is a trigger
     * @return true if trigger, false if solid
     *
     * Trigger colliders detect collisions but don't cause physical responses
     */
    bool isTrigger() const;

    /**
     * @brief Sets the trigger state of this collider
     * @param trigger New trigger state
     */
    void setTrigger(bool trigger);

    /**
     * @brief Checks if this collider is static (immovable)
     * @return true if static, false if dynamic
     *
     * Static colliders cannot be moved by collisions (like walls/ground)
     */
    bool isStatic() const;

    /**
     * @brief Sets the static state of this collider
     * @param isStatic New static state
     */
    void setStatic(bool isStatic);

    /**
     * @brief Gets the type identifier for this component
     * @return String "Collider"
     */
    std::string getType() const override;

    // Implement Component pure virtual functions
    void serialize(JsonBuilder& builder) const override;
    void deserialize(const JsonValue& value) override;

protected:
    bool m_isTrigger = false;  // If true, detect but don't resolve collisions
    bool m_isStatic  = false;  // If true, object cannot be moved by collisions
};

#endif