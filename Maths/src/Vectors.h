#pragma once

#include <iostream>
#include "Common.h"

namespace Maths
{
#pragma region Union Declarations
	union vec2
	{
		vec2() = default;
		vec2(float x, float y)
			: x(x), y(y)
		{}

		float e[2];

		struct { float x; float y; };
	};

	union vec2i
	{
		vec2i() = default;
		vec2i(int x, int y)
			: x(x), y(y)
		{}

		int e[2];

		struct { int x; int y; };
	};

	union vec3
	{
		vec3() = default;
		constexpr vec3(float d)
			: x(d), y(d), z(d)
		{}
		constexpr vec3(float x, float y, float z)
			: x(x), y(y), z(z)
		{}

		vec3(vec2 xy, float z)
			: x(xy.x), y(xy.y), z(z)
		{}

		float e[3];

		struct { float x, y, z; };
		struct { float r, g, b; };

		vec2 xy;
		vec2 uv;
	};

	union vec3i
	{
		vec3i() = default;
		vec3i(int d)
			: x(d), y(d), z(d)
		{}
		vec3i(int x, int y, int z)
			: x(x), y(y), z(z)
		{}

		vec3i(vec2i xy, int z)
			: x(xy.x), y(xy.y), z(z)
		{}

		int e[3];

		struct { int v, vt, vn; };
		struct { int x, y, z; };
		struct { int r, g, b; };

		vec2i xy;
		vec2i uv;
	};

	union vec4
	{
		vec4() = default;
		vec4(float d)
			: x(d), y(d), z(d), w(d)
		{}
		vec4(float x, float y, float z, float w)
			: x(x), y(y), z(z), w(w)
		{}
		vec4(vec3 xyz, float w)
			: x(xyz.x), y(xyz.y), z(xyz.z), w(w)
		{}

		float e[4];

		struct { float x, y, z, w; };
		struct { float r, g, b, a; };

		vec3 rgb;
		vec3 xyz;
		vec3 xy;
	};

	union vec4i
	{
		vec4i() = default;
		vec4i(int d)
			: x(d), y(d), z(d), w(d)
		{}
		vec4i(int x, int y, int z, int w)
			: x(x), y(y), z(z), w(w)
		{}
		vec4i(vec3i xyz, int w)
			: x(xyz.x), y(xyz.y), z(xyz.z), w(w)
		{}

		int e[4];

		struct { int x, y, z, w; };
		struct { int r, g, b, a; };

		vec3i rgb;
		vec3i xyz;
		vec3i xy;
	};
#pragma endregion

#pragma region Operator Overloads
	inline vec2 operator+(const vec2& a, const vec2& b) { return { a.x + b.x, a.y + b.y }; }

	inline vec2 operator+=(vec2& a, const vec2& b)
	{
		a = a + b;
		return a;
	}

	inline vec2 operator-(const vec2& a) { return { -a.x, -a.y }; }

	inline vec2 operator-(const vec2& a, const vec2& b) { return { a.x - b.x, a.y - b.y }; }
	inline vec2 operator-=(vec2& a, const vec2& b)
	{
		a = a - b;
		return a;
	}

	inline vec2 operator/(vec2& v, float w) { return { v.x / w, v.y / w }; }

	inline vec2 operator*(vec2& v, float k) { return { v.x * k, v.y * k }; }
	inline vec2 operator*(float k, const vec2& v) { return { v.x * k, v.y * k }; }
	inline vec2 operator*(const vec2& a, const vec2& b) { return { a.x * b.x, a.y * b.y }; }

	inline vec2 operator*=(vec2& a, float k)
	{
		a = a * k;
		return a;
	}

	inline vec2 operator*=(vec2& a, vec2 b)
	{
		a = a * b;
		return a;
	}

	inline vec3 operator+(const vec3& a, const vec3& b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }

	inline vec3 operator+=(vec3& a, const vec3& b)
	{
		a = a + b;
		return a;
	}

