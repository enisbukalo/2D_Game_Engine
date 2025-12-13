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
