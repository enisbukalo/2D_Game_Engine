#ifndef VEC2_H
#define VEC2_H

#include <math.h>

// A 2D vector class
class Vec2
{
public:
#pragma region Variables
    float x = 0.0f;
    float y = 0.0f;
#pragma endregion

#pragma region Constructors
    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}
#pragma endregion

#pragma region Methods
    // Calculate the length of the vector
    float length();

    // Normalize the vector
    Vec2& normalize();

    // Add two vectors
    Vec2& add(const Vec2& other);

    // Scale the vector
    Vec2& scale(float multiplier);

    // Rotate the vector
    Vec2& rotate(float angle);

    // Calculate the distance between two vectors
    float distance(const Vec2& other);
#pragma endregion

#pragma region Operators
    Vec2& operator+=(const Vec2& other);
    Vec2& operator-=(const Vec2& other);
    Vec2  operator+(const Vec2& other) const;
    Vec2  operator-(const Vec2& other) const;
    Vec2  operator*(const float& multiplier) const;
    Vec2  operator/(const float& divisor) const;
#pragma endregion
};

#endif  // VEC2_H