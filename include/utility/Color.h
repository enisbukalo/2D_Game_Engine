#ifndef COLOR_H
#define COLOR_H

#include <SFML/Graphics/Color.hpp>
#include <cstdint>

/**
 * @brief Abstraction for color representation with RGBA components
 *
 * @description
 * Color provides a platform-agnostic color representation using RGBA values.
 * It can be converted to and from SFML's color type for rendering, but provides
 * a clean interface for future portability to other rendering backends.
 */
struct Color
{
    uint8_t r;  ///< Red component (0-255)
    uint8_t g;  ///< Green component (0-255)
    uint8_t b;  ///< Blue component (0-255)
    uint8_t a;  ///< Alpha/opacity component (0-255)

    /**
     * @brief Default constructor - creates opaque white
     */
    Color() : r(255), g(255), b(255), a(255) {}

    /**
     * @brief Constructs a color with specified RGB values and optional alpha
     * @param red Red component (0-255)
     * @param green Green component (0-255)
     * @param blue Blue component (0-255)
     * @param alpha Alpha component (0-255), defaults to 255 (opaque)
     */
    Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) : r(red), g(green), b(blue), a(alpha) {}

    /**
     * @brief Converts from SFML color to engine Color
     * @param sfColor SFML color object
     * @return Equivalent engine Color
     */
    static Color fromSFML(const sf::Color& sfColor)
    {
        return Color(sfColor.r, sfColor.g, sfColor.b, sfColor.a);
    }

    /**
     * @brief Converts engine Color to SFML color
     * @return Equivalent SFML color
     */
    sf::Color toSFML() const
    {
        return sf::Color(r, g, b, a);
    }

    // Common color constants
    static const Color White;
    static const Color Black;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Magenta;
    static const Color Cyan;
    static const Color Transparent;

    /**
     * @brief Equality comparison operator
     */
    bool operator==(const Color& other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    /**
     * @brief Inequality comparison operator
     */
    bool operator!=(const Color& other) const
    {
        return !(*this == other);
    }
};

#endif  // COLOR_H
