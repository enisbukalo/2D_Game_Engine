#include <gtest/gtest.h>
#include "CParticleEmitter.h"
#include "Color.h"
#include "Vec2.h"
#include "SSerialization.h"
#include <vector>

/**
 * @brief Test fixture for CParticleEmitter tests
 */
class CParticleEmitterTest : public ::testing::Test
{
protected:
    CParticleEmitter* m_emitter;

    void SetUp() override
    {
        m_emitter = new CParticleEmitter();
    }

    void TearDown() override
    {
        delete m_emitter;
        m_emitter = nullptr;
    }
};

//=============================================================================
// Default Value Tests
//=============================================================================

TEST_F(CParticleEmitterTest, DefaultValuesAreSet)
{
    EXPECT_EQ(m_emitter->getEmissionShape(), EmissionShape::Point);
    EXPECT_TRUE(m_emitter->isActive());
    EXPECT_EQ(m_emitter->getZIndex(), 0);
    EXPECT_EQ(m_emitter->getMaxParticles(), 200);  // Default is 200
}

TEST_F(CParticleEmitterTest, DefaultDirectionIsUp)
{
    Vec2 direction = m_emitter->getDirection();
    EXPECT_FLOAT_EQ(direction.x, 0.0f);
    EXPECT_FLOAT_EQ(direction.y, 1.0f);
}

//=============================================================================
// Z-Index Tests
//=============================================================================

TEST_F(CParticleEmitterTest, ZIndexCanBeSet)
{
    m_emitter->setZIndex(5);
    EXPECT_EQ(m_emitter->getZIndex(), 5);
}

TEST_F(CParticleEmitterTest, ZIndexCanBeNegative)
{
    m_emitter->setZIndex(-10);
    EXPECT_EQ(m_emitter->getZIndex(), -10);
}

TEST_F(CParticleEmitterTest, ZIndexCanBeLargePositive)
{
    m_emitter->setZIndex(1000);
    EXPECT_EQ(m_emitter->getZIndex(), 1000);
}

//=============================================================================
// Emission Shape Tests
//=============================================================================

TEST_F(CParticleEmitterTest, EmissionShapeCanBeSetToCircle)
{
    m_emitter->setEmissionShape(EmissionShape::Circle);
    EXPECT_EQ(m_emitter->getEmissionShape(), EmissionShape::Circle);
}

TEST_F(CParticleEmitterTest, EmissionShapeCanBeSetToRectangle)
{
    m_emitter->setEmissionShape(EmissionShape::Rectangle);
    EXPECT_EQ(m_emitter->getEmissionShape(), EmissionShape::Rectangle);
}

TEST_F(CParticleEmitterTest, EmissionShapeCanBeSetToLine)
{
    m_emitter->setEmissionShape(EmissionShape::Line);
    EXPECT_EQ(m_emitter->getEmissionShape(), EmissionShape::Line);
}

TEST_F(CParticleEmitterTest, EmissionShapeCanBeSetToPolygon)
{
    m_emitter->setEmissionShape(EmissionShape::Polygon);
    EXPECT_EQ(m_emitter->getEmissionShape(), EmissionShape::Polygon);
}

TEST_F(CParticleEmitterTest, ShapeRadiusCanBeSet)
{
    m_emitter->setShapeRadius(2.5f);
    EXPECT_FLOAT_EQ(m_emitter->getShapeRadius(), 2.5f);
}

TEST_F(CParticleEmitterTest, ShapeSizeCanBeSet)
{
    m_emitter->setShapeSize(Vec2(3.0f, 4.0f));
    Vec2 size = m_emitter->getShapeSize();
    EXPECT_FLOAT_EQ(size.x, 3.0f);
    EXPECT_FLOAT_EQ(size.y, 4.0f);
}

TEST_F(CParticleEmitterTest, LineEndpointsCanBeSet)
{
    m_emitter->setLineStart(Vec2(-1.0f, 0.0f));
    m_emitter->setLineEnd(Vec2(1.0f, 0.0f));

    Vec2 start = m_emitter->getLineStart();
    Vec2 end   = m_emitter->getLineEnd();

    EXPECT_FLOAT_EQ(start.x, -1.0f);
    EXPECT_FLOAT_EQ(start.y, 0.0f);
    EXPECT_FLOAT_EQ(end.x, 1.0f);
    EXPECT_FLOAT_EQ(end.y, 0.0f);
}

