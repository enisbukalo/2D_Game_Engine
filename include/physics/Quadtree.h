#ifndef QUADTREE_H
#define QUADTREE_H

#include <memory>
#include <vector>
#include "Entity.h"
#include "Vec2.h"

/**
 * @brief Represents a rectangular boundary for quadtree nodes
 *
 * An Axis-Aligned Bounding Box (AABB) used to define the boundaries
 * of quadtree nodes and for spatial queries. The AABB is defined by
 * its center position and half-size dimensions.
 */
struct AABB
{
    Vec2 position;  // Center position
    Vec2 halfSize;  // Half-width and half-height

    /**
     * @brief Constructs an AABB with given position and size
     * @param pos Center position of the AABB
     * @param size Full width and height (will be halved internally)
     */
    AABB(const Vec2& pos, const Vec2& size);

    /**
     * @brief Checks if a point is inside this AABB
     * @param point The point to check
     * @return true if the point is inside the AABB
     */
    bool contains(const Vec2& point) const;

    /**
     * @brief Checks if this AABB overlaps with another
     * @param other The other AABB to check against
     * @return true if the AABBs intersect
     */
    bool intersects(const AABB& other) const;
};

/**
 * @brief Quadtree data structure for spatial partitioning
 *
 * A quadtree recursively subdivides space into four quadrants to optimize
 * spatial queries. Each node can contain up to MAX_OBJECTS before subdividing,
 * and the tree can grow up to MAX_LEVELS deep. This implementation is used
 * for efficient collision detection and spatial queries in the physics system.
 */
class Quadtree
{
public:
    static const int MAX_OBJECTS = 8;  ///< Maximum objects before subdivision
    static const int MAX_LEVELS  = 5;  ///< Maximum depth of the tree

    /**
     * @brief Constructs a quadtree node
     * @param level Current depth level of this node
     * @param bounds Spatial boundaries of this node
     */
    Quadtree(int level, const AABB& bounds);
    ~Quadtree() = default;

    /**
     * @brief Removes all entities and clears subdivisions
     */
    void clear();

    /**
     * @brief Inserts an entity into the quadtree
     * @param entity Pointer to the entity to insert
     *
     * The entity must have a CTransform component. It will be inserted
     * into the appropriate quadrant based on its position.
     */
    void insert(Entity* entity);

    /**
     * @brief Queries entities within a given area
     * @param area The AABB defining the query area
     * @return Vector of entity pointers within the area
     */
    std::vector<Entity*> query(const AABB& area);

private:
    int                       m_level;        ///< Current depth level
    std::vector<Entity*>      m_objects;      ///< Entities at this node
    AABB                      m_bounds;       ///< Spatial bounds of this node
    std::unique_ptr<Quadtree> m_children[4];  ///< Child nodes (null if leaf)

    /**
     * @brief Subdivides this node into four children
     */
    void split();

    /**
     * @brief Determines which quadrant a position belongs to
     * @param position The position to check
     * @return Index (0-3) of the appropriate quadrant
     */
    int getQuadrant(const Vec2& position) const;
};

#endif  // QUADTREE_H