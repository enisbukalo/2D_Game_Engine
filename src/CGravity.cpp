#include "../components/CGravity.h"
#include "../include/Entity.h"

void CGravity::update(float deltaTime)
{
    if (owner && owner->getComponent<CTransform>())
    {
        auto transform = owner->getComponent<CTransform>();
        transform->velocity += force * deltaTime;
    }
}