TEST_F(CParticleEmitterTest, PolygonVerticesCanBeSet)
{
    std::vector<Vec2> vertices = {Vec2(0, 0), Vec2(1, 0), Vec2(1, 1), Vec2(0, 1)};

    m_emitter->setPolygonVertices(vertices);

    const auto& storedVertices = m_emitter->getPolygonVertices();
    ASSERT_EQ(storedVertices.size(), 4);
    EXPECT_FLOAT_EQ(storedVertices[0].x, 0.0f);
    EXPECT_FLOAT_EQ(storedVertices[2].x, 1.0f);
}

TEST_F(CParticleEmitterTest, SetPolygonFromConvexHullComputesHull)
{
    // Create a set of points that includes interior points
    std::vector<Vec2> points = {
        Vec2(0, 0),    // corner
        Vec2(2, 0),    // corner
        Vec2(2, 2),    // corner
        Vec2(0, 2),    // corner
        Vec2(1, 1),    // interior point - should be excluded
        Vec2(0.5, 0.5) // interior point - should be excluded
    };

    m_emitter->setPolygonFromConvexHull(points);

    const auto& hull = m_emitter->getPolygonVertices();
    // Convex hull should have 4 vertices (the corners)
    EXPECT_EQ(hull.size(), 4);
}

//=============================================================================
// Emit Outward Tests
//=============================================================================

TEST_F(CParticleEmitterTest, EmitOutwardDefaultsToFalse)
{
    EXPECT_FALSE(m_emitter->getEmitOutward());
}

TEST_F(CParticleEmitterTest, EmitOutwardCanBeEnabled)
{
    m_emitter->setEmitOutward(true);
    EXPECT_TRUE(m_emitter->getEmitOutward());
}

TEST_F(CParticleEmitterTest, EmitOutwardCanBeDisabled)
{
    m_emitter->setEmitOutward(true);
    m_emitter->setEmitOutward(false);
    EXPECT_FALSE(m_emitter->getEmitOutward());
}

//=============================================================================
// Particle Property Tests
//=============================================================================

TEST_F(CParticleEmitterTest, LifetimeRangeCanBeSet)
{
    m_emitter->setMinLifetime(1.0f);
    m_emitter->setMaxLifetime(5.0f);
    EXPECT_FLOAT_EQ(m_emitter->getMinLifetime(), 1.0f);
    EXPECT_FLOAT_EQ(m_emitter->getMaxLifetime(), 5.0f);
}

TEST_F(CParticleEmitterTest, SizeRangeCanBeSet)
{
    m_emitter->setMinSize(0.1f);
    m_emitter->setMaxSize(0.5f);
    EXPECT_FLOAT_EQ(m_emitter->getMinSize(), 0.1f);
    EXPECT_FLOAT_EQ(m_emitter->getMaxSize(), 0.5f);
}

TEST_F(CParticleEmitterTest, SpeedRangeCanBeSet)
{
    m_emitter->setMinSpeed(2.0f);
    m_emitter->setMaxSpeed(10.0f);
    EXPECT_FLOAT_EQ(m_emitter->getMinSpeed(), 2.0f);
    EXPECT_FLOAT_EQ(m_emitter->getMaxSpeed(), 10.0f);
}

TEST_F(CParticleEmitterTest, EmissionRateCanBeSet)
{
    m_emitter->setEmissionRate(100.0f);
    EXPECT_FLOAT_EQ(m_emitter->getEmissionRate(), 100.0f);
}

TEST_F(CParticleEmitterTest, ColorsCanBeSet)
{
    Color startColor(255, 0, 0);
    Color endColor(0, 0, 255);

    m_emitter->setStartColor(startColor);
    m_emitter->setEndColor(endColor);

    EXPECT_EQ(m_emitter->getStartColor().r, 255);
    EXPECT_EQ(m_emitter->getStartColor().g, 0);
    EXPECT_EQ(m_emitter->getEndColor().b, 255);
}

TEST_F(CParticleEmitterTest, AlphaValuesCanBeSet)
{
    m_emitter->setStartAlpha(1.0f);
    m_emitter->setEndAlpha(0.0f);
    EXPECT_FLOAT_EQ(m_emitter->getStartAlpha(), 1.0f);
    EXPECT_FLOAT_EQ(m_emitter->getEndAlpha(), 0.0f);
}

