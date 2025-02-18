#include "physics/Quadtree.h"
#include "components/CTransform.h"

Quadtree::Quadtree(int level, const AABB& bounds)
    : m_level(level), m_bounds(bounds) {}

void Quadtree::clear() {
    m_objects.clear();
    for (int i = 0; i < 4; i++) {
        if (m_children[i]) {
            m_children[i]->clear();
            m_children[i].reset();
        }
    }
}

void Quadtree::split() {
    Vec2 childSize = m_bounds.halfSize * 0.5f;
    Vec2 quarterSize = childSize * 0.5f;

    m_children[0] = std::make_unique<Quadtree>(m_level + 1,
        AABB(m_bounds.position + Vec2(-quarterSize.x, quarterSize.y), childSize));
    m_children[1] = std::make_unique<Quadtree>(m_level + 1,
        AABB(m_bounds.position + Vec2(quarterSize.x, quarterSize.y), childSize));
    m_children[2] = std::make_unique<Quadtree>(m_level + 1,
        AABB(m_bounds.position + Vec2(-quarterSize.x, -quarterSize.y), childSize));
    m_children[3] = std::make_unique<Quadtree>(m_level + 1,
        AABB(m_bounds.position + Vec2(quarterSize.x, -quarterSize.y), childSize));
}

int Quadtree::getQuadrant(const Vec2& position) const {
    if (position.y > m_bounds.position.y) {
        if (position.x < m_bounds.position.x) return 0;
        return 1;
    }
    if (position.x < m_bounds.position.x) return 2;
    return 3;
}

void Quadtree::insert(Entity* entity) {
    if (!entity) return;

    auto transform = entity->getComponent<CTransform>();
    if (!transform) return;

    Vec2 position = transform->getPosition();

    // If we have children, insert into them
    if (m_children[0]) {
        int quadrant = getQuadrant(position);
        if (m_bounds.contains(position)) {
            m_children[quadrant]->insert(entity);
            return;
        }
    }

    // Add object to this node
    m_objects.push_back(entity);

    // Split if needed
    if (m_objects.size() > MAX_OBJECTS && m_level < MAX_LEVELS) {
        if (!m_children[0]) {
            split();
        }

        // Redistribute existing objects
        auto it = m_objects.begin();
        while (it != m_objects.end()) {
            Entity* obj = *it;
            auto objTransform = obj->getComponent<CTransform>();
            if (objTransform) {
                Vec2 objPos = objTransform->getPosition();
                if (m_bounds.contains(objPos)) {
                    int quadrant = getQuadrant(objPos);
                    m_children[quadrant]->insert(obj);
                    it = m_objects.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }
}

std::vector<Entity*> Quadtree::query(const AABB& area) {
    std::vector<Entity*> found;

    if (!m_bounds.intersects(area)) {
        return found;
    }

    // Add objects at this level
    for (auto* obj : m_objects) {
        auto transform = obj->getComponent<CTransform>();
        if (transform) {
            Vec2 pos = transform->getPosition();
            if (area.contains(pos)) {
                found.push_back(obj);
            }
        }
    }

    // Check children
    if (m_children[0]) {
        for (int i = 0; i < 4; i++) {
            auto childResults = m_children[i]->query(area);
            found.insert(found.end(), childResults.begin(), childResults.end());
        }
    }

    return found;
}