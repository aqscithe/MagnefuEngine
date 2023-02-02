#pragma once

#include <string>
#include <iostream>


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

		union vec3
		{
			vec3() = default;
			vec3(float x, float y, float z)
				: x(x), y(y), z(z)
			{}
			vec3(vec2 xy, float z)
				: x(xy.x), y(xy.y), z(z)
			{}

			float e[3];
			struct { float x, y, z; };
			struct { float r, g, b; };

			vec2 xy;
		};

		union vec4
		{
			vec4() = default;
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

		union mat4
		{
			float e[16];
			vec4 c[4];
		};
		#pragma endregion

#pragma region vec3 
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

#pragma endregion

#pragma region vec4

		inline vec4 operator*(const mat4& m, const vec4& v)
		{
			return {
				v.x * m.c[0].e[0] + v.y * m.c[1].e[0] + v.z * m.c[2].e[0] + v.w * m.c[3].e[0],
				v.x * m.c[0].e[1] + v.y * m.c[1].e[1] + v.z * m.c[2].e[1] + v.w * m.c[3].e[1],
				v.x * m.c[0].e[2] + v.y * m.c[1].e[2] + v.z * m.c[2].e[2] + v.w * m.c[3].e[2],
				v.x * m.c[0].e[3] + v.y * m.c[1].e[3] + v.z * m.c[2].e[3] + v.w * m.c[3].e[3]
			};
		}

		inline vec4 operator*=(const mat4& m, vec4& v)
		{
			v = m * v;
			return v;
		}
#pragma endregion   

#pragma region mat4

		mat4 identity();
		mat4 axis(const vec3& right, const vec3& up, const vec3& forward);
		mat4 translate(const vec3& t);
		mat4 rotateX(float& pitch);
		mat4 rotateY(float& yaw);
		mat4 rotateZ(float& roll);
		mat4 rotate(const float angleInDegrees, const vec3& rotAxis);
		mat4 scale(const vec3& s);
		mat4 orthographic(float left, float right, float bottom, float top, float near, float far);
		mat4 perspective(float fovY, float aspect, float near, float far);
		mat4 frustum(float left, float right, float bottom, float top, float near, float far);

		vec3 normalize(const vec3& v);
		vec4 normalize(const vec4& v);
		vec3 crossProduct(const vec3& v, const vec3& k);
		bool invert(const float in[16], float out[16]);

		inline mat4 operator*(const mat4& a, const mat4& b)
		{
			mat4 res = {};
			for (int c = 0; c < 4; ++c)
				for (int r = 0; r < 4; ++r)
					for (int i = 0; i < 4; ++i)
						res.c[c].e[r] += a.c[i].e[r] * b.c[c].e[i];
			return res;
		}

		inline mat4& operator*=(mat4& a, mat4& b)
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

		inline std::ostream& operator<<(std::ostream& os, const mat4& m)
		{
			os << "[" << std::endl << m.c[0] << std::endl << m.c[1] << std::endl << m.c[2] << std::endl << m.c[3] << std::endl << "]";
			return os;
		}

		
#pragma endregion

#pragma region Quaternions

		union quat
		{

		};

#pragma endregion

#pragma region Constants & common math functions

		const float TAU = 6.283185307179586476925f;

		template<typename T>
		inline int min(T x, T y) { return x < y ? x : y; };
		template<typename T>
		inline int max(T x, T y) { return x > y ? x : y; };

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

		inline float abs(float x) { return x > 0.f ? x : x * -1.f; }
		inline int abs(int x) { return x > 0 ? x : x * -1; }
		

		inline float cos(float x) { return cosf(x); }
		inline float sin(float x) { return sinf(x); }
		inline float tan(float x) { return tanf(x); }

		inline float toRadians(float degrees) { return degrees * TAU / 360.f; };


		inline float dotProduct(const vec3& a, const vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
		inline float dotProduct(const vec4& a, const vec4& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
		inline float magnitude(const vec3& v) { return sqrtf(dotProduct(v, v)); }
		inline float magnitude(const vec4& v) { return sqrtf(dotProduct(v, v)); }

		inline float max(float a, float b) { return a > b ? a : b; }
		inline float min(float a, float b) { return a < b ? a : b; }
		inline float clamp(float min, float max, float val) { return val < min ? min : val > max ? max : val; }

#pragma endregion
	}