TEST_F(CParticleEmitterTest, GravityCanBeSet)
{
    m_emitter->setGravity(Vec2(0.0f, -9.8f));
    Vec2 gravity = m_emitter->getGravity();
    EXPECT_FLOAT_EQ(gravity.x, 0.0f);
    EXPECT_FLOAT_EQ(gravity.y, -9.8f);
}

TEST_F(CParticleEmitterTest, SpreadAngleCanBeSet)
{
    m_emitter->setSpreadAngle(0.5f);
    EXPECT_FLOAT_EQ(m_emitter->getSpreadAngle(), 0.5f);
}

TEST_F(CParticleEmitterTest, DirectionCanBeSet)
{
    m_emitter->setDirection(Vec2(1.0f, 0.0f));
    Vec2 direction = m_emitter->getDirection();
    EXPECT_FLOAT_EQ(direction.x, 1.0f);
    EXPECT_FLOAT_EQ(direction.y, 0.0f);
}

TEST_F(CParticleEmitterTest, PositionOffsetCanBeSet)
{
    m_emitter->setPositionOffset(Vec2(1.0f, 2.0f));
    Vec2 offset = m_emitter->getPositionOffset();
    EXPECT_FLOAT_EQ(offset.x, 1.0f);
    EXPECT_FLOAT_EQ(offset.y, 2.0f);
}

//=============================================================================
// Effect Toggle Tests
//=============================================================================

TEST_F(CParticleEmitterTest, FadeOutCanBeToggled)
{
    m_emitter->setFadeOut(true);
    EXPECT_TRUE(m_emitter->getFadeOut());
    m_emitter->setFadeOut(false);
    EXPECT_FALSE(m_emitter->getFadeOut());
}

TEST_F(CParticleEmitterTest, ShrinkCanBeToggled)
{
    m_emitter->setShrink(true);
    EXPECT_TRUE(m_emitter->getShrink());
    m_emitter->setShrink(false);
    EXPECT_FALSE(m_emitter->getShrink());
}

TEST_F(CParticleEmitterTest, ShrinkEndScaleCanBeSet)
{
    m_emitter->setShrinkEndScale(0.1f);
    EXPECT_FLOAT_EQ(m_emitter->getShrinkEndScale(), 0.1f);
}

//=============================================================================
// Particle Management Tests
//=============================================================================

TEST_F(CParticleEmitterTest, MaxParticlesCanBeSet)
{
    m_emitter->setMaxParticles(500);
    EXPECT_EQ(m_emitter->getMaxParticles(), 500);
}

TEST_F(CParticleEmitterTest, AliveCountStartsAtZero)
{
    EXPECT_EQ(m_emitter->getAliveCount(), 0);
}

TEST_F(CParticleEmitterTest, ActiveStateCanBeToggled)
{
    EXPECT_TRUE(m_emitter->isActive());
    m_emitter->setActive(false);
    EXPECT_FALSE(m_emitter->isActive());
    m_emitter->setActive(true);
    EXPECT_TRUE(m_emitter->isActive());
}

//=============================================================================
// Rotation Tests
//=============================================================================

TEST_F(CParticleEmitterTest, RotationSpeedRangeCanBeSet)
{
    m_emitter->setMinRotationSpeed(-1.0f);
    m_emitter->setMaxRotationSpeed(1.0f);
    EXPECT_FLOAT_EQ(m_emitter->getMinRotationSpeed(), -1.0f);
    EXPECT_FLOAT_EQ(m_emitter->getMaxRotationSpeed(), 1.0f);
}

//=============================================================================
// Serialization Tests
//=============================================================================

