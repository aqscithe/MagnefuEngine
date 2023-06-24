#include "mfpch.h"

#include "Quaternion.h"

namespace Maths
{
	//vec3 Quaternion::RotateVertexPos()
	//{
	//	vec4 q = quat_from_axis_angle(axis, angle);
	//	vec3 v = position.xyz;
	//	return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
	//}

	vec4 Quaternion::CalculateQuaternion(float angle, vec3 axis)
	{
		vec4 quat(0.f);
		float halfAngleInRadians = toRadians(angle / 2.f);
		
		quat.x = axis.x * sin(halfAngleInRadians);
		quat.y = axis.y * sin(halfAngleInRadians);
		quat.z = axis.z * sin(halfAngleInRadians);
		quat.w = cos(halfAngleInRadians);

		return quat;
	}

	mat4 Quaternion::CalculateRotationMatrix(float angle, const vec3& axis)
	{
		vec4 unitQ = normalize(CalculateQuaternion(angle, axis));
		return ToRotationMatrix(unitQ);
	}

	mat4 Quaternion::ToRotationMatrix(const vec4& unitQuat, bool IsPointRotation)
	{
		if(IsPointRotation)
			return {
				2.f * (pow(unitQuat.w, 2) + pow(unitQuat.x, 2)) - 1.f,   2.f * (unitQuat.x * unitQuat.y + unitQuat.w * unitQuat.z),               2.f * (unitQuat.x * unitQuat.z - unitQuat.w * unitQuat.y),              0.f,
				2.f * (unitQuat.x * unitQuat.y - unitQuat.w * unitQuat.z),               2.f * (pow(unitQuat.w, 2) + pow(unitQuat.y, 2)) - 1.f,   2.f * (unitQuat.y * unitQuat.z + unitQuat.w * unitQuat.x),              0.f,
				2.f * (unitQuat.x * unitQuat.z + unitQuat.w * unitQuat.y),               2.f * (unitQuat.y * unitQuat.z - unitQuat.w * unitQuat.x),               2.f * (pow(unitQuat.w, 2) + pow(unitQuat.z, 2)) - 1.f,  0.f,
				0.f,                                                                 0.f,																  0.f,																  1.f
			};

		// frame rotation
		return {
			2.f * (pow(unitQuat.w, 2) + pow(unitQuat.x, 2)) - 1.f,   2.f * (unitQuat.x * unitQuat.y - unitQuat.w * unitQuat.z),               2.f * (unitQuat.x * unitQuat.z + unitQuat.w * unitQuat.y),              0.f,
			2.f * (unitQuat.x * unitQuat.y + unitQuat.w * unitQuat.z),               2.f * (pow(unitQuat.w, 2) + pow(unitQuat.y, 2)) - 1.f,   2.f * (unitQuat.y * unitQuat.z - unitQuat.w * unitQuat.x),              0.f,
			2.f * (unitQuat.x * unitQuat.z - unitQuat.w * unitQuat.y),               2.f * (unitQuat.y * unitQuat.z + unitQuat.w * unitQuat.x),               2.f * (pow(unitQuat.w, 2) + pow(unitQuat.z, 2)) - 1.f,  0.f,
			0.f,                                                                 0.f,																  0.f,																  1.f
		};
	}

	vec4 Quaternion::Invert(vec4& q)
	{
		return CalculateConjugate(q) / magnitude(q);
	}

	vec4 Quaternion::CalculateConjugate(const vec4& q)
	{
		return { -q.x, -q.y, -q.z, q.w };
	}

	vec4 Quaternion::Multiply(const vec4& q1, const vec4& q2)
	{
		vec4 qr;
		qr.x = (q1.w * q2.x) + (q1.x * q2.w) + (q1.y * q2.z) - (q1.z * q2.y);
		qr.y = (q1.w * q2.y) - (q1.x * q2.z) + (q1.y * q2.w) + (q1.z * q2.x);
		qr.z = (q1.w * q2.z) + (q1.x * q2.y) - (q1.y * q2.x) + (q1.z * q2.w);
		qr.w = (q1.w * q2.w) - (q1.x * q2.x) - (q1.y * q2.y) - (q1.z * q2.z);
		return qr;
	}
}

