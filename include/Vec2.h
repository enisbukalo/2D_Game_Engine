#ifndef VEC2_H
#define VEC2_H

#include <math.h>

class Vec2 {
public:
  float x = 0.0;
  float y = 0.0f;

  Vec2() {};
  Vec2(float x, float y) : x(x), y(y) {};

  float length() { return sqrt(x * x + y * y); };

  Vec2 &normalize()
  {
    float l = length();
    x /= l;
    y /= l;
    return *this;
  };
  Vec2 &add(const Vec2 &other)
  {
    x += other.x;
    y += other.y;
    return *this;
  };
  Vec2 &scale(float multiplier)
  {
    x *= multiplier;
    y *= multiplier;
    return *this;
  };
  Vec2 &rotate(float angle)
  {
    float c = cos(angle);
    float s = sin(angle);
    float newX = x * c - y * s;
    float newY = x * s + y * c;
    x = newX;
    y = newY;
    return *this;
  };
  float distance(const Vec2 &other) { return sqrtf((other.x - x) * (other.x - x) + (other.y - y) * (other.y - y)); };

#pragma region Operators
  Vec2 &operator+=(const Vec2 &other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }
  Vec2 &operator-=(const Vec2 &other)
  {
    x -= other.x;
    y -= other.y;
    return *this;
  }
  Vec2 operator+(const Vec2 &other) const { return Vec2(x + other.x, y + other.y); }
  Vec2 operator-(const Vec2 &other) const { return Vec2(x - other.x, y - other.y); }
  Vec2 operator*(const float &multiplier) const { return Vec2(x * multiplier, y * multiplier); }
  Vec2 operator/(const float &divisor) const { return Vec2(x / divisor, y / divisor); }
#pragma endregion
};

#endif // VEC2_H