TEST_F(CParticleEmitterTest, SerializationContainsAllFields)
{
    // Set up all fields to non-default values
    m_emitter->setDirection(Vec2(1.0f, 0.0f));
    m_emitter->setSpreadAngle(0.75f);
    m_emitter->setMinSpeed(0.5f);
    m_emitter->setMaxSpeed(1.5f);
    m_emitter->setMinLifetime(2.0f);
    m_emitter->setMaxLifetime(3.0f);
    m_emitter->setMinSize(0.25f);
    m_emitter->setMaxSize(0.75f);
    m_emitter->setEmissionRate(50.0f);
    m_emitter->setBurstCount(10.0f);
    m_emitter->setStartColor(Color(255, 0, 0, 255));
    m_emitter->setEndColor(Color(0, 0, 255, 255));
    m_emitter->setStartAlpha(0.9f);
    m_emitter->setEndAlpha(0.1f);
    m_emitter->setGravity(Vec2(0.0f, -9.8f));
    m_emitter->setMinRotationSpeed(-2.0f);
    m_emitter->setMaxRotationSpeed(2.0f);
    m_emitter->setFadeOut(false);
    m_emitter->setShrink(false);
    m_emitter->setShrinkEndScale(0.5f);
    m_emitter->setMaxParticles(500);
    m_emitter->setZIndex(5);
    m_emitter->setPositionOffset(Vec2(1.0f, 2.0f));
    m_emitter->setEmissionShape(EmissionShape::Circle);
    m_emitter->setShapeRadius(2.0f);
    m_emitter->setShapeSize(Vec2(3.0f, 4.0f));
    m_emitter->setLineStart(Vec2(-1.0f, 0.0f));
    m_emitter->setLineEnd(Vec2(1.0f, 0.0f));
    m_emitter->setEmitFromEdge(false);
    m_emitter->setEmitOutward(true);
    m_emitter->setActive(false);

    Serialization::JsonBuilder builder;
    m_emitter->serialize(builder);
    std::string json = builder.toString();

    // Check all major fields exist
    EXPECT_TRUE(json.find("\"cParticleEmitter\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"direction\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"spreadAngle\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"minSpeed\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"maxSpeed\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"minLifetime\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"maxLifetime\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"minSize\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"maxSize\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"emissionRate\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"burstCount\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"startColor\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"endColor\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"startAlpha\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"endAlpha\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"gravity\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"minRotationSpeed\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"maxRotationSpeed\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"fadeOut\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"shrink\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"shrinkEndScale\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"maxParticles\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"zIndex\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"offset\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"emissionShape\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"shapeRadius\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"shapeSize\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"lineStart\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"lineEnd\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"emitFromEdge\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"emitOutward\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"active\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"polygonVertices\"") != std::string::npos);
}

