#include "components/CCollider.h"

bool CCollider::isTrigger() const
{
    return m_isTrigger;
}

void CCollider::setTrigger(bool trigger)
{
    m_isTrigger = trigger;
}

void CCollider::serialize(JsonBuilder& builder) const
{
    // Base collider doesn't need to serialize anything since it's abstract
    // Derived classes will handle their own serialization
}

void CCollider::deserialize(const JsonValue& value)
{
    // Base collider doesn't need to deserialize anything since it's abstract
    // Derived classes will handle their own deserialization
}

std::string CCollider::getType() const
{
    return "Collider";
}