	inline vec3 operator-(const vec3& a) { return { -a.x, -a.y, -a.z }; }

	inline vec3 operator-(const vec3& a, const vec3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
	inline vec3 operator-=(vec3& a, const vec3& b)
	{
		a = a - b;
		return a;
	}

	inline vec3 operator/(vec3& v, float w) { return { v.x / w, v.y / w, v.z / w }; }

	inline vec3 operator*(vec3& v, float k) { return { v.x * k, v.y * k, v.z * k }; }
	inline vec3 operator*(float k, const vec3& v) { return { v.x * k, v.y * k, v.z * k }; }
	inline vec3 operator*(const vec3& a, const vec3& b) { return { a.x * b.x, a.y * b.y, a.z * b.z }; }

	inline vec3 operator*=(vec3& a, float k)
	{
		a = a * k;
		return a;
	}

	inline vec3 operator*=(vec3& a, vec3 b)
	{
		a = a * b;
		return a;
	}

	inline vec4 operator+(const vec4& a, const vec4& b) { return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }

	inline vec4 operator+=(vec4& a, const vec4& b)
	{
		a = a + b;
		return a;
	}

	inline vec4 operator*(float k, const vec4& v) { return { v.x * k, v.y * k, v.z * k, v.w * k }; }
	inline vec4 operator*(const vec4& v, float k) { return { v.x * k, v.y * k, v.z * k, v.w * k }; }
	inline vec4 operator*(const vec4& a, const vec4& b) { return { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w }; }

	inline vec4 operator*=(vec4& a, float k)
	{
		a = a * k;
		return a;
	}

	inline vec4 operator*=(vec4& a, vec4 b)
	{
		a = a * b;
		return a;
	}

	inline std::ostream& operator<<(std::ostream& os, const vec2& v)
	{
		os << "(" << v.x << ", " << v.y << ")";
		return os;
	}

	inline std::ostream& operator<<(std::ostream& os, const vec3& v)
	{
		os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
		return os;
	}

	inline std::ostream& operator<<(std::ostream& os, const vec4& v)
	{
		os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
		return os;
	}
#pragma endregion

#pragma Functions
	vec3 StrtoVec3(const std::string& str);

	vec3 normalize(const vec3& v);
	vec4 normalize(const vec4& v);
	vec3 crossProduct(const vec3& v, const vec3& k);

	inline float dotProduct(const vec3& ua, const vec3& ub) { return ua.x * ub.x + ua.y * ub.y + ua.z * ub.z; };
	inline float dotProduct(const vec4& ua, const vec4& ub) { return ua.x * ub.x + ua.y * ub.y + ua.z * ub.z + ua.w * ub.w; };

	inline float magnitude(const vec3& v) { return sqrtf(dotProduct(v, v)); }
	inline float magnitude(const vec4& v) { return sqrtf(dotProduct(v, v)); }

	// linear transitions over time, point a to point b
	inline vec3 lerp(const vec3& v_i, const vec3& v_f, float k) { return (1.f - k) * v_i + (k * v_f); };
	inline vec4 lerp(const vec4& v_i, const vec4& v_f, float k) { return (1.f - k) * v_i + (k * v_f); };


	// rotations, less computationally expensive, inaccurate
	// only use for small angle rotations
	// https://keithmaggio.wordpress.com/2011/02/15/math-magician-lerp-slerp-and-nlerp/
	inline vec3 nlerp(const vec3& v_i, const vec3& v_f, float k) { return normalize(lerp(v_i, v_f, k)); };
	inline vec4 nlerp(const vec4& v_i, const vec4& v_f, float k) { return normalize(lerp(v_i, v_f, k)); };

	// rotations, more computationally expensive
	vec3 slerp(const vec3& v_i, const vec3& v_f, float k);
	vec4 slerp(const vec4& v_i, const vec4 & v_f, float k);

	vec3 RotateVector(const vec3& v, const vec3& rotAxis, float angle);

#pragma endregion
}