TEST_F(CParticleEmitterTest, DeserializationAllFields)
{
    std::string json = R"({
        "cParticleEmitter": {
            "active": false,
            "direction": { "x": 1.0, "y": 0.0 },
            "spreadAngle": 0.75,
            "minSpeed": 0.5,
            "maxSpeed": 1.5,
            "minLifetime": 2.0,
            "maxLifetime": 3.0,
            "minSize": 0.25,
            "maxSize": 0.75,
            "emissionRate": 50.0,
            "burstCount": 10.0,
            "startColor": { "r": 255, "g": 0, "b": 0, "a": 255 },
            "endColor": { "r": 0, "g": 0, "b": 255, "a": 255 },
            "startAlpha": 0.9,
            "endAlpha": 0.1,
            "gravity": { "x": 0.0, "y": -9.8 },
            "minRotationSpeed": -2.0,
            "maxRotationSpeed": 2.0,
            "fadeOut": false,
            "shrink": false,
            "shrinkEndScale": 0.5,
            "maxParticles": 500,
            "zIndex": 5,
            "offset": { "x": 1.0, "y": 2.0 },
            "emissionShape": 1,
            "shapeRadius": 2.0,
            "shapeSize": { "x": 3.0, "y": 4.0 },
            "lineStart": { "x": -1.0, "y": 0.0 },
            "lineEnd": { "x": 1.0, "y": 0.0 },
            "emitFromEdge": false,
            "emitOutward": true,
            "polygonVertices": [
                { "x": 0.0, "y": 0.0 },
                { "x": 1.0, "y": 0.0 },
                { "x": 1.0, "y": 1.0 }
            ]
        }
    })";    Serialization::SSerialization::JsonValue value(json);
    m_emitter->deserialize(value);

    EXPECT_FALSE(m_emitter->isActive());

    Vec2 dir = m_emitter->getDirection();
    EXPECT_FLOAT_EQ(dir.x, 1.0f);
    EXPECT_FLOAT_EQ(dir.y, 0.0f);

    EXPECT_FLOAT_EQ(m_emitter->getSpreadAngle(), 0.75f);
    EXPECT_FLOAT_EQ(m_emitter->getMinSpeed(), 0.5f);
    EXPECT_FLOAT_EQ(m_emitter->getMaxSpeed(), 1.5f);
    EXPECT_FLOAT_EQ(m_emitter->getMinLifetime(), 2.0f);
    EXPECT_FLOAT_EQ(m_emitter->getMaxLifetime(), 3.0f);
    EXPECT_FLOAT_EQ(m_emitter->getMinSize(), 0.25f);
    EXPECT_FLOAT_EQ(m_emitter->getMaxSize(), 0.75f);
    EXPECT_FLOAT_EQ(m_emitter->getEmissionRate(), 50.0f);
    EXPECT_FLOAT_EQ(m_emitter->getBurstCount(), 10.0f);

    Color startColor = m_emitter->getStartColor();
    EXPECT_EQ(startColor.r, 255);
    EXPECT_EQ(startColor.g, 0);
    EXPECT_EQ(startColor.b, 0);
    EXPECT_EQ(startColor.a, 255);

    Color endColor = m_emitter->getEndColor();
    EXPECT_EQ(endColor.r, 0);
    EXPECT_EQ(endColor.g, 0);
    EXPECT_EQ(endColor.b, 255);
    EXPECT_EQ(endColor.a, 255);

    EXPECT_FLOAT_EQ(m_emitter->getStartAlpha(), 0.9f);
    EXPECT_FLOAT_EQ(m_emitter->getEndAlpha(), 0.1f);

    Vec2 gravity = m_emitter->getGravity();
    EXPECT_FLOAT_EQ(gravity.x, 0.0f);
    EXPECT_FLOAT_EQ(gravity.y, -9.8f);

    EXPECT_FLOAT_EQ(m_emitter->getMinRotationSpeed(), -2.0f);
    EXPECT_FLOAT_EQ(m_emitter->getMaxRotationSpeed(), 2.0f);

    EXPECT_FALSE(m_emitter->getFadeOut());
    EXPECT_FALSE(m_emitter->getShrink());
    EXPECT_FLOAT_EQ(m_emitter->getShrinkEndScale(), 0.5f);

    EXPECT_EQ(m_emitter->getMaxParticles(), 500);
    EXPECT_EQ(m_emitter->getZIndex(), 5);

    Vec2 offset = m_emitter->getPositionOffset();
    EXPECT_FLOAT_EQ(offset.x, 1.0f);
    EXPECT_FLOAT_EQ(offset.y, 2.0f);

    EXPECT_EQ(m_emitter->getEmissionShape(), EmissionShape::Circle);
    EXPECT_FLOAT_EQ(m_emitter->getShapeRadius(), 2.0f);

    Vec2 shapeSize = m_emitter->getShapeSize();
    EXPECT_FLOAT_EQ(shapeSize.x, 3.0f);
    EXPECT_FLOAT_EQ(shapeSize.y, 4.0f);

    Vec2 lineStart = m_emitter->getLineStart();
    EXPECT_FLOAT_EQ(lineStart.x, -1.0f);
    EXPECT_FLOAT_EQ(lineStart.y, 0.0f);

    Vec2 lineEnd = m_emitter->getLineEnd();
    EXPECT_FLOAT_EQ(lineEnd.x, 1.0f);
    EXPECT_FLOAT_EQ(lineEnd.y, 0.0f);

    EXPECT_FALSE(m_emitter->getEmitFromEdge());
    EXPECT_TRUE(m_emitter->getEmitOutward());

    const auto& vertices = m_emitter->getPolygonVertices();
    ASSERT_EQ(vertices.size(), 3);
    EXPECT_FLOAT_EQ(vertices[0].x, 0.0f);
    EXPECT_FLOAT_EQ(vertices[0].y, 0.0f);
    EXPECT_FLOAT_EQ(vertices[1].x, 1.0f);
    EXPECT_FLOAT_EQ(vertices[1].y, 0.0f);
    EXPECT_FLOAT_EQ(vertices[2].x, 1.0f);
    EXPECT_FLOAT_EQ(vertices[2].y, 1.0f);
}

