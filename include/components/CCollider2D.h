// Data-only collider component.
//
// Stores authoring/configuration data for collider shapes in engine primitives.
// Any physics-backend-specific object creation lives in systems (e.g., S2DPhysics).

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>
#include "Vec2.h"

namespace Components
{

enum class ColliderShape
{
    Circle,
    Box,
    Polygon,
    Segment,
    ChainSegment
};

struct ShapeFixture
{
    struct Circle
    {
        Vec2  center{0.0f, 0.0f};
        float radius{0.0f};
    } circle;

    struct Box
    {
        float halfWidth{0.0f};
        float halfHeight{0.0f};
    } box;

    struct Polygon
    {
        std::vector<Vec2> vertices;
        float             radius{0.0f};
    } polygon;

    struct Segment
    {
        Vec2 point1{0.0f, 0.0f};
        Vec2 point2{0.0f, 0.0f};
    } segment;

    struct ChainSegment
    {
        Vec2 ghost1{0.0f, 0.0f};
        Vec2 point1{0.0f, 0.0f};
        Vec2 point2{0.0f, 0.0f};
        Vec2 ghost2{0.0f, 0.0f};
    } chainSegment;

    ColliderShape shapeType{ColliderShape::Box};
};

struct CCollider2D
{
    // Fixtures (multiple fixtures per entity supported)
    std::vector<ShapeFixture> fixtures;

    // Shared material properties
    bool  sensor{false};
    float density{1.0f};
    float friction{0.3f};
    float restitution{0.0f};

    inline void clear() { fixtures.clear(); }

    inline void createCircle(float radius, const Vec2& center = Vec2{0.0f, 0.0f})
    {
        fixtures.clear();
        ShapeFixture f;
        f.shapeType          = ColliderShape::Circle;
        f.circle.center      = center;
        f.circle.radius      = radius;
        fixtures.push_back(std::move(f));
    }

    inline void createBox(float halfWidth, float halfHeight)
    {
        fixtures.clear();
        ShapeFixture f;
        f.shapeType       = ColliderShape::Box;
        f.box.halfWidth   = halfWidth;
        f.box.halfHeight  = halfHeight;
        fixtures.push_back(std::move(f));
    }

    inline void createPolygon(const std::vector<Vec2>& vertices, float radius = 0.0f)
    {
        fixtures.clear();
        addPolygon(vertices, radius);
    }

    inline void addPolygon(const std::vector<Vec2>& vertices, float radius = 0.0f)
    {
        ShapeFixture f;
        f.shapeType           = ColliderShape::Polygon;
        f.polygon.vertices    = vertices;
        f.polygon.radius      = radius;
        fixtures.push_back(std::move(f));
    }

    inline void createSegment(const Vec2& point1, const Vec2& point2)
    {
        fixtures.clear();
        addSegment(point1, point2);
    }

    inline void addSegment(const Vec2& point1, const Vec2& point2)
    {
        ShapeFixture f;
        f.shapeType         = ColliderShape::Segment;
        f.segment.point1    = point1;
        f.segment.point2    = point2;
        fixtures.push_back(std::move(f));
    }

    inline void createChainSegment(const Vec2& ghost1, const Vec2& point1, const Vec2& point2, const Vec2& ghost2)
    {
        fixtures.clear();
        addChainSegment(ghost1, point1, point2, ghost2);
    }

    inline void addChainSegment(const Vec2& ghost1, const Vec2& point1, const Vec2& point2, const Vec2& ghost2)
    {
        ShapeFixture f;
        f.shapeType                 = ColliderShape::ChainSegment;
        f.chainSegment.ghost1       = ghost1;
        f.chainSegment.point1       = point1;
        f.chainSegment.point2       = point2;
        f.chainSegment.ghost2       = ghost2;
        fixtures.push_back(std::move(f));
    }

    inline ColliderShape getShapeType() const
    {
        return fixtures.empty() ? ColliderShape::Box : fixtures[0].shapeType;
    }

    inline const std::vector<ShapeFixture>& getFixtures() const { return fixtures; }
    inline size_t getFixtureCount() const { return fixtures.size(); }

    inline void setIsSensor(bool isSensor) { sensor = isSensor; }
    inline bool isSensor() const { return sensor; }

