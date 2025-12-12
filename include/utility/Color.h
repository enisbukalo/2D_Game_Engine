#ifndef COLOR_H
#define COLOR_H

#include <cstdint>

/**
 * @brief Abstraction for color representation with RGBA components
 *
 * @description
 * Color provides a platform-agnostic color representation using RGBA values.
 * Renderer backends should perform any API-specific conversion internally.
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
