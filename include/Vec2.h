#ifndef VEC2_H
#define VEC2_H

#include <math.h>

/**
 * @brief A 2D vector class for handling positions, velocities, and other 2D quantities
 *
 * @description
 * Vec2 provides basic 2D vector operations including arithmetic operations,
 * normalization, rotation, and distance calculations. It's used throughout
 * the engine for representing positions, velocities, scales, and forces.
 */
class Vec2
{
public:
    float x = 0.0f;  ///< X component of the vector
    float y = 0.0f;  ///< Y component of the vector

    Vec2() = default;

    /**
     * @brief Constructs a vector with given x and y components
     * @param x The x component of the vector
     * @param y The y component of the vector
     */
    Vec2(float x, float y) : x(x), y(y) {}

    /**
     * @brief Calculates the length (magnitude) of the vector
     * @return The length of the vector
     */
    float length();

    /**
     * @brief Normalizes the vector to unit length
     * @return Reference to this vector after normalization
     */
    Vec2& normalize();

    /**
     * @brief Adds another vector to this vector
     * @param other The vector to add
     * @return Reference to this vector after addition
     */
    Vec2& add(const Vec2& other);

    /**
     * @brief Scales the vector by a scalar value
     * @param multiplier The scaling factor
     * @return Reference to this vector after scaling
     */
    Vec2& scale(float multiplier);

    /**
     * @brief Rotates the vector by an angle
     * @param angle The rotation angle in radians
     * @return Reference to this vector after rotation
     */
    Vec2& rotate(float angle);

    /**
     * @brief Calculates the Euclidean distance to another vector
     * @param other The vector to calculate distance to
     * @return The distance between the two vectors
     */
    float distance(const Vec2& other);

    /**
     * @brief Adds another vector to this vector
     * @param other The vector to add
     * @return Reference to this vector after addition
     */
    Vec2& operator+=(const Vec2& other);

    /**
     * @brief Subtracts another vector from this vector
     * @param other The vector to subtract
     * @return Reference to this vector after subtraction
     */
    Vec2& operator-=(const Vec2& other);

    /**
     * @brief Creates a new vector that is the sum of this vector and another
     * @param other The vector to add
     * @return The resulting vector
     */
    Vec2 operator+(const Vec2& other) const;

    /**
     * @brief Creates a new vector that is the difference of this vector and another
     * @param other The vector to subtract
     * @return The resulting vector
     */
    Vec2 operator-(const Vec2& other) const;

    /**
     * @brief Creates a new vector that is this vector scaled by a factor
     * @param multiplier The scaling factor
     * @return The scaled vector
     */
    Vec2 operator*(const float& multiplier) const;

    /**
     * @brief Creates a new vector that is this vector divided by a factor
     * @param divisor The division factor
     * @return The divided vector
     */
    Vec2 operator/(const float& divisor) const;

    /**
     * @brief Checks if two vectors are equal
     * @param other The vector to compare with
     * @return True if vectors are equal, false otherwise
     */
    bool operator==(const Vec2& other) const;

    /**
     * @brief Checks if two vectors are not equal
     * @param other The vector to compare with
     * @return True if vectors are not equal, false otherwise
     */
    bool operator!=(const Vec2& other) const;

    /**
     * @brief Scales the vector by a scalar value
     * @param scalar The scaling factor
     * @return Reference to this vector after scaling
     */
    Vec2& operator*=(float scalar);

    /**
     * @brief Divides the vector by a scalar value
     * @param scalar The division factor
     * @return Reference to this vector after division
     */
    Vec2& operator/=(float scalar);

    /**
     * @brief Returns the dot product of two vectors
     * @param other The vector to calculate the dot product with
     * @return The dot product of the two vectors
     */
    float dot(const Vec2& other) const;
};

#endif  // VEC2_H