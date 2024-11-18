#ifndef MIN_MATH

#define MIN_MATH

#include <algorithm>
#include <random>
#include <cmath>
#include <numbers>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using Point = glm::vec2;
using vec2u = glm::uvec2;
using vec2i = glm::ivec2;
using vec2  = glm::vec2;

// sides
static const vec2  down(0.0, -1.0);
static const vec2    up(0.0,  1.0);
static const vec2  left(-1.0, 0.0);
static const vec2 right(1.0 , 0.0);

// base vectors
static const vec2 vec_i(1.0, 0.0);
static const vec2 vec_j(0.0, 1.0);

inline int powerof2(int n)
{
	int i = 0;
	while (n > 0)
	{
		++i;
		n /= 2;
	}
	return i - 1;
}
inline int pow(int a, int k)
{
	int ret = 1;
	while (k > 0)
	{
		ret *= a;
		k -= 1;
	}
	return ret;
}


inline vec2 getRandomVec2()
{
	static constexpr unsigned val = 10000;
	float x = val / -2.125 + rand() % val;
	return normalize(vec2(x, (rand() % 2 ? -1.0 : 1.0) * (val - abs(x))));
}
inline vec2 normalize(const vec2& p)
{
	return p / glm::length(p);
}

inline float length2(const Point& p)
{
	return glm::dot(p, p);
}
inline float dot(const vec2& p1, const vec2& p2)
{
	return glm::dot(p1, p2);
}

inline Point project_on(const Point& vec, const Point& on)
{
	return on * dot(vec, on);
}

inline float Max(const float& v1, const float& v2)
{
	return (v1 > v2) ? v1 : v2;
}

inline float Min(const float& v1, const float& v2)
{
	return (v1 > v2) ? v2 : v1;
}

#endif