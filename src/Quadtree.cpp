#include "Quadtree.h"
#include <unordered_set>
#include "CCircleCollider.h"
#include "CCollider.h"
#include "CTransform.h"

Quadtree::Quadtree(int level, const AABB& bounds) : m_level(level), m_bounds(bounds) {}

void Quadtree::clear()
{
    m_objects.clear();
    for (int i = 0; i < 4; i++)
    {
        if (m_children[i])
        {
            m_children[i]->clear();
            m_children[i].reset();
        }
    }
}

void Quadtree::split()
{
    Vec2 childSize   = m_bounds.halfSize;         // Child's full size is parent's halfSize
    Vec2 quarterSize = m_bounds.halfSize * 0.5f;  // Quarter of parent's full size for positioning

    // In Y-down coordinates: -Y is up (smaller Y), +Y is down (larger Y)
    m_children[0] = std::make_unique<Quadtree>(m_level + 1,
                                               AABB(m_bounds.position + Vec2(-quarterSize.x, -quarterSize.y), childSize));  // Top-left
    m_children[1] = std::make_unique<Quadtree>(m_level + 1,
                                               AABB(m_bounds.position + Vec2(quarterSize.x, -quarterSize.y), childSize));  // Top-right
    m_children[2] = std::make_unique<Quadtree>(m_level + 1,
                                               AABB(m_bounds.position + Vec2(-quarterSize.x, quarterSize.y), childSize));  // Bottom-left
    m_children[3] = std::make_unique<Quadtree>(m_level + 1,
                                               AABB(m_bounds.position + Vec2(quarterSize.x, quarterSize.y), childSize));  // Bottom-right
}

std::vector<int> Quadtree::getOverlappingQuadrants(const AABB& bounds) const
{
    std::vector<int> quadrants;

    // Check each quadrant
    // Note: In SFML/screen coordinates, Y increases DOWNWARD (Y=0 at top, larger Y at bottom)
    Vec2 topLeft     = bounds.position - bounds.halfSize;
    Vec2 bottomRight = bounds.position + bounds.halfSize;

    // Check top quadrants (smaller Y values, above center line)
    if (topLeft.y < m_bounds.position.y)
    {
        if (topLeft.x < m_bounds.position.x)
            quadrants.push_back(0);  // Top-left
        if (bottomRight.x > m_bounds.position.x)
            quadrants.push_back(1);  // Top-right
    }

    // Check bottom quadrants (larger Y values, below center line)
    if (bottomRight.y > m_bounds.position.y)
    {
        if (topLeft.x < m_bounds.position.x)
            quadrants.push_back(2);  // Bottom-left
        if (bottomRight.x > m_bounds.position.x)
            quadrants.push_back(3);  // Bottom-right
    }

    return quadrants;
}

void Quadtree::insert(Entity* entity)
{
    if (!entity)
    {
        return;
    }

    auto transform = entity->getComponent<CTransform>();
    auto collider  = entity->getComponentDerived<CCollider>();  // Use polymorphic getter
    if (!transform || !collider)
    {
        return;
    }

    // Get entity's AABB
    AABB entityBounds = collider->getBounds();

    // First check if the entity is within our bounds
    if (!m_bounds.intersects(entityBounds))
    {
        return;
    }

    // If we have children
    if (m_children[0])
    {
        // Get all quadrants this entity overlaps
        auto overlappingQuadrants = getOverlappingQuadrants(entityBounds);

        // Only delegate to child if entity fits entirely within a SINGLE quadrant
        // Entities spanning multiple quadrants stay at this level
        if (overlappingQuadrants.size() == 1)
        {
            m_children[overlappingQuadrants[0]]->insert(entity);
            return;
        }
    }

    // If we reach here, either:
    // 1. We have no children
    // 2. The entity is too large for children
    // 3. The entity spans multiple quadrants at the root level
    m_objects.push_back(entity);

    // Split if needed
    if (m_objects.size() > MAX_OBJECTS && m_level < MAX_LEVELS)
    {
        if (!m_children[0])
        {
            split();
        }

        // Try to redistribute existing objects
        auto it = m_objects.begin();
        while (it != m_objects.end())
        {
            Entity* obj         = *it;
            auto    objCollider = obj->getComponentDerived<CCollider>();  // Use polymorphic getter
            if (objCollider)
            {
                AABB objBounds    = objCollider->getBounds();
                auto objQuadrants = getOverlappingQuadrants(objBounds);

                // Only move down if the object fits entirely within a SINGLE child quadrant
                // Objects spanning multiple quadrants stay at this level
                if (objQuadrants.size() == 1)
                {
                    m_children[objQuadrants[0]]->insert(obj);
                    it = m_objects.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }
}

std::vector<Entity*> Quadtree::query(const AABB& area)
{
    std::vector<Entity*>        found;
    std::unordered_set<Entity*> uniqueEntities;  // Track unique entities

    if (!m_bounds.intersects(area))
    {
        return found;
    }

    // Add objects at this level
    for (auto* obj : m_objects)
    {
        auto collider = obj->getComponentDerived<CCollider>();  // Use polymorphic getter
        if (collider)
        {
            AABB objBounds = collider->getBounds();
            if (area.intersects(objBounds) && uniqueEntities.insert(obj).second)
            {
                found.push_back(obj);
            }
        }
    }

    // Check children
    if (m_children[0])
    {
        for (int i = 0; i < 4; i++)
        {
            auto childResults = m_children[i]->query(area);
            for (auto* result : childResults)
            {
                if (uniqueEntities.insert(result).second)
                {
                    found.push_back(result);
                }
            }
        }
    }

    return found;
}

void Quadtree::getAllBounds(std::vector<AABB>& bounds) const
{
    // Add this node's bounds
    bounds.push_back(m_bounds);

    // Recursively add children's bounds
    if (m_children[0])
    {
        for (int i = 0; i < 4; i++)
        {
            m_children[i]->getAllBounds(bounds);
        }
    }
}