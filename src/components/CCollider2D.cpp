#include "CCollider2D.h"
#include "CPhysicsBody2D.h"
#include "Entity.h"

CCollider2D::CCollider2D()
    : m_isSensor(false),
      m_density(1.0f),
      m_friction(0.3f),
      m_restitution(0.0f),
      m_initialized(false)
{
    // Fixtures vector starts empty
}

CCollider2D::~CCollider2D()
{
    destroyShape();
}

void CCollider2D::createCircle(float radius, const b2Vec2& center)
{
    // Clear existing fixtures
    destroyShape();
    m_fixtures.clear();
    
    ShapeFixture fixture;
    fixture.shapeType = ColliderShape::Circle;
    fixture.shapeData.circle.center = center;
    fixture.shapeData.circle.radius = radius;
    fixture.shapeId = b2_nullShapeId;
    
    m_fixtures.push_back(fixture);
    attachToBody();
}

void CCollider2D::createBox(float halfWidth, float halfHeight)
{
    // Clear existing fixtures
    destroyShape();
    m_fixtures.clear();
    
    ShapeFixture fixture;
    fixture.shapeType = ColliderShape::Box;
    fixture.shapeData.box.halfWidth = halfWidth;
    fixture.shapeData.box.halfHeight = halfHeight;
    fixture.shapeId = b2_nullShapeId;
    
    m_fixtures.push_back(fixture);
    attachToBody();
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

    // Clear existing fixtures
    destroyShape();
    m_fixtures.clear();
    
    // Create first fixture with this polygon
    ShapeFixture fixture;
    fixture.shapeType = ColliderShape::Polygon;
    fixture.shapeData.polygon.vertexCount = hull.count;
    fixture.shapeData.polygon.radius = radius;
    for (int i = 0; i < hull.count; ++i)
    {
        fixture.shapeData.polygon.vertices[i] = hull.points[i];
    }
    fixture.shapeId = b2_nullShapeId;
    
    m_fixtures.push_back(fixture);
    attachToBody();
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
    fixture.shapeType = ColliderShape::Polygon;
    fixture.shapeData.polygon.vertexCount = hull.count;
    fixture.shapeData.polygon.radius = radius;
    for (int i = 0; i < hull.count; ++i)
    {
        fixture.shapeData.polygon.vertices[i] = hull.points[i];
    }
    fixture.shapeId = b2_nullShapeId;
    
    m_fixtures.push_back(fixture);
    
    // Attach only the new fixture to body
    if (!getOwner())
    {
        return;
    }

    auto physicsBody = getOwner()->getComponent<CPhysicsBody2D>();
    if (!physicsBody || !physicsBody->isInitialized())
    {
        return;
    }

    b2BodyId bodyId = physicsBody->getBodyId();
    if (!b2Body_IsValid(bodyId))
    {
        return;
    }

    // Create shape for the new fixture
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = m_density;
    shapeDef.enableSensorEvents = m_isSensor;
    shapeDef.invokeContactCreation = true;

    // Reconstruct hull from stored vertices
    b2Hull newHull;
    newHull.count = fixture.shapeData.polygon.vertexCount;
    for (int i = 0; i < newHull.count; ++i)
    {
        newHull.points[i] = fixture.shapeData.polygon.vertices[i];
    }

    // Create polygon from hull
    b2Polygon polygon = b2MakePolygon(&newHull, fixture.shapeData.polygon.radius);
    b2ShapeId newShapeId = b2CreatePolygonShape(bodyId, &shapeDef, &polygon);

    // Update the fixture's shape ID
    m_fixtures.back().shapeId = newShapeId;
    m_initialized = true;

    // Set friction and restitution
    if (b2Shape_IsValid(newShapeId))
    {
        b2Shape_SetFriction(newShapeId, m_friction);
        b2Shape_SetRestitution(newShapeId, m_restitution);
    }
}

void CCollider2D::createPolygonFromHull(const b2Hull& hull, float radius)
{
    if (hull.count < 3 || hull.count > B2_MAX_POLYGON_VERTICES)
    {
        return;
    }

    // Clear existing fixtures
    destroyShape();
    m_fixtures.clear();
    
    ShapeFixture fixture;
    fixture.shapeType = ColliderShape::Polygon;
    fixture.shapeData.polygon.vertexCount = hull.count;
    fixture.shapeData.polygon.radius = radius;
    for (int i = 0; i < hull.count; ++i)
    {
        fixture.shapeData.polygon.vertices[i] = hull.points[i];
    }
    fixture.shapeId = b2_nullShapeId;
    
    m_fixtures.push_back(fixture);
    attachToBody();
}