TEST_F(CParticleEmitterTest, SerializeDeserializeRoundTrip)
{
    // Set up first emitter with custom values
    m_emitter->setDirection(Vec2(0.5f, 0.5f));
    m_emitter->setSpreadAngle(1.0f);
    m_emitter->setMinSpeed(0.2f);
    m_emitter->setMaxSpeed(0.8f);
    m_emitter->setMinLifetime(0.5f);
    m_emitter->setMaxLifetime(2.5f);
    m_emitter->setMinSize(0.1f);
    m_emitter->setMaxSize(1.0f);
    m_emitter->setEmissionRate(25.0f);
    m_emitter->setBurstCount(5.0f);
    m_emitter->setStartColor(Color(100, 150, 200, 255));
    m_emitter->setEndColor(Color(50, 75, 100, 128));
    m_emitter->setStartAlpha(0.8f);
    m_emitter->setEndAlpha(0.2f);
    m_emitter->setGravity(Vec2(1.0f, -5.0f));
    m_emitter->setMinRotationSpeed(-0.5f);
    m_emitter->setMaxRotationSpeed(0.5f);
    m_emitter->setFadeOut(false);
    m_emitter->setShrink(true);
    m_emitter->setShrinkEndScale(0.25f);
    m_emitter->setMaxParticles(300);
    m_emitter->setZIndex(-2);
    m_emitter->setPositionOffset(Vec2(-0.5f, 0.5f));
    m_emitter->setEmissionShape(EmissionShape::Rectangle);
    m_emitter->setShapeRadius(1.5f);
    m_emitter->setShapeSize(Vec2(2.0f, 1.0f));
    m_emitter->setLineStart(Vec2(-2.0f, -1.0f));
    m_emitter->setLineEnd(Vec2(2.0f, 1.0f));
    m_emitter->setEmitFromEdge(true);
    m_emitter->setEmitOutward(false);
    m_emitter->setActive(false);

    std::vector<Vec2> vertices = {Vec2(0, 0), Vec2(1, 0), Vec2(1, 1), Vec2(0, 1)};
    m_emitter->setPolygonVertices(vertices);

    // Serialize
    Serialization::JsonBuilder builder;
    m_emitter->serialize(builder);
    std::string json = builder.toString();

    // Deserialize into a new emitter
    CParticleEmitter emitter2;
   Serialization::SSerialization::JsonValue        value(json);
    emitter2.deserialize(value);

    // Compare all values
    EXPECT_FALSE(emitter2.isActive());

    Vec2 dir1 = m_emitter->getDirection();
    Vec2 dir2 = emitter2.getDirection();
    EXPECT_FLOAT_EQ(dir1.x, dir2.x);
    EXPECT_FLOAT_EQ(dir1.y, dir2.y);

    EXPECT_FLOAT_EQ(m_emitter->getSpreadAngle(), emitter2.getSpreadAngle());
    EXPECT_FLOAT_EQ(m_emitter->getMinSpeed(), emitter2.getMinSpeed());
    EXPECT_FLOAT_EQ(m_emitter->getMaxSpeed(), emitter2.getMaxSpeed());
    EXPECT_FLOAT_EQ(m_emitter->getMinLifetime(), emitter2.getMinLifetime());
    EXPECT_FLOAT_EQ(m_emitter->getMaxLifetime(), emitter2.getMaxLifetime());
    EXPECT_FLOAT_EQ(m_emitter->getMinSize(), emitter2.getMinSize());
    EXPECT_FLOAT_EQ(m_emitter->getMaxSize(), emitter2.getMaxSize());
    EXPECT_FLOAT_EQ(m_emitter->getEmissionRate(), emitter2.getEmissionRate());
    EXPECT_FLOAT_EQ(m_emitter->getBurstCount(), emitter2.getBurstCount());

    Color sc1 = m_emitter->getStartColor();
    Color sc2 = emitter2.getStartColor();
    EXPECT_EQ(sc1.r, sc2.r);
    EXPECT_EQ(sc1.g, sc2.g);
    EXPECT_EQ(sc1.b, sc2.b);
    EXPECT_EQ(sc1.a, sc2.a);

    Color ec1 = m_emitter->getEndColor();
    Color ec2 = emitter2.getEndColor();
    EXPECT_EQ(ec1.r, ec2.r);
    EXPECT_EQ(ec1.g, ec2.g);
    EXPECT_EQ(ec1.b, ec2.b);
    EXPECT_EQ(ec1.a, ec2.a);

    EXPECT_FLOAT_EQ(m_emitter->getStartAlpha(), emitter2.getStartAlpha());
    EXPECT_FLOAT_EQ(m_emitter->getEndAlpha(), emitter2.getEndAlpha());

    Vec2 grav1 = m_emitter->getGravity();
    Vec2 grav2 = emitter2.getGravity();
    EXPECT_FLOAT_EQ(grav1.x, grav2.x);
    EXPECT_FLOAT_EQ(grav1.y, grav2.y);

    EXPECT_FLOAT_EQ(m_emitter->getMinRotationSpeed(), emitter2.getMinRotationSpeed());
    EXPECT_FLOAT_EQ(m_emitter->getMaxRotationSpeed(), emitter2.getMaxRotationSpeed());

    EXPECT_EQ(m_emitter->getFadeOut(), emitter2.getFadeOut());
    EXPECT_EQ(m_emitter->getShrink(), emitter2.getShrink());
    EXPECT_FLOAT_EQ(m_emitter->getShrinkEndScale(), emitter2.getShrinkEndScale());

    EXPECT_EQ(m_emitter->getMaxParticles(), emitter2.getMaxParticles());
    EXPECT_EQ(m_emitter->getZIndex(), emitter2.getZIndex());

    Vec2 off1 = m_emitter->getPositionOffset();
    Vec2 off2 = emitter2.getPositionOffset();
    EXPECT_FLOAT_EQ(off1.x, off2.x);
    EXPECT_FLOAT_EQ(off1.y, off2.y);

    EXPECT_EQ(m_emitter->getEmissionShape(), emitter2.getEmissionShape());
    EXPECT_FLOAT_EQ(m_emitter->getShapeRadius(), emitter2.getShapeRadius());

    Vec2 ss1 = m_emitter->getShapeSize();
    Vec2 ss2 = emitter2.getShapeSize();
    EXPECT_FLOAT_EQ(ss1.x, ss2.x);
    EXPECT_FLOAT_EQ(ss1.y, ss2.y);

    Vec2 ls1 = m_emitter->getLineStart();
    Vec2 ls2 = emitter2.getLineStart();
    EXPECT_FLOAT_EQ(ls1.x, ls2.x);
    EXPECT_FLOAT_EQ(ls1.y, ls2.y);

    Vec2 le1 = m_emitter->getLineEnd();
    Vec2 le2 = emitter2.getLineEnd();
    EXPECT_FLOAT_EQ(le1.x, le2.x);
    EXPECT_FLOAT_EQ(le1.y, le2.y);

    EXPECT_EQ(m_emitter->getEmitFromEdge(), emitter2.getEmitFromEdge());
    EXPECT_EQ(m_emitter->getEmitOutward(), emitter2.getEmitOutward());

    const auto& verts1 = m_emitter->getPolygonVertices();
    const auto& verts2 = emitter2.getPolygonVertices();
    ASSERT_EQ(verts1.size(), verts2.size());
    for (size_t i = 0; i < verts1.size(); ++i)
    {
        EXPECT_FLOAT_EQ(verts1[i].x, verts2[i].x);
        EXPECT_FLOAT_EQ(verts1[i].y, verts2[i].y);
    }
}

