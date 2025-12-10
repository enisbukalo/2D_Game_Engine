#include "Box2DContactListener.h"

void Box2DContactListener::onBeginContact(b2ShapeId shapeIdA, b2ShapeId shapeIdB)
{
    Entity entityA = getEntityFromShape(shapeIdA);
    Entity entityB = getEntityFromShape(shapeIdB);

    if (entityA.isValid() && entityB.isValid())
    {
        // TODO: Trigger collision events in ECS
        // For now, this is a placeholder for future event system integration
    }
}

void Box2DContactListener::onEndContact(b2ShapeId shapeIdA, b2ShapeId shapeIdB)
{
    Entity entityA = getEntityFromShape(shapeIdA);
    Entity entityB = getEntityFromShape(shapeIdB);

    if (entityA.isValid() && entityB.isValid())
    {
        // TODO: Trigger collision end events in ECS
        // For now, this is a placeholder for future event system integration
    }
}

Entity Box2DContactListener::getEntityFromShape(b2ShapeId shapeId)
{
    if (!b2Shape_IsValid(shapeId))
    {
        return Entity::null();
    }

    b2BodyId bodyId = b2Shape_GetBody(shapeId);
    if (!b2Body_IsValid(bodyId))
    {
        return Entity::null();
    }

    // TODO: Store Entity ID in Box2D body user data
    // For now, return null entity
    return Entity::null();
}