void CCollider2D::createOffsetPolygon(const b2Hull& hull, const b2Vec2& position, float rotation,
                                      float radius)
{
    if (hull.count < 3 || hull.count > B2_MAX_POLYGON_VERTICES)
    {
        return;
    }

    // Clear existing fixtures
    destroyShape();
    m_fixtures.clear();

    // Create offset polygon using Box2D function
    b2Rot rot = b2MakeRot(rotation);
    b2Polygon offsetPoly =
        radius > 0.0f ? b2MakeOffsetRoundedPolygon(&hull, position, rot, radius)
                      : b2MakeOffsetPolygon(&hull, position, rot);

    // Store the transformed vertices
    ShapeFixture fixture;
    fixture.shapeType = ColliderShape::Polygon;
    fixture.shapeData.polygon.vertexCount = offsetPoly.count;
    fixture.shapeData.polygon.radius = offsetPoly.radius;
    for (int i = 0; i < offsetPoly.count; ++i)
    {
        fixture.shapeData.polygon.vertices[i] = offsetPoly.vertices[i];
    }
    fixture.shapeId = b2_nullShapeId;
    
    m_fixtures.push_back(fixture);
    attachToBody();
}

void CCollider2D::createSegment(const b2Vec2& point1, const b2Vec2& point2)
{
    // Clear existing fixtures
    destroyShape();
    m_fixtures.clear();
    
    ShapeFixture fixture;
    fixture.shapeType = ColliderShape::Segment;
    fixture.shapeData.segment.point1 = point1;
    fixture.shapeData.segment.point2 = point2;
    fixture.shapeId = b2_nullShapeId;
    
    m_fixtures.push_back(fixture);
    attachToBody();
}

void CCollider2D::addSegment(const b2Vec2& point1, const b2Vec2& point2)
{
    // Add additional segment fixture
    ShapeFixture fixture;
    fixture.shapeType = ColliderShape::Segment;
    fixture.shapeData.segment.point1 = point1;
    fixture.shapeData.segment.point2 = point2;
    fixture.shapeId = b2_nullShapeId;
    
    m_fixtures.push_back(fixture);
    
    // Attach only the new fixture to body
    if (!getOwner())
    {
        return;
    }

    auto physicsBody = getOwner()->getComponent<CPhysicsBody2D>();
    if (!physicsBody || !physicsBody->isInitialized())
    {
        return;
    }

    b2BodyId bodyId = physicsBody->getBodyId();
    if (!b2Body_IsValid(bodyId))
    {
        return;
    }

    // Create shape for the new fixture
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = m_density;
    shapeDef.enableSensorEvents = m_isSensor;
    shapeDef.invokeContactCreation = true;

    b2Segment segment = {fixture.shapeData.segment.point1, fixture.shapeData.segment.point2};
    b2ShapeId newShapeId = b2CreateSegmentShape(bodyId, &shapeDef, &segment);

    // Update the fixture's shape ID
    m_fixtures.back().shapeId = newShapeId;
    m_initialized = true;

    // Set friction and restitution
    if (b2Shape_IsValid(newShapeId))
    {
        b2Shape_SetFriction(newShapeId, m_friction);
        b2Shape_SetRestitution(newShapeId, m_restitution);
    }
}

void CCollider2D::createChainSegment(const b2Vec2& ghost1, const b2Vec2& point1, const b2Vec2& point2, const b2Vec2& ghost2)
{
    // Clear existing fixtures
    destroyShape();
    m_fixtures.clear();
    
    ShapeFixture fixture;
    fixture.shapeType = ColliderShape::ChainSegment;
    fixture.shapeData.chainSegment.ghost1 = ghost1;
    fixture.shapeData.chainSegment.point1 = point1;
    fixture.shapeData.chainSegment.point2 = point2;
    fixture.shapeData.chainSegment.ghost2 = ghost2;
    fixture.shapeId = b2_nullShapeId;
    
    m_fixtures.push_back(fixture);
    attachToBody();
}

