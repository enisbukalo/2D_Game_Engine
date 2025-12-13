#include <gtest/gtest.h>

#include <cmath>

#include <Color.h>
#include <Vec2.h>

namespace
{
constexpr float kEpsilon = 1e-5f;
constexpr float kPi      = 3.14159265358979323846f;
}

TEST(Vec2Test, LengthAndLengthSquaredConsistent)
{
	Vec2 v{3.0f, 4.0f};
	EXPECT_NEAR(v.length(), 5.0f, kEpsilon);
	EXPECT_NEAR(v.lengthSquared(), 25.0f, kEpsilon);
}

TEST(Vec2Test, NormalizeNonZeroProducesUnitLength)
{
	Vec2 v{3.0f, 4.0f};
	v.normalize();
	EXPECT_NEAR(v.length(), 1.0f, 1e-4f);
	EXPECT_NEAR(v.x, 0.6f, 1e-4f);
	EXPECT_NEAR(v.y, 0.8f, 1e-4f);
}

TEST(Vec2Test, NormalizeZeroVectorRemainsZeroAndFinite)
{
	Vec2 v{0.0f, 0.0f};
	v.normalize();

	EXPECT_TRUE(std::isfinite(v.x));
	EXPECT_TRUE(std::isfinite(v.y));
	EXPECT_NEAR(v.x, 0.0f, kEpsilon);
	EXPECT_NEAR(v.y, 0.0f, kEpsilon);
	EXPECT_NEAR(v.length(), 0.0f, kEpsilon);
}

TEST(Vec2Test, NormalizeAlreadyUnitVectorIsStable)
{
	Vec2 v{0.0f, 1.0f};
	v.normalize();
	EXPECT_NEAR(v.x, 0.0f, kEpsilon);
	EXPECT_NEAR(v.y, 1.0f, kEpsilon);
	EXPECT_TRUE(std::isfinite(v.x));
	EXPECT_TRUE(std::isfinite(v.y));
}

TEST(Vec2Test, RotateByPiOver2)
{
	Vec2 v{1.0f, 0.0f};
	v.rotate(kPi * 0.5f);

	EXPECT_NEAR(v.x, 0.0f, 1e-4f);
	EXPECT_NEAR(v.y, 1.0f, 1e-4f);
}

TEST(Vec2Test, RotateZeroVectorStaysZero)
{
	Vec2 v{0.0f, 0.0f};
	v.rotate(kPi * 0.5f);
	EXPECT_NEAR(v.x, 0.0f, kEpsilon);
	EXPECT_NEAR(v.y, 0.0f, kEpsilon);
}

TEST(Vec2Test, EqualityUsesEpsilon)
{
	Vec2 a{1.0f, 1.0f};
	Vec2 b{1.0f + 5e-5f, 1.0f - 5e-5f};
	Vec2 c{1.0f + 2e-4f, 1.0f};

	EXPECT_TRUE(a == b);
	EXPECT_FALSE(a != b);

	EXPECT_FALSE(a == c);
	EXPECT_TRUE(a != c);
}

TEST(Vec2Test, EqualityAtEpsilonBoundaryIsNotEqual)
{
	// Vec2::operator== uses strict '< EPSILON' checks.
	constexpr float kImplEps = 1e-4f;
	Vec2 a{0.0f, 0.0f};
	Vec2 b{kImplEps, 0.0f};
	EXPECT_FALSE(a == b);
}

TEST(Vec2Test, DotProduct)
{
	Vec2 a{2.0f, 3.0f};
	Vec2 b{4.0f, -5.0f};
	EXPECT_NEAR(a.dot(b), (2.0f * 4.0f + 3.0f * -5.0f), kEpsilon);
}

TEST(Vec2Test, DotWithSelfEqualsLengthSquared)
{
	Vec2 v{-2.5f, 7.0f};
	EXPECT_NEAR(v.dot(v), v.lengthSquared(), kEpsilon);
}

TEST(ColorTest, DefaultIsWhite)
{
	Color c;
	EXPECT_EQ(c, Color::White);
}

