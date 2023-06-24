#pragma once

#include <cmath>

namespace Maths
{
#pragma region Constants & common math functions
	const float PI = 3.1415926535897932384626f;
	const float TAU = 6.283185307179586476925f;

	inline float cos(float x) { return cosf(x); }
	inline float sin(float x) { return sinf(x); }
	inline float tan(float x) { return tanf(x); }
	inline float atan(float x) { return atanf(x); }
	inline float acos(float x) { return acosf(x); }

	constexpr inline float toRadians(float degrees) { return degrees * TAU / 360.f; };
	constexpr inline float toDegrees(float radians) { return radians * 360.f / TAU; };

	template<typename T>
	inline T min(T x, T y) { return x < y ? x : y; };
	template<typename T>
	inline T max(T x, T y) { return x > y ? x : y; };
	template<typename T>
	inline T clamp(T min, T max, T val) { return val < min ? min : val > max ? max : val; }

	inline float abs(float x) { return x > 0.f ? x : x * -1.f; }
	inline int abs(int x) { return x > 0 ? x : x * -1; }

	inline int pow(int base, int power)
	{
		int result = base;
		for (int i = 1; i < power; i++)
		{
			result *= base;
		}
		return result;
	}

	inline float pow(float base, int power)
	{
		float result = base;
		for (int i = 1; i < power; i++)
		{
			result *= base;
		}
		return result;
	}

	inline float lerp(float initial, float final, float factor) { return (1.f - factor) * initial + (factor * final); };
	inline float lerp(int initial, int final, float factor) { return (1.f - factor) * initial + (factor * final); };

#pragma endregion
}