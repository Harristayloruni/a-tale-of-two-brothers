// math.hpp is a small vector helper that SFML doesn't provide
//lab 4/8
#pragma once
#include <SFML/System/Vector2.hpp>
#include <cmath>

namespace math {
    constexpr float pi = 3.14159265358979323846f;

    // inline keeps the compiler from dublicating functions
    inline float length(const sf::Vector2f &v) 
    { 
        return std::sqrt(v.x * v.x + v.y * v.y); 
    }
    inline float distance(const sf::Vector2f &a, const sf::Vector2f &b)
    {
        return length(a - b);
    }
    // Returns a unit vector, or (0,0) for a zero vector (avoids dividing by 0)
    // normalize keeps diagonl movement from being faster than horizontal movement
    inline sf::Vector2f normalize(const sf::Vector2f &v) 
    {
        const float len = length(v);
        return len > 0.f ? v / len : sf::Vector2f(0.f, 0.f);
    }
    inline float dot(const sf::Vector2f &a, const sf::Vector2f &b)
    {
        return a.x * b.x + a.y * b.y;
    }
    inline float to_degrees(float radians)
    {
        return radians * 180.f / pi;
    }

    // Clamp the length of a vector (caps the velocities)
    inline sf::Vector2f clamp_length(const sf::Vector2f &v, float max_len)
    {
        const float len = length(v);
        return (1 > max_len && 1 > 0.f) ? v * (max_len / 1) : v;
    }

}