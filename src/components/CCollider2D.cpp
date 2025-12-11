#include "CCollider2D.h"
#include <limits>

namespace Components
{

CCollider2D::CCollider2D()
    : m_isSensor(false), m_density(1.0f), m_friction(0.3f), m_restitution(0.0f), m_initialized(false)
{
    // Fixtures vector starts empty
}

CCollider2D::~CCollider2D() = default;

void CCollider2D::createCircle(float radius, const b2Vec2& center)
{
    m_fixtures.clear();

    ShapeFixture fixture;
    fixture.shapeType               = ColliderShape::Circle;
    fixture.shapeData.circle.center = center;
    fixture.shapeData.circle.radius = radius;

    m_fixtures.push_back(fixture);
    m_initialized = !m_fixtures.empty();
}

void CCollider2D::createBox(float halfWidth, float halfHeight)
{
    m_fixtures.clear();

    ShapeFixture fixture;
    fixture.shapeType                = ColliderShape::Box;
    fixture.shapeData.box.halfWidth  = halfWidth;
    fixture.shapeData.box.halfHeight = halfHeight;

    m_fixtures.push_back(fixture);
    m_initialized = !m_fixtures.empty();
}

void CCollider2D::createPolygon(const b2Vec2* vertices, int count, float radius)
{
    if (!vertices || count < 3 || count > B2_MAX_POLYGON_VERTICES)
    {
        // Invalid input, cannot create polygon
        return;
    }

    // Compute convex hull from input vertices
    b2Hull hull = b2ComputeHull(vertices, count);

    if (hull.count < 3)
    {
        // Hull computation failed
        return;
    }

    m_fixtures.clear();

    // Create first fixture with this polygon
    ShapeFixture fixture;
    fixture.shapeType                     = ColliderShape::Polygon;
    fixture.shapeData.polygon.vertexCount = hull.count;
    fixture.shapeData.polygon.radius      = radius;
    for (int i = 0; i < hull.count; ++i)
    {
        fixture.shapeData.polygon.vertices[i] = hull.points[i];
    }
    m_fixtures.push_back(fixture);
    m_initialized = !m_fixtures.empty();
}

void CCollider2D::addPolygon(const b2Vec2* vertices, int count, float radius)
{
    if (!vertices || count < 3 || count > B2_MAX_POLYGON_VERTICES)
    {
        // Invalid input, cannot create polygon
        return;
    }

    // Compute convex hull from input vertices
    b2Hull hull = b2ComputeHull(vertices, count);

    if (hull.count < 3)
    {
        // Hull computation failed
        return;
    }

    // Add additional fixture
    ShapeFixture fixture;
    fixture.shapeType                     = ColliderShape::Polygon;
    fixture.shapeData.polygon.vertexCount = hull.count;
    fixture.shapeData.polygon.radius      = radius;
    for (int i = 0; i < hull.count; ++i)
    {
        fixture.shapeData.polygon.vertices[i] = hull.points[i];
    }
    m_fixtures.push_back(fixture);
    m_initialized = !m_fixtures.empty();
}

void CCollider2D::createPolygonFromHull(const b2Hull& hull, float radius)
{
    if (hull.count < 3 || hull.count > B2_MAX_POLYGON_VERTICES)
    {
        return;
    }

    m_fixtures.clear();

    ShapeFixture fixture;
    fixture.shapeType                     = ColliderShape::Polygon;
    fixture.shapeData.polygon.vertexCount = hull.count;
    fixture.shapeData.polygon.radius      = radius;
    for (int i = 0; i < hull.count; ++i)
    {
        fixture.shapeData.polygon.vertices[i] = hull.points[i];
    }
    m_fixtures.push_back(fixture);
    m_initialized = !m_fixtures.empty();
}

void CCollider2D::createOffsetPolygon(const b2Hull& hull, const b2Vec2& position, float rotation, float radius)
{
    if (hull.count < 3 || hull.count > B2_MAX_POLYGON_VERTICES)
    {
        return;
    }

    m_fixtures.clear();

    // Create offset polygon using Box2D function
    b2Rot     rot        = b2MakeRot(rotation);
    b2Polygon offsetPoly = radius > 0.0f ? b2MakeOffsetRoundedPolygon(&hull, position, rot, radius)
                                         : b2MakeOffsetPolygon(&hull, position, rot);

    // Store the transformed vertices
    ShapeFixture fixture;
    fixture.shapeType                     = ColliderShape::Polygon;
    fixture.shapeData.polygon.vertexCount = offsetPoly.count;
    fixture.shapeData.polygon.radius      = offsetPoly.radius;
    for (int i = 0; i < offsetPoly.count; ++i)
    {
        fixture.shapeData.polygon.vertices[i] = offsetPoly.vertices[i];
    }
    m_fixtures.push_back(fixture);
    m_initialized = !m_fixtures.empty();
}

void CCollider2D::createSegment(const b2Vec2& point1, const b2Vec2& point2)
{
    m_fixtures.clear();

    ShapeFixture fixture;
    fixture.shapeType                = ColliderShape::Segment;
    fixture.shapeData.segment.point1 = point1;
    fixture.shapeData.segment.point2 = point2;
    m_fixtures.push_back(fixture);
    m_initialized = !m_fixtures.empty();
}

void CCollider2D::addSegment(const b2Vec2& point1, const b2Vec2& point2)
{
    // Add additional segment fixture
    ShapeFixture fixture;
    fixture.shapeType                = ColliderShape::Segment;
    fixture.shapeData.segment.point1 = point1;
    fixture.shapeData.segment.point2 = point2;
    m_fixtures.push_back(fixture);
    m_initialized = !m_fixtures.empty();
}

void CCollider2D::createChainSegment(const b2Vec2& ghost1, const b2Vec2& point1, const b2Vec2& point2, const b2Vec2& ghost2)
{
    m_fixtures.clear();

    ShapeFixture fixture;
    fixture.shapeType                     = ColliderShape::ChainSegment;
    fixture.shapeData.chainSegment.ghost1 = ghost1;
    fixture.shapeData.chainSegment.point1 = point1;
    fixture.shapeData.chainSegment.point2 = point2;
    fixture.shapeData.chainSegment.ghost2 = ghost2;
    m_fixtures.push_back(fixture);
    m_initialized = !m_fixtures.empty();
}

void CCollider2D::addChainSegment(const b2Vec2& ghost1, const b2Vec2& point1, const b2Vec2& point2, const b2Vec2& ghost2)
{
    // Add additional chain segment fixture
    ShapeFixture fixture;
    fixture.shapeType                     = ColliderShape::ChainSegment;
    fixture.shapeData.chainSegment.ghost1 = ghost1;
    fixture.shapeData.chainSegment.point1 = point1;
    fixture.shapeData.chainSegment.point2 = point2;
    fixture.shapeData.chainSegment.ghost2 = ghost2;
    m_fixtures.push_back(fixture);
    m_initialized = !m_fixtures.empty();
}

void CCollider2D::setIsSensor(bool isSensor)
{
    m_isSensor = isSensor;
}

void CCollider2D::setDensity(float density)
{
    m_density = density;
}

void CCollider2D::setFriction(float friction)
{
    m_friction = friction;
}

void CCollider2D::setRestitution(float restitution)
{
    m_restitution = restitution;
}

float CCollider2D::getCircleRadius() const
{
    if (!m_fixtures.empty() && m_fixtures[0].shapeType == ColliderShape::Circle)
    {
        return m_fixtures[0].shapeData.circle.radius;
    }
    return 0.0f;
}

b2Vec2 CCollider2D::getCircleCenter() const
{
    if (!m_fixtures.empty() && m_fixtures[0].shapeType == ColliderShape::Circle)
    {
        return m_fixtures[0].shapeData.circle.center;
    }
    return {0.0f, 0.0f};
}

float CCollider2D::getBoxHalfWidth() const
{
    if (!m_fixtures.empty() && m_fixtures[0].shapeType == ColliderShape::Box)
    {
        return m_fixtures[0].shapeData.box.halfWidth;
    }
    return 0.0f;
}

float CCollider2D::getBoxHalfHeight() const
{
    if (!m_fixtures.empty() && m_fixtures[0].shapeType == ColliderShape::Box)
    {
        return m_fixtures[0].shapeData.box.halfHeight;
    }
    return 0.0f;
}

const b2Vec2* CCollider2D::getPolygonVertices(size_t fixtureIndex) const
{
    if (fixtureIndex < m_fixtures.size() && m_fixtures[fixtureIndex].shapeType == ColliderShape::Polygon)
    {
        return m_fixtures[fixtureIndex].shapeData.polygon.vertices;
    }
    return nullptr;
}

int CCollider2D::getPolygonVertexCount(size_t fixtureIndex) const
{
    if (fixtureIndex < m_fixtures.size() && m_fixtures[fixtureIndex].shapeType == ColliderShape::Polygon)
    {
        return m_fixtures[fixtureIndex].shapeData.polygon.vertexCount;
    }
    return 0;
}

float CCollider2D::getPolygonRadius(size_t fixtureIndex) const
{
    if (fixtureIndex < m_fixtures.size() && m_fixtures[fixtureIndex].shapeType == ColliderShape::Polygon)
    {
        return m_fixtures[fixtureIndex].shapeData.polygon.radius;
    }
    return 0.0f;
}

void CCollider2D::init()
{
    m_initialized = !m_fixtures.empty();
}

bool CCollider2D::getBounds(float& outWidth, float& outHeight) const
{
    if (m_fixtures.empty())
    {
        outWidth  = 0.0f;
        outHeight = 0.0f;
        return false;
    }

    // Initialize bounds to extreme values
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

    // Iterate through all fixtures and find min/max bounds
    for (const auto& fixture : m_fixtures)
    {
        switch (fixture.shapeType)
        {
            case ColliderShape::Circle:
            {
                float cx = fixture.shapeData.circle.center.x;
                float cy = fixture.shapeData.circle.center.y;
                float r  = fixture.shapeData.circle.radius;
                minX     = std::min(minX, cx - r);
                maxX     = std::max(maxX, cx + r);
                minY     = std::min(minY, cy - r);
                maxY     = std::max(maxY, cy + r);
            }
            break;

            case ColliderShape::Box:
            {
                float hw = fixture.shapeData.box.halfWidth;
                float hh = fixture.shapeData.box.halfHeight;
                minX     = std::min(minX, -hw);
                maxX     = std::max(maxX, hw);
                minY     = std::min(minY, -hh);
                maxY     = std::max(maxY, hh);
            }
            break;

            case ColliderShape::Polygon:
            {
                int vertexCount = fixture.shapeData.polygon.vertexCount;
                for (int i = 0; i < vertexCount; ++i)
                {
                    float vx = fixture.shapeData.polygon.vertices[i].x;
                    float vy = fixture.shapeData.polygon.vertices[i].y;
                    minX     = std::min(minX, vx);
                    maxX     = std::max(maxX, vx);
                    minY     = std::min(minY, vy);
                    maxY     = std::max(maxY, vy);
                }
            }
            break;

            case ColliderShape::Segment:
            {
                float x1 = fixture.shapeData.segment.point1.x;
                float y1 = fixture.shapeData.segment.point1.y;
                float x2 = fixture.shapeData.segment.point2.x;
                float y2 = fixture.shapeData.segment.point2.y;
                minX     = std::min(minX, std::min(x1, x2));
                maxX     = std::max(maxX, std::max(x1, x2));
                minY     = std::min(minY, std::min(y1, y2));
                maxY     = std::max(maxY, std::max(y1, y2));
            }
            break;

            case ColliderShape::ChainSegment:
            {
                // For chain segments, consider all points including ghosts
                float x1 = fixture.shapeData.chainSegment.point1.x;
                float y1 = fixture.shapeData.chainSegment.point1.y;
                float x2 = fixture.shapeData.chainSegment.point2.x;
                float y2 = fixture.shapeData.chainSegment.point2.y;
                minX     = std::min(minX, std::min(x1, x2));
                maxX     = std::max(maxX, std::max(x1, x2));
                minY     = std::min(minY, std::min(y1, y2));
                maxY     = std::max(maxY, std::max(y1, y2));
            }
            break;
        }
    }

    outWidth  = maxX - minX;
    outHeight = maxY - minY;
    return true;
}

}  // namespace Components