void CCollider2D::addChainSegment(const b2Vec2& ghost1, const b2Vec2& point1, const b2Vec2& point2, const b2Vec2& ghost2)
{
    // Add additional chain segment fixture
    ShapeFixture fixture;
    fixture.shapeType = ColliderShape::ChainSegment;
    fixture.shapeData.chainSegment.ghost1 = ghost1;
    fixture.shapeData.chainSegment.point1 = point1;
    fixture.shapeData.chainSegment.point2 = point2;
    fixture.shapeData.chainSegment.ghost2 = ghost2;
    fixture.shapeId = b2_nullShapeId;
    
    m_fixtures.push_back(fixture);
    
    // Attach only the new fixture to body
    if (!getOwner())
    {
        return;
    }

    auto physicsBody = getOwner()->getComponent<CPhysicsBody2D>();
    if (!physicsBody || !physicsBody->isInitialized())
    {
        return;
    }

    b2BodyId bodyId = physicsBody->getBodyId();
    if (!b2Body_IsValid(bodyId))
    {
        return;
    }

    // Create shape for the new fixture
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = m_density;
    shapeDef.enableSensorEvents = m_isSensor;
    shapeDef.invokeContactCreation = true;

    b2ChainSegment chainSeg;
    chainSeg.ghost1 = fixture.shapeData.chainSegment.ghost1;
    chainSeg.segment.point1 = fixture.shapeData.chainSegment.point1;
    chainSeg.segment.point2 = fixture.shapeData.chainSegment.point2;
    chainSeg.ghost2 = fixture.shapeData.chainSegment.ghost2;
    
    b2ShapeId newShapeId = b2CreateSegmentShape(bodyId, &shapeDef, &chainSeg.segment);

    // Update the fixture's shape ID
    m_fixtures.back().shapeId = newShapeId;
    m_initialized = true;

    // Set friction and restitution
    if (b2Shape_IsValid(newShapeId))
    {
        b2Shape_SetFriction(newShapeId, m_friction);
        b2Shape_SetRestitution(newShapeId, m_restitution);
    }
}

void CCollider2D::attachToBody()
{
    if (!getOwner())
    {
        return;
    }

    // Get the physics body component
    auto physicsBody = getOwner()->getComponent<CPhysicsBody2D>();
    if (!physicsBody || !physicsBody->isInitialized())
    {
        return;
    }

    b2BodyId bodyId = physicsBody->getBodyId();
    if (!b2Body_IsValid(bodyId))
    {
        return;
    }

    // Create shape definition
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density    = m_density;
    shapeDef.enableSensorEvents = m_isSensor;
    shapeDef.invokeContactCreation = true;

    // Create shapes for all fixtures
    for (auto& fixture : m_fixtures)
    {
        b2ShapeId shapeId = b2_nullShapeId;
        
        switch (fixture.shapeType)
        {
            case ColliderShape::Circle:
            {
                b2Circle circle;
                circle.center = fixture.shapeData.circle.center;
                circle.radius = fixture.shapeData.circle.radius;
                shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
                break;
            }

            case ColliderShape::Box:
            {
                b2Polygon box = b2MakeBox(fixture.shapeData.box.halfWidth, fixture.shapeData.box.halfHeight);
                shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &box);
                break;
            }

            case ColliderShape::Polygon:
            {
                // Reconstruct hull from stored vertices
                b2Hull hull;
                hull.count = fixture.shapeData.polygon.vertexCount;
                for (int i = 0; i < hull.count; ++i)
                {
                    hull.points[i] = fixture.shapeData.polygon.vertices[i];
                }

                // Create polygon from hull
                b2Polygon polygon = b2MakePolygon(&hull, fixture.shapeData.polygon.radius);
                shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
                break;
            }

            case ColliderShape::Segment:
            {
                b2Segment segment;
                segment.point1 = fixture.shapeData.segment.point1;
                segment.point2 = fixture.shapeData.segment.point2;
                shapeId = b2CreateSegmentShape(bodyId, &shapeDef, &segment);
                break;
            }

            case ColliderShape::ChainSegment:
            {
                b2ChainSegment chainSeg;
                chainSeg.ghost1 = fixture.shapeData.chainSegment.ghost1;
                chainSeg.segment.point1 = fixture.shapeData.chainSegment.point1;
                chainSeg.segment.point2 = fixture.shapeData.chainSegment.point2;
                chainSeg.ghost2 = fixture.shapeData.chainSegment.ghost2;
                shapeId = b2CreateSegmentShape(bodyId, &shapeDef, &chainSeg.segment);
                break;
            }
        }

        fixture.shapeId = shapeId;
        
        // Set friction and restitution after creation
        if (b2Shape_IsValid(shapeId))
        {
            b2Shape_SetFriction(shapeId, m_friction);
            b2Shape_SetRestitution(shapeId, m_restitution);
        }
    }

    m_initialized = !m_fixtures.empty();
}

