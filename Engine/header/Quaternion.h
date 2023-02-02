#pragma once
#include "Core.h"


class Quaternion
{
private:
	Maths::vec4 m_Quat;
	Maths::vec4 m_UnitQuat;
	Maths::mat4 m_RotationMatrix;

public:
	Quaternion(float rotationAngle, const Maths::vec3& rotationAxis);
	void SetQuaternion(const Maths::vec3& rotationAxis, float rotationAngle);
	~Quaternion();

	Maths::vec4 MultiplyQuat(const Maths::vec4& q1, const Maths::vec4& q2);

	static Maths::vec4 CreateQuatConjugate(const Maths::vec4& q1);

	Maths::vec4 quat_from_axis_angle(Maths::vec3 axis, float angle);
	
	Maths::mat4& UpdateRotMatrix(float rotationAngle, const Maths::vec3& rotationAxis);

	inline const Maths::mat4& GetRotMatrix() const { return m_RotationMatrix; }
	inline const Maths::vec4 GetInverseQuat() const { return { -m_UnitQuat.x, -m_UnitQuat.y, -m_UnitQuat.z, m_UnitQuat.w }; }

private:
	void CreateQuatRotMatrix();
};