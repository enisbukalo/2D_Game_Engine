#ifndef CCIRCLECOLLIDER_H
#define CCIRCLECOLLIDER_H

#include "CCollider.h"

class CCircleCollider : public CCollider
{
public:
    CCircleCollider() : m_radius(1.0f) {}
    explicit CCircleCollider(float radius);

    AABB getBounds() const override;
    bool intersects(const CCollider* other) const override;

    float getRadius() const;
    void  setRadius(float radius);

    // Override Component serialization methods
    void        serialize(JsonBuilder& builder) const override;
    void        deserialize(const JsonValue& value) override;
    std::string getType() const override
    {
        return "CircleCollider";
    }

private:
    float m_radius;
    bool  circleVsCircle(const CCircleCollider* other) const;
};

#endif