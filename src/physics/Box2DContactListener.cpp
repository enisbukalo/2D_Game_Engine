#include "Box2DContactListener.h"
#include "Entity.h"

void Box2DContactListener::onBeginContact(b2ShapeId shapeIdA, b2ShapeId shapeIdB)
{
    ::Entity::Entity* entityA = getEntityFromShape(shapeIdA);
    ::Entity::Entity* entityB = getEntityFromShape(shapeIdB);

    if (entityA && entityB)
    {
        // TODO: Trigger collision events in ECS
        // For now, this is a placeholder for future event system integration
    }
}

void Box2DContactListener::onEndContact(b2ShapeId shapeIdA, b2ShapeId shapeIdB)
{
    ::Entity::Entity* entityA = getEntityFromShape(shapeIdA);
    ::Entity::Entity* entityB = getEntityFromShape(shapeIdB);

    if (entityA && entityB)
    {
        // TODO: Trigger collision end events in ECS
        // For now, this is a placeholder for future event system integration
    }
}

::Entity::Entity* Box2DContactListener::getEntityFromShape(b2ShapeId shapeId)
{
    if (!b2Shape_IsValid(shapeId))
    {
        return nullptr;
    }

    b2BodyId bodyId = b2Shape_GetBody(shapeId);
    if (!b2Body_IsValid(bodyId))
    {
        return nullptr;
    }

    void* userData = b2Body_GetUserData(bodyId);
    return static_cast<::Entity::Entity*>(userData);
}