void CCollider2D::destroyShape()
{
    for (auto& fixture : m_fixtures)
    {
        if (b2Shape_IsValid(fixture.shapeId))
        {
            b2DestroyShape(fixture.shapeId, true);
            fixture.shapeId = b2_nullShapeId;
        }
    }
    m_initialized = false;
}

void CCollider2D::setIsSensor(bool isSensor)
{
    m_isSensor = isSensor;
    if (m_initialized)
    {
        for (auto& fixture : m_fixtures)
        {
            if (b2Shape_IsValid(fixture.shapeId))
            {
                b2Shape_EnableSensorEvents(fixture.shapeId, isSensor);
            }
        }
    }
}

void CCollider2D::setDensity(float density)
{
    m_density = density;
    if (m_initialized)
    {
        for (auto& fixture : m_fixtures)
        {
            if (b2Shape_IsValid(fixture.shapeId))
            {
                b2Shape_SetDensity(fixture.shapeId, density, true);
            }
        }
    }
}

void CCollider2D::setFriction(float friction)
{
    m_friction = friction;
    if (m_initialized)
    {
        for (auto& fixture : m_fixtures)
        {
            if (b2Shape_IsValid(fixture.shapeId))
            {
                b2Shape_SetFriction(fixture.shapeId, friction);
            }
        }
    }
}

