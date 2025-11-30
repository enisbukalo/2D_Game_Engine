#include <gtest/gtest.h>
#include "Color.h"
#include <SFML/Graphics/Color.hpp>

class ColorTest : public ::testing::Test
{
protected:
};

TEST_F(ColorTest, DefaultConstruction)
{
    Color color;
    EXPECT_EQ(color.r, 255);
    EXPECT_EQ(color.g, 255);
    EXPECT_EQ(color.b, 255);
    EXPECT_EQ(color.a, 255);
}

TEST_F(ColorTest, ParameterizedConstruction)
{
    Color color(100, 150, 200, 128);
    EXPECT_EQ(color.r, 100);
    EXPECT_EQ(color.g, 150);
    EXPECT_EQ(color.b, 200);
    EXPECT_EQ(color.a, 128);
}

TEST_F(ColorTest, ThreeParameterConstruction)
{
    Color color(50, 100, 150);
    EXPECT_EQ(color.r, 50);
    EXPECT_EQ(color.g, 100);
    EXPECT_EQ(color.b, 150);
    EXPECT_EQ(color.a, 255); // Default alpha
}

TEST_F(ColorTest, EqualityOperator)
{
    Color color1(100, 150, 200, 255);
    Color color2(100, 150, 200, 255);
    Color color3(100, 150, 201, 255);

    EXPECT_TRUE(color1 == color2);
    EXPECT_FALSE(color1 == color3);
}

TEST_F(ColorTest, InequalityOperator)
{
    Color color1(100, 150, 200, 255);
    Color color2(100, 150, 200, 255);
    Color color3(100, 150, 201, 255);

    EXPECT_FALSE(color1 != color2);
    EXPECT_TRUE(color1 != color3);
}

TEST_F(ColorTest, AlphaDifference)
{
    Color color1(100, 150, 200, 255);
    Color color2(100, 150, 200, 128);

    EXPECT_FALSE(color1 == color2);
    EXPECT_TRUE(color1 != color2);
}

TEST_F(ColorTest, ToSFMLConversion)
{
    Color color(50, 100, 150, 200);
    sf::Color sfColor = color.toSFML();

    EXPECT_EQ(sfColor.r, 50);
    EXPECT_EQ(sfColor.g, 100);
    EXPECT_EQ(sfColor.b, 150);
    EXPECT_EQ(sfColor.a, 200);
}

TEST_F(ColorTest, FromSFMLConversion)
{
    sf::Color sfColor(75, 125, 175, 225);
    Color color = Color::fromSFML(sfColor);

    EXPECT_EQ(color.r, 75);
    EXPECT_EQ(color.g, 125);
    EXPECT_EQ(color.b, 175);
    EXPECT_EQ(color.a, 225);
}

TEST_F(ColorTest, RoundTripConversion)
{
    Color original(10, 20, 30, 40);
    sf::Color sfColor = original.toSFML();
    Color converted = Color::fromSFML(sfColor);

    EXPECT_EQ(original, converted);
}

TEST_F(ColorTest, PredefinedColorBlack)
{
    EXPECT_EQ(Color::Black.r, 0);
    EXPECT_EQ(Color::Black.g, 0);
    EXPECT_EQ(Color::Black.b, 0);
    EXPECT_EQ(Color::Black.a, 255);
}

TEST_F(ColorTest, PredefinedColorWhite)
{
    EXPECT_EQ(Color::White.r, 255);
    EXPECT_EQ(Color::White.g, 255);
    EXPECT_EQ(Color::White.b, 255);
    EXPECT_EQ(Color::White.a, 255);
}

TEST_F(ColorTest, PredefinedColorRed)
{
    EXPECT_EQ(Color::Red.r, 255);
    EXPECT_EQ(Color::Red.g, 0);
    EXPECT_EQ(Color::Red.b, 0);
    EXPECT_EQ(Color::Red.a, 255);
}

TEST_F(ColorTest, PredefinedColorGreen)
{
    EXPECT_EQ(Color::Green.r, 0);
    EXPECT_EQ(Color::Green.g, 255);
    EXPECT_EQ(Color::Green.b, 0);
    EXPECT_EQ(Color::Green.a, 255);
}

TEST_F(ColorTest, PredefinedColorBlue)
{
    EXPECT_EQ(Color::Blue.r, 0);
    EXPECT_EQ(Color::Blue.g, 0);
    EXPECT_EQ(Color::Blue.b, 255);
    EXPECT_EQ(Color::Blue.a, 255);
}

TEST_F(ColorTest, PredefinedColorYellow)
{
    EXPECT_EQ(Color::Yellow.r, 255);
    EXPECT_EQ(Color::Yellow.g, 255);
    EXPECT_EQ(Color::Yellow.b, 0);
    EXPECT_EQ(Color::Yellow.a, 255);
}

TEST_F(ColorTest, PredefinedColorMagenta)
{
    EXPECT_EQ(Color::Magenta.r, 255);
    EXPECT_EQ(Color::Magenta.g, 0);
    EXPECT_EQ(Color::Magenta.b, 255);
    EXPECT_EQ(Color::Magenta.a, 255);
}

TEST_F(ColorTest, PredefinedColorCyan)
{
    EXPECT_EQ(Color::Cyan.r, 0);
    EXPECT_EQ(Color::Cyan.g, 255);
    EXPECT_EQ(Color::Cyan.b, 255);
    EXPECT_EQ(Color::Cyan.a, 255);
}

TEST_F(ColorTest, PredefinedColorTransparent)
{
    EXPECT_EQ(Color::Transparent.r, 0);
    EXPECT_EQ(Color::Transparent.g, 0);
    EXPECT_EQ(Color::Transparent.b, 0);
    EXPECT_EQ(Color::Transparent.a, 0);
}

TEST_F(ColorTest, BoundaryValuesMinimum)
{
    Color color(0, 0, 0, 0);
    EXPECT_EQ(color.r, 0);
    EXPECT_EQ(color.g, 0);
    EXPECT_EQ(color.b, 0);
    EXPECT_EQ(color.a, 0);
}

TEST_F(ColorTest, BoundaryValuesMaximum)
{
    Color color(255, 255, 255, 255);
    EXPECT_EQ(color.r, 255);
    EXPECT_EQ(color.g, 255);
    EXPECT_EQ(color.b, 255);
    EXPECT_EQ(color.a, 255);
}

TEST_F(ColorTest, SFMLConversionWithTransparent)
{
    Color color = Color::Transparent;
    sf::Color sfColor = color.toSFML();

    EXPECT_EQ(sfColor.r, 0);
    EXPECT_EQ(sfColor.g, 0);
    EXPECT_EQ(sfColor.b, 0);
    EXPECT_EQ(sfColor.a, 0);

    Color converted = Color::fromSFML(sfColor);
    EXPECT_EQ(converted, Color::Transparent);
}

TEST_F(ColorTest, AllPredefinedColorsUnique)
{
    // Ensure all predefined colors are distinct
    EXPECT_NE(Color::Black, Color::White);
    EXPECT_NE(Color::Red, Color::Green);
    EXPECT_NE(Color::Red, Color::Blue);
    EXPECT_NE(Color::Green, Color::Blue);
    EXPECT_NE(Color::Yellow, Color::Magenta);
    EXPECT_NE(Color::Yellow, Color::Cyan);
    EXPECT_NE(Color::Magenta, Color::Cyan);
    EXPECT_NE(Color::Black, Color::Transparent);
}