TEST_F(CParticleEmitterTest, BackwardCompatibilityOldFormat)
{
    // Test that old serialization format (with only offset, active, maxParticles) still works
    std::string oldJson = R"({
        "cParticleEmitter": {
            "offset": { "x": 5.0, "y": 10.0 },
            "active": false,
            "maxParticles": 100
        }
    })";

   Serialization::SSerialization::JsonValue value(oldJson);
    m_emitter->deserialize(value);

    // Check that the old fields are read correctly
    Vec2 offset = m_emitter->getPositionOffset();
    EXPECT_FLOAT_EQ(offset.x, 5.0f);
    EXPECT_FLOAT_EQ(offset.y, 10.0f);
    EXPECT_FALSE(m_emitter->isActive());
    EXPECT_EQ(m_emitter->getMaxParticles(), 100);

    // Other fields should remain at defaults
    Vec2 dir = m_emitter->getDirection();
    EXPECT_FLOAT_EQ(dir.x, 0.0f);
    EXPECT_FLOAT_EQ(dir.y, 1.0f);
    EXPECT_FLOAT_EQ(m_emitter->getSpreadAngle(), 0.5f);
}

TEST_F(CParticleEmitterTest, EmissionShapeEnumSerialization)
{
    // Test each emission shape type
    EmissionShape shapes[] = {EmissionShape::Point, EmissionShape::Circle, EmissionShape::Rectangle, EmissionShape::Line, EmissionShape::Polygon};

    for (int i = 0; i < 5; ++i)
    {
        m_emitter->setEmissionShape(shapes[i]);

        Serialization::JsonBuilder builder;
        m_emitter->serialize(builder);
        std::string json = builder.toString();

        CParticleEmitter emitter2;
       Serialization::SSerialization::JsonValue        value(json);
        emitter2.deserialize(value);

        EXPECT_EQ(m_emitter->getEmissionShape(), emitter2.getEmissionShape());
    }
}

