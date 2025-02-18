#ifndef CCOLLIDER_H
#define CCOLLIDER_H

#include "Component.h"
#include "Vec2.h"
#include "physics/Quadtree.h"

class CCollider : public Component
{
public:
    virtual ~CCollider() override = default;

    // Get the AABB bounds for broad-phase collision detection
    virtual AABB getBounds() const = 0;

    // Detailed collision check for specific shape types
    virtual bool intersects(const CCollider* other) const = 0;

    // Optional: collision response properties
    bool isTrigger() const;
    void setTrigger(bool trigger);

    // Implement Component pure virtual functions
    void        serialize(JsonBuilder& builder) const override;
    void        deserialize(const JsonValue& value) override;
    std::string getType() const override
    {
        return "Collider";
    }

protected:
    bool m_isTrigger = false;  // If true, detect but don't resolve collisions
};

#endif