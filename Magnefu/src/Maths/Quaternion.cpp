#include "mfpch.h"

#include "Quaternion.h"

namespace Maths
{
	Quaternion::Quaternion(float rotationAngle, const vec3& rotationAxis)
	{
		SetQuaternion(rotationAxis, rotationAngle);

		m_UnitQuat = normalize(m_Quat);

		CreateQuatRotMatrix();
	}




	void Quaternion::SetQuaternion(const vec3& rotationAxis, float rotationAngle)
	{
		// i see a slightly different formula here:
		// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

		float halfAngleInRadians = toRadians(rotationAngle / 2.f);

		m_Quat.x = rotationAxis.x * sin(halfAngleInRadians);
		m_Quat.y = rotationAxis.y * sin(halfAngleInRadians);
		m_Quat.z = rotationAxis.z * sin(halfAngleInRadians);
		m_Quat.w = cos(halfAngleInRadians);
	}

	Quaternion::~Quaternion()
	{

	}

	vec4 Quaternion::MultiplyQuat(const vec4& q1, const vec4& q2)
	{
		vec4 qr;
		qr.x = (q1.w * q2.x) + (q1.x * q2.w) + (q1.y * q2.z) - (q1.z * q2.y);
		qr.y = (q1.w * q2.y) - (q1.x * q2.z) + (q1.y * q2.w) + (q1.z * q2.x);
		qr.z = (q1.w * q2.z) + (q1.x * q2.y) - (q1.y * q2.x) + (q1.z * q2.w);
		qr.w = (q1.w * q2.w) - (q1.x * q2.x) - (q1.y * q2.y) - (q1.z * q2.z);
		return qr;
	}

	vec4 Quaternion::CreateQuatConjugate(const vec4& q)
	{
		return { -q.x, -q.y, -q.z, q.w };
	}

	

	//vec3 Quaternion::RotateVertexPos()
	//{
	//	vec4 q = quat_from_axis_angle(axis, angle);
	//	vec3 v = position.xyz;
	//	return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
	//}

	vec4 Quaternion::quat_from_axis_angle(vec3 axis, float angle)
	{
		vec4 qr;
		float half_angle = (angle * 0.5) * 3.14159 / 180.0;
		qr.x = axis.x * sin(half_angle);
		qr.y = axis.y * sin(half_angle);
		qr.z = axis.z * sin(half_angle);
		qr.w = cos(half_angle);
		return qr;
	}

	mat4& Quaternion::UpdateRotMatrix(float rotationAngle, const vec3& rotationAxis)
	{
		SetQuaternion(rotationAxis, rotationAngle);

		m_UnitQuat = normalize(m_Quat);

		CreateQuatRotMatrix();

		return m_RotationMatrix;
	}

	//https://automaticaddison.com/how-to-convert-a-quaternion-to-a-rotation-matrix/
	void Quaternion::CreateQuatRotMatrix()
	{
		// frame rotation
		//m_RotationMatrix = {
		//	2.f * (pow(m_UnitQuat.w, 2) + pow(m_UnitQuat.x, 2)) - 1.f,   2.f * (m_UnitQuat.x * m_UnitQuat.y - m_UnitQuat.w * m_UnitQuat.z),               2.f * (m_UnitQuat.x * m_UnitQuat.z + m_UnitQuat.w * m_UnitQuat.y),              0.f,
		//	2.f * (m_UnitQuat.x * m_UnitQuat.y + m_UnitQuat.w * m_UnitQuat.z),               2.f * (pow(m_UnitQuat.w, 2) + pow(m_UnitQuat.y, 2)) - 1.f,   2.f * (m_UnitQuat.y * m_UnitQuat.z - m_UnitQuat.w * m_UnitQuat.x),              0.f,
		//	2.f * (m_UnitQuat.x * m_UnitQuat.z - m_UnitQuat.w * m_UnitQuat.y),               2.f * (m_UnitQuat.y * m_UnitQuat.z + m_UnitQuat.w * m_UnitQuat.x),               2.f * (pow(m_UnitQuat.w, 2) + pow(m_UnitQuat.z, 2)) - 1.f,  0.f,
		//	0.f,                                                                 0.f,																  0.f,																  1.f
		//};

		// point rotation (transpose of frame)
		m_RotationMatrix = {
			2.f * (pow(m_UnitQuat.w, 2) + pow(m_UnitQuat.x, 2)) - 1.f,   2.f * (m_UnitQuat.x * m_UnitQuat.y + m_UnitQuat.w * m_UnitQuat.z),               2.f * (m_UnitQuat.x * m_UnitQuat.z - m_UnitQuat.w * m_UnitQuat.y),              0.f,
			2.f * (m_UnitQuat.x * m_UnitQuat.y - m_UnitQuat.w * m_UnitQuat.z),               2.f * (pow(m_UnitQuat.w, 2) + pow(m_UnitQuat.y, 2)) - 1.f,   2.f * (m_UnitQuat.y * m_UnitQuat.z + m_UnitQuat.w * m_UnitQuat.x),              0.f,
			2.f * (m_UnitQuat.x * m_UnitQuat.z + m_UnitQuat.w * m_UnitQuat.y),               2.f * (m_UnitQuat.y * m_UnitQuat.z - m_UnitQuat.w * m_UnitQuat.x),               2.f * (pow(m_UnitQuat.w, 2) + pow(m_UnitQuat.z, 2)) - 1.f,  0.f,
			0.f,                                                                 0.f,																  0.f,																  1.f
		};
	}
}

