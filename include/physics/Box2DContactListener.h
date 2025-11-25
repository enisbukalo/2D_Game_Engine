#pragma once

#include "box2d/box2d.h"

class Entity;

/**
 * @brief Contact Listener for Box2D collision events
 *
 * This class provides callbacks for collision events in the Box2D world.
 * Users can derive from this class to implement custom collision handling logic.
 *
 * Events:
 * - BeginContact: Called when two shapes start touching
 * - EndContact: Called when two shapes stop touching
 * - PreSolve: Called before collision resolution (can modify contact)
 * - PostSolve: Called after collision resolution (can read impulses)
 */
class Box2DContactListener
{
public:
    virtual ~Box2DContactListener() = default;

    /**
     * @brief Called when two shapes begin to touch
     * @param shapeIdA First shape in the contact
     * @param shapeIdB Second shape in the contact
     */
    virtual void onBeginContact(b2ShapeId shapeIdA, b2ShapeId shapeIdB);

    /**
     * @brief Called when two shapes cease to touch
     * @param shapeIdA First shape in the contact
     * @param shapeIdB Second shape in the contact
     */
    virtual void onEndContact(b2ShapeId shapeIdA, b2ShapeId shapeIdB);

protected:
    /**
     * @brief Get the entity associated with a shape
     * @param shapeId Shape to query
     * @return Entity pointer or nullptr if not found
     */
    Entity* getEntityFromShape(b2ShapeId shapeId);
};
