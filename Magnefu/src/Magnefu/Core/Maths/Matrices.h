#pragma once

#include "Vectors.h"


namespace Maths
{
#pragma region Union Declarations

	union mat4;

	union mat3
	{
		mat3(const mat4& m);

		static std::type_index getTypeIndex() {
			return std::type_index(typeid(mat3));
		}

		float e[9];
		vec3 c[3];
	};

	union mat4
	{
		mat4() = default;

		mat4(float d);
		mat4(
			float x0, float x1, float x2, float x3,
			float y0, float y1, float y2, float y3,
			float z0, float z1, float z2, float z3,
			float w0, float w1, float w2, float w3
		);
		mat4(vec4 v0, vec4 v1, vec4 v2, vec4 v3);
		mat4(const mat3&);

		static std::type_index getTypeIndex() {
			return std::type_index(typeid(mat4));
		}

		float e[16];
		vec4 c[4];
	};

#pragma endregion

#pragma region Operator Overloads
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

	inline mat4 operator*(const mat4& a, float f)
	{
		mat4 res = {};
		for (int c = 0; c < 4; ++c)
			for (int r = 0; r < 4; ++r)
				res.c[c].e[r] += a.c[c].e[r] * f;
		return res;
	}

	inline mat4 operator+(const mat4& a, const mat4& b)
	{
		mat4 res = {};
		for (int c = 0; c < 4; ++c)
			for (int r = 0; r < 4; ++r)
				res.c[c].e[r] = a.c[c].e[r] + b.c[c].e[r];
		return res;
	}

	inline std::ostream& operator<<(std::ostream& os, const mat4& m)
	{
		os << "[" << std::endl << m.c[0] << std::endl << m.c[1] << std::endl << m.c[2] << std::endl << m.c[3] << std::endl << "]";
		return os;
	}
#pragma endregion

#pragma region Matrix Functions
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
	bool invert(const float in[16], float out[16]);
	mat4 transpose(const mat4&);
#pragma endregion
}