TEST(ColorTest, TransparentHasZeroAlpha)
{
	EXPECT_EQ(Color::Transparent.a, 0);
	EXPECT_EQ(Color::Transparent.r, 0);
	EXPECT_EQ(Color::Transparent.g, 0);
	EXPECT_EQ(Color::Transparent.b, 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Additional Vec2 coverage tests
// ─────────────────────────────────────────────────────────────────────────────

TEST(Vec2Test, AddModifiesInPlace)
{
	Vec2 v{1.0f, 2.0f};
	Vec2 other{3.0f, 4.0f};
	Vec2& result = v.add(other);

	EXPECT_NEAR(v.x, 4.0f, kEpsilon);
	EXPECT_NEAR(v.y, 6.0f, kEpsilon);
	EXPECT_EQ(&result, &v);
}

TEST(Vec2Test, ScaleModifiesInPlace)
{
	Vec2 v{2.0f, 3.0f};
	Vec2& result = v.scale(2.5f);

	EXPECT_NEAR(v.x, 5.0f, kEpsilon);
	EXPECT_NEAR(v.y, 7.5f, kEpsilon);
	EXPECT_EQ(&result, &v);
}

TEST(Vec2Test, DistanceCalculatesCorrectly)
{
	Vec2 a{0.0f, 0.0f};
	Vec2 b{3.0f, 4.0f};

	EXPECT_NEAR(a.distance(b), 5.0f, kEpsilon);
	EXPECT_NEAR(b.distance(a), 5.0f, kEpsilon);
}

TEST(Vec2Test, DistanceSquaredCalculatesCorrectly)
{
	Vec2 a{0.0f, 0.0f};
	Vec2 b{3.0f, 4.0f};

	EXPECT_NEAR(a.distanceSquared(b), 25.0f, kEpsilon);
	EXPECT_NEAR(b.distanceSquared(a), 25.0f, kEpsilon);
}

TEST(Vec2Test, OperatorPlusEqualsModifiesInPlace)
{
	Vec2 v{1.0f, 2.0f};
	Vec2 other{5.0f, 6.0f};
	v += other;

	EXPECT_NEAR(v.x, 6.0f, kEpsilon);
	EXPECT_NEAR(v.y, 8.0f, kEpsilon);
}

TEST(Vec2Test, OperatorMinusEqualsModifiesInPlace)
{
	Vec2 v{5.0f, 6.0f};
	Vec2 other{1.0f, 2.0f};
	v -= other;

	EXPECT_NEAR(v.x, 4.0f, kEpsilon);
	EXPECT_NEAR(v.y, 4.0f, kEpsilon);
}

TEST(Vec2Test, OperatorTimesEqualsScales)
{
	Vec2 v{2.0f, 3.0f};
	v *= 3.0f;

	EXPECT_NEAR(v.x, 6.0f, kEpsilon);
	EXPECT_NEAR(v.y, 9.0f, kEpsilon);
}

TEST(Vec2Test, OperatorDivideEqualsScales)
{
	Vec2 v{6.0f, 9.0f};
	v /= 3.0f;

	EXPECT_NEAR(v.x, 2.0f, kEpsilon);
	EXPECT_NEAR(v.y, 3.0f, kEpsilon);
}

TEST(Vec2Test, OperatorPlusReturnsNewVector)
{
	Vec2 a{1.0f, 2.0f};
	Vec2 b{3.0f, 4.0f};
	Vec2 c = a + b;

	EXPECT_NEAR(c.x, 4.0f, kEpsilon);
	EXPECT_NEAR(c.y, 6.0f, kEpsilon);
	// Originals unchanged
	EXPECT_NEAR(a.x, 1.0f, kEpsilon);
	EXPECT_NEAR(b.x, 3.0f, kEpsilon);
}

TEST(Vec2Test, OperatorMinusReturnsNewVector)
{
	Vec2 a{5.0f, 6.0f};
	Vec2 b{1.0f, 2.0f};
	Vec2 c = a - b;

	EXPECT_NEAR(c.x, 4.0f, kEpsilon);
	EXPECT_NEAR(c.y, 4.0f, kEpsilon);
	// Originals unchanged
	EXPECT_NEAR(a.x, 5.0f, kEpsilon);
	EXPECT_NEAR(b.x, 1.0f, kEpsilon);
}

TEST(Vec2Test, OperatorTimesReturnsNewVector)
{
	Vec2 v{2.0f, 3.0f};
	Vec2 result = v * 2.0f;

	EXPECT_NEAR(result.x, 4.0f, kEpsilon);
	EXPECT_NEAR(result.y, 6.0f, kEpsilon);
	// Original unchanged
	EXPECT_NEAR(v.x, 2.0f, kEpsilon);
}

TEST(Vec2Test, OperatorDivideReturnsNewVector)
{
	Vec2 v{6.0f, 9.0f};
	Vec2 result = v / 3.0f;

	EXPECT_NEAR(result.x, 2.0f, kEpsilon);
	EXPECT_NEAR(result.y, 3.0f, kEpsilon);
	// Original unchanged
	EXPECT_NEAR(v.x, 6.0f, kEpsilon);
}
