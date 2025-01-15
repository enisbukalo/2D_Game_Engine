#include "../components/Component.h"
#include "../include/Entity.h"

void CGravity::update(float deltaTime)
{
    if (owner && owner->getComponent<CTransform>())
    {
        CTransform *transform = owner->getComponent<CTransform>();
        transform->velocity += force * deltaTime;
    }
}