TEST_F(CParticleEmitterTest, EmptyPolygonVerticesSerialization)
{
    m_emitter->clearPolygonVertices();

    Serialization::JsonBuilder builder;
    m_emitter->serialize(builder);
    std::string json = builder.toString();

    CParticleEmitter emitter2;
    // Set some vertices first, they should be cleared after deserialization
    emitter2.addPolygonVertex(Vec2(1, 1));
    emitter2.addPolygonVertex(Vec2(2, 2));    Serialization::SSerialization::JsonValue value(json);
    emitter2.deserialize(value);

    EXPECT_TRUE(emitter2.getPolygonVertices().empty());
}

TEST_F(CParticleEmitterTest, ColorAlphaChannelSerialization)
{
    m_emitter->setStartColor(Color(128, 64, 32, 200));
    m_emitter->setEndColor(Color(255, 128, 64, 100));

    Serialization::JsonBuilder builder;
    m_emitter->serialize(builder);
    std::string json = builder.toString();

    CParticleEmitter emitter2;
   Serialization::SSerialization::JsonValue        value(json);
    emitter2.deserialize(value);

    Color sc = emitter2.getStartColor();
    EXPECT_EQ(sc.r, 128);
    EXPECT_EQ(sc.g, 64);
    EXPECT_EQ(sc.b, 32);
    EXPECT_EQ(sc.a, 200);

    Color ec = emitter2.getEndColor();
    EXPECT_EQ(ec.r, 255);
    EXPECT_EQ(ec.g, 128);
    EXPECT_EQ(ec.b, 64);
    EXPECT_EQ(ec.a, 100);
}

TEST_F(CParticleEmitterTest, NegativeValuesSerialization)
{
    m_emitter->setGravity(Vec2(-5.0f, -10.0f));
    m_emitter->setPositionOffset(Vec2(-1.0f, -2.0f));
    m_emitter->setZIndex(-10);
    m_emitter->setMinRotationSpeed(-3.0f);
    m_emitter->setMaxRotationSpeed(-1.0f);

    Serialization::JsonBuilder builder;
    m_emitter->serialize(builder);
    std::string json = builder.toString();

    CParticleEmitter emitter2;
   Serialization::SSerialization::JsonValue        value(json);
    emitter2.deserialize(value);

    Vec2 gravity = emitter2.getGravity();
    EXPECT_FLOAT_EQ(gravity.x, -5.0f);
    EXPECT_FLOAT_EQ(gravity.y, -10.0f);

    Vec2 offset = emitter2.getPositionOffset();
    EXPECT_FLOAT_EQ(offset.x, -1.0f);
    EXPECT_FLOAT_EQ(offset.y, -2.0f);

    EXPECT_EQ(emitter2.getZIndex(), -10);
    EXPECT_FLOAT_EQ(emitter2.getMinRotationSpeed(), -3.0f);
    EXPECT_FLOAT_EQ(emitter2.getMaxRotationSpeed(), -1.0f);
}

TEST_F(CParticleEmitterTest, ZeroValuesSerialization)
{
    m_emitter->setSpreadAngle(0.0f);
    m_emitter->setMinSpeed(0.0f);
    m_emitter->setEmissionRate(0.0f);
    m_emitter->setBurstCount(0.0f);
    m_emitter->setGravity(Vec2(0.0f, 0.0f));

    Serialization::JsonBuilder builder;
    m_emitter->serialize(builder);
    std::string json = builder.toString();

    CParticleEmitter emitter2;
   Serialization::SSerialization::JsonValue        value(json);
    emitter2.deserialize(value);

    EXPECT_FLOAT_EQ(emitter2.getSpreadAngle(), 0.0f);
    EXPECT_FLOAT_EQ(emitter2.getMinSpeed(), 0.0f);
    EXPECT_FLOAT_EQ(emitter2.getEmissionRate(), 0.0f);
    EXPECT_FLOAT_EQ(emitter2.getBurstCount(), 0.0f);

    Vec2 gravity = emitter2.getGravity();
    EXPECT_FLOAT_EQ(gravity.x, 0.0f);
    EXPECT_FLOAT_EQ(gravity.y, 0.0f);
}