void CCollider2D::setRestitution(float restitution)
{
    m_restitution = restitution;
    if (m_initialized)
    {
        for (auto& fixture : m_fixtures)
        {
            if (b2Shape_IsValid(fixture.shapeId))
            {
                b2Shape_SetRestitution(fixture.shapeId, restitution);
            }
        }
    }
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

void CCollider2D::serialize(JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cCollider2D");
    builder.beginObject();

    // Serialize fixture properties
    builder.addKey("isSensor");
    builder.addBool(m_isSensor);
    builder.addKey("density");
    builder.addNumber(m_density);
    builder.addKey("friction");
    builder.addNumber(m_friction);
    builder.addKey("restitution");
    builder.addNumber(m_restitution);

    // Serialize all fixtures
    builder.addKey("fixtures");
    builder.beginArray();
    for (const auto& fixture : m_fixtures)
    {
        builder.beginObject();
        
        // Shape type
        builder.addKey("shapeType");
        switch (fixture.shapeType)
        {
            case ColliderShape::Circle:
                builder.addString("Circle");
                break;
            case ColliderShape::Box:
                builder.addString("Box");
                break;
            case ColliderShape::Polygon:
                builder.addString("Polygon");
                break;
            case ColliderShape::Segment:
                builder.addString("Segment");
                break;
            case ColliderShape::ChainSegment:
                builder.addString("ChainSegment");
                break;
        }

        // Shape data
        if (fixture.shapeType == ColliderShape::Circle)
        {
            builder.addKey("radius");
            builder.addNumber(fixture.shapeData.circle.radius);
            builder.addKey("centerX");
            builder.addNumber(fixture.shapeData.circle.center.x);
            builder.addKey("centerY");
            builder.addNumber(fixture.shapeData.circle.center.y);
        }
        else if (fixture.shapeType == ColliderShape::Box)
        {
            builder.addKey("halfWidth");
            builder.addNumber(fixture.shapeData.box.halfWidth);
            builder.addKey("halfHeight");
            builder.addNumber(fixture.shapeData.box.halfHeight);
        }
        else if (fixture.shapeType == ColliderShape::Polygon)
        {
            builder.addKey("vertexCount");
            builder.addNumber(fixture.shapeData.polygon.vertexCount);
            builder.addKey("radius");
            builder.addNumber(fixture.shapeData.polygon.radius);
            builder.addKey("vertices");
            builder.beginArray();
            for (int i = 0; i < fixture.shapeData.polygon.vertexCount; ++i)
            {
                builder.beginObject();
                builder.addKey("x");
                builder.addNumber(fixture.shapeData.polygon.vertices[i].x);
                builder.addKey("y");
                builder.addNumber(fixture.shapeData.polygon.vertices[i].y);
                builder.endObject();
            }
            builder.endArray();
        }
        else if (fixture.shapeType == ColliderShape::Segment)
        {
            builder.addKey("point1");
            builder.beginObject();
            builder.addKey("x");
            builder.addNumber(fixture.shapeData.segment.point1.x);
            builder.addKey("y");
            builder.addNumber(fixture.shapeData.segment.point1.y);
            builder.endObject();
            
            builder.addKey("point2");
            builder.beginObject();
            builder.addKey("x");
            builder.addNumber(fixture.shapeData.segment.point2.x);
            builder.addKey("y");
            builder.addNumber(fixture.shapeData.segment.point2.y);
            builder.endObject();
        }
        else if (fixture.shapeType == ColliderShape::ChainSegment)
        {
            builder.addKey("ghost1");
            builder.beginObject();
            builder.addKey("x");
            builder.addNumber(fixture.shapeData.chainSegment.ghost1.x);
            builder.addKey("y");
            builder.addNumber(fixture.shapeData.chainSegment.ghost1.y);
            builder.endObject();
            
            builder.addKey("point1");
            builder.beginObject();
            builder.addKey("x");
            builder.addNumber(fixture.shapeData.chainSegment.point1.x);
            builder.addKey("y");
            builder.addNumber(fixture.shapeData.chainSegment.point1.y);
            builder.endObject();
            
            builder.addKey("point2");
            builder.beginObject();
            builder.addKey("x");
            builder.addNumber(fixture.shapeData.chainSegment.point2.x);
            builder.addKey("y");
            builder.addNumber(fixture.shapeData.chainSegment.point2.y);
            builder.endObject();
            
            builder.addKey("ghost2");
            builder.beginObject();
            builder.addKey("x");
            builder.addNumber(fixture.shapeData.chainSegment.ghost2.x);
            builder.addKey("y");
            builder.addNumber(fixture.shapeData.chainSegment.ghost2.y);
            builder.endObject();
        }
        
        builder.endObject();
    }
    builder.endArray();

    builder.endObject();
    builder.endObject();
}

void CCollider2D::init()
{
    // If the collider hasn't been initialized yet, attach it to the body now
    // This is called after deserialization to create the Box2D shape
    if (!m_initialized)
    {
        attachToBody();
    }
}

void CCollider2D::deserialize(const JsonValue& value)
{
    if (!value.isObject())
        return;

    const auto& collider = value["cCollider2D"];
    if (!collider.isObject())
        return;

    // Fixture properties
    const auto& isSensorValue = collider["isSensor"];
    if (isSensorValue.isBool())
    {
        m_isSensor = isSensorValue.getBool();
    }

    const auto& densityValue = collider["density"];
    if (densityValue.isNumber())
    {
        m_density = static_cast<float>(densityValue.getNumber());
    }

    const auto& frictionValue = collider["friction"];
    if (frictionValue.isNumber())
    {
        m_friction = static_cast<float>(frictionValue.getNumber());
    }

    const auto& restitutionValue = collider["restitution"];
    if (restitutionValue.isNumber())
    {
        m_restitution = static_cast<float>(restitutionValue.getNumber());
    }

    // Deserialize fixtures
    const auto& fixturesValue = collider["fixtures"];
    if (fixturesValue.isArray())
    {
        const auto& fixturesArray = fixturesValue.getArray();
        m_fixtures.clear();
        
        for (const auto& fixtureValue : fixturesArray)
        {
            if (!fixtureValue.isObject())
                continue;
                
            ShapeFixture fixture;
            fixture.shapeId = b2_nullShapeId;
            
            // Shape type
            const auto& shapeTypeValue = fixtureValue["shapeType"];
            if (shapeTypeValue.isString())
            {
                std::string typeStr = shapeTypeValue.getString();
                if (typeStr == "Circle")
                {
                    fixture.shapeType = ColliderShape::Circle;

                    const auto& radiusValue = fixtureValue["radius"];
                    if (radiusValue.isNumber())
                    {
                        fixture.shapeData.circle.radius = static_cast<float>(radiusValue.getNumber());
                    }

                    const auto& centerXValue = fixtureValue["centerX"];
                    if (centerXValue.isNumber())
                    {
                        fixture.shapeData.circle.center.x = static_cast<float>(centerXValue.getNumber());
                    }

                    const auto& centerYValue = fixtureValue["centerY"];
                    if (centerYValue.isNumber())
                    {
                        fixture.shapeData.circle.center.y = static_cast<float>(centerYValue.getNumber());
                    }
                }
                else if (typeStr == "Box")
                {
                    fixture.shapeType = ColliderShape::Box;

                    const auto& halfWidthValue = fixtureValue["halfWidth"];
                    if (halfWidthValue.isNumber())
                    {
                        fixture.shapeData.box.halfWidth = static_cast<float>(halfWidthValue.getNumber());
                    }

                    const auto& halfHeightValue = fixtureValue["halfHeight"];
                    if (halfHeightValue.isNumber())
                    {
                        fixture.shapeData.box.halfHeight = static_cast<float>(halfHeightValue.getNumber());
                    }
                }
                else if (typeStr == "Polygon")
                {
                    fixture.shapeType = ColliderShape::Polygon;

                    const auto& vertexCountValue = fixtureValue["vertexCount"];
                    if (vertexCountValue.isNumber())
                    {
                        fixture.shapeData.polygon.vertexCount = static_cast<int>(vertexCountValue.getNumber());
                    }

                    const auto& radiusValue = fixtureValue["radius"];
                    if (radiusValue.isNumber())
                    {
                        fixture.shapeData.polygon.radius = static_cast<float>(radiusValue.getNumber());
                    }

                    const auto& verticesValue = fixtureValue["vertices"];
                    if (verticesValue.isArray())
                    {
                        const auto& vertices = verticesValue.getArray();
                        int count = std::min(static_cast<int>(vertices.size()),
                                             static_cast<int>(B2_MAX_POLYGON_VERTICES));
                        for (int i = 0; i < count; ++i)
                        {
                            if (vertices[i].isObject())
                            {
                                const auto& xValue = vertices[i]["x"];
                                const auto& yValue = vertices[i]["y"];
                                if (xValue.isNumber() && yValue.isNumber())
                                {
                                    fixture.shapeData.polygon.vertices[i].x =
                                        static_cast<float>(xValue.getNumber());
                                    fixture.shapeData.polygon.vertices[i].y =
                                        static_cast<float>(yValue.getNumber());
                                }
                            }
                        }
                    }
                }
                else if (typeStr == "Segment")
                {
                    fixture.shapeType = ColliderShape::Segment;

                    const auto& point1Value = fixtureValue["point1"];
                    if (point1Value.isObject())
                    {
                        const auto& x = point1Value["x"];
                        const auto& y = point1Value["y"];
                        if (x.isNumber() && y.isNumber())
                        {
                            fixture.shapeData.segment.point1.x = static_cast<float>(x.getNumber());
                            fixture.shapeData.segment.point1.y = static_cast<float>(y.getNumber());
                        }
                    }

                    const auto& point2Value = fixtureValue["point2"];
                    if (point2Value.isObject())
                    {
                        const auto& x = point2Value["x"];
                        const auto& y = point2Value["y"];
                        if (x.isNumber() && y.isNumber())
                        {
                            fixture.shapeData.segment.point2.x = static_cast<float>(x.getNumber());
                            fixture.shapeData.segment.point2.y = static_cast<float>(y.getNumber());
                        }
                    }
                }
                else if (typeStr == "ChainSegment")
                {
                    fixture.shapeType = ColliderShape::ChainSegment;

                    const auto& ghost1Value = fixtureValue["ghost1"];
                    if (ghost1Value.isObject())
                    {
                        const auto& x = ghost1Value["x"];
                        const auto& y = ghost1Value["y"];
                        if (x.isNumber() && y.isNumber())
                        {
                            fixture.shapeData.chainSegment.ghost1.x = static_cast<float>(x.getNumber());
                            fixture.shapeData.chainSegment.ghost1.y = static_cast<float>(y.getNumber());
                        }
                    }

                    const auto& point1Value = fixtureValue["point1"];
                    if (point1Value.isObject())
                    {
                        const auto& x = point1Value["x"];
                        const auto& y = point1Value["y"];
                        if (x.isNumber() && y.isNumber())
                        {
                            fixture.shapeData.chainSegment.point1.x = static_cast<float>(x.getNumber());
                            fixture.shapeData.chainSegment.point1.y = static_cast<float>(y.getNumber());
                        }
                    }

                    const auto& point2Value = fixtureValue["point2"];
                    if (point2Value.isObject())
                    {
                        const auto& x = point2Value["x"];
                        const auto& y = point2Value["y"];
                        if (x.isNumber() && y.isNumber())
                        {
                            fixture.shapeData.chainSegment.point2.x = static_cast<float>(x.getNumber());
                            fixture.shapeData.chainSegment.point2.y = static_cast<float>(y.getNumber());
                        }
                    }

                    const auto& ghost2Value = fixtureValue["ghost2"];
                    if (ghost2Value.isObject())
                    {
                        const auto& x = ghost2Value["x"];
                        const auto& y = ghost2Value["y"];
                        if (x.isNumber() && y.isNumber())
                        {
                            fixture.shapeData.chainSegment.ghost2.x = static_cast<float>(x.getNumber());
                            fixture.shapeData.chainSegment.ghost2.y = static_cast<float>(y.getNumber());
                        }
                    }
                }
                
                m_fixtures.push_back(fixture);
            }
        }
    }
    // Support legacy format (single shape)
    else
    {
        const auto& shapeTypeValue = collider["shapeType"];
        if (shapeTypeValue.isString())
        {
            ShapeFixture fixture;
            fixture.shapeId = b2_nullShapeId;
            std::string typeStr = shapeTypeValue.getString();
            
            if (typeStr == "Circle")
            {
                fixture.shapeType = ColliderShape::Circle;
                const auto& radiusValue = collider["radius"];
                if (radiusValue.isNumber())
                {
                    fixture.shapeData.circle.radius = static_cast<float>(radiusValue.getNumber());
                }
                const auto& centerXValue = collider["centerX"];
                if (centerXValue.isNumber())
                {
                    fixture.shapeData.circle.center.x = static_cast<float>(centerXValue.getNumber());
                }
                const auto& centerYValue = collider["centerY"];
                if (centerYValue.isNumber())
                {
                    fixture.shapeData.circle.center.y = static_cast<float>(centerYValue.getNumber());
                }
            }
            else if (typeStr == "Box")
            {
                fixture.shapeType = ColliderShape::Box;
                const auto& halfWidthValue = collider["halfWidth"];
                if (halfWidthValue.isNumber())
                {
                    fixture.shapeData.box.halfWidth = static_cast<float>(halfWidthValue.getNumber());
                }
                const auto& halfHeightValue = collider["halfHeight"];
                if (halfHeightValue.isNumber())
                {
                    fixture.shapeData.box.halfHeight = static_cast<float>(halfHeightValue.getNumber());
                }
            }
            else if (typeStr == "Polygon")
            {
                fixture.shapeType = ColliderShape::Polygon;
                const auto& vertexCountValue = collider["vertexCount"];
                if (vertexCountValue.isNumber())
                {
                    fixture.shapeData.polygon.vertexCount = static_cast<int>(vertexCountValue.getNumber());
                }
                const auto& radiusValue = collider["radius"];
                if (radiusValue.isNumber())
                {
                    fixture.shapeData.polygon.radius = static_cast<float>(radiusValue.getNumber());
                }
                const auto& verticesValue = collider["vertices"];
                if (verticesValue.isArray())
                {
                    const auto& vertices = verticesValue.getArray();
                    int count = std::min(static_cast<int>(vertices.size()),
                                         static_cast<int>(B2_MAX_POLYGON_VERTICES));
                    for (int i = 0; i < count; ++i)
                    {
                        if (vertices[i].isObject())
                        {
                            const auto& xValue = vertices[i]["x"];
                            const auto& yValue = vertices[i]["y"];
                            if (xValue.isNumber() && yValue.isNumber())
                            {
                                fixture.shapeData.polygon.vertices[i].x =
                                    static_cast<float>(xValue.getNumber());
                                fixture.shapeData.polygon.vertices[i].y =
                                    static_cast<float>(yValue.getNumber());
                            }
                        }
                    }
                }
            }
            
            m_fixtures.push_back(fixture);
        }
    }
}

std::string CCollider2D::getType() const
{
    return "CCollider2D";
}
