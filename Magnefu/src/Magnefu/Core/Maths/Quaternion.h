#pragma once

// -- Core Includes ---------------------------------- //
#include "Vectors.h"
#include "Matrices.h"

// -- other Includes -------------- //




//TODO: Implement lerp functions for quaternions

namespace Maths
{
	class Quaternion
	{
	public:
		static vec4 Invert(vec4& q);
		static vec4 CalculateConjugate(const vec4& q);
		static vec4 CalculateQuaternion(float angle, vec3 axis);
		static mat4 CalculateRotationMatrix(float angle, const vec3& axis);
		static mat4 ToRotationMatrix(const vec4&, bool IsPointRotation = true);
		//static mat4 FromRotationMatrix(const vec4&);
		static vec4 CreateQuatConjugate(const vec4& q1);
		static vec4 Multiply(const vec4& q1, const vec4& q2);
		//static vec3 RotateVector(const vec3& v, const vec3& rotAxis, float angle);
	};
}