    inline void setDensity(float d) { density = d; }
    inline float getDensity() const { return density; }
    inline void setFriction(float f) { friction = f; }
    inline float getFriction() const { return friction; }
    inline void setRestitution(float r) { restitution = r; }
    inline float getRestitution() const { return restitution; }

    inline float getCircleRadius() const
    {
        if (!fixtures.empty() && fixtures[0].shapeType == ColliderShape::Circle)
        {
            return fixtures[0].circle.radius;
        }
        return 0.0f;
    }

    inline Vec2 getCircleCenter() const
    {
        if (!fixtures.empty() && fixtures[0].shapeType == ColliderShape::Circle)
        {
            return fixtures[0].circle.center;
        }
        return Vec2{0.0f, 0.0f};
    }

    inline float getBoxHalfWidth() const
    {
        if (!fixtures.empty() && fixtures[0].shapeType == ColliderShape::Box)
        {
            return fixtures[0].box.halfWidth;
        }
        return 0.0f;
    }

    inline float getBoxHalfHeight() const
    {
        if (!fixtures.empty() && fixtures[0].shapeType == ColliderShape::Box)
        {
            return fixtures[0].box.halfHeight;
        }
        return 0.0f;
    }

    inline const std::vector<Vec2>& getPolygonVertices(size_t fixtureIndex = 0) const
    {
        static const std::vector<Vec2> empty;
        if (fixtureIndex < fixtures.size() && fixtures[fixtureIndex].shapeType == ColliderShape::Polygon)
        {
            return fixtures[fixtureIndex].polygon.vertices;
        }
        return empty;
    }

    inline int getPolygonVertexCount(size_t fixtureIndex = 0) const
    {
        const auto& verts = getPolygonVertices(fixtureIndex);
        return static_cast<int>(verts.size());
    }

    inline float getPolygonRadius(size_t fixtureIndex = 0) const
    {
        if (fixtureIndex < fixtures.size() && fixtures[fixtureIndex].shapeType == ColliderShape::Polygon)
        {
            return fixtures[fixtureIndex].polygon.radius;
        }
        return 0.0f;
    }

    inline bool getBounds(float& outWidth, float& outHeight) const
    {
        if (fixtures.empty())
        {
            outWidth = 0.0f;
            outHeight = 0.0f;
            return false;
        }

        float minX = std::numeric_limits<float>::infinity();
        float minY = std::numeric_limits<float>::infinity();
        float maxX = -std::numeric_limits<float>::infinity();
        float maxY = -std::numeric_limits<float>::infinity();

        auto includePoint = [&](const Vec2& p)
        {
            minX = std::min(minX, p.x);
            minY = std::min(minY, p.y);
            maxX = std::max(maxX, p.x);
            maxY = std::max(maxY, p.y);
        };

        for (const auto& f : fixtures)
        {
            switch (f.shapeType)
            {
                case ColliderShape::Circle:
                {
                    const Vec2 c = f.circle.center;
                    const float r = f.circle.radius;
                    includePoint(Vec2{c.x - r, c.y - r});
                    includePoint(Vec2{c.x + r, c.y + r});
                    break;
                }
                case ColliderShape::Box:
                {
                    const float hw = f.box.halfWidth;
                    const float hh = f.box.halfHeight;
                    includePoint(Vec2{-hw, -hh});
                    includePoint(Vec2{hw, hh});
                    break;
                }
                case ColliderShape::Polygon:
                {
                    for (const auto& v : f.polygon.vertices)
                    {
                        includePoint(v);
                    }
                    break;
                }
                case ColliderShape::Segment:
                {
                    includePoint(f.segment.point1);
                    includePoint(f.segment.point2);
                    break;
                }
                case ColliderShape::ChainSegment:
                {
                    includePoint(f.chainSegment.ghost1);
                    includePoint(f.chainSegment.point1);
                    includePoint(f.chainSegment.point2);
                    includePoint(f.chainSegment.ghost2);
                    break;
                }
            }
        }

        outWidth  = (maxX - minX);
        outHeight = (maxY - minY);
        return std::isfinite(outWidth) && std::isfinite(outHeight);
    }
};

}  // namespace Components
