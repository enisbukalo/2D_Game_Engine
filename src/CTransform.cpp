#include "CTransform.h"

void CTransform::update(float deltaTime)
{
    position += velocity * deltaTime;
}

std::string CTransform::getType() const
{
    return "Transform";
}

void CTransform::serialize() const {}

void CTransform::deserialize() {}