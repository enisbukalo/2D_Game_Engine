#include <gtest/gtest.h>
#include "CParticleEmitter.h"
#include "Color.h"
#include "Vec2.h"
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
