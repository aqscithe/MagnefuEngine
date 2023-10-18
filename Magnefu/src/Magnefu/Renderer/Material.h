#pragma once


namespace Magnefu
{
	struct Material
	{
		Maths::vec3 Translation = Maths::vec3(0.0f);
		Maths::vec3 Rotation = Maths::vec3(0.0f);;
		Maths::vec3 Scale = Maths::vec3(1.0f);
		Maths::vec3 Tint = Maths::vec3(1.0f);
		float       AngleOfRot = 0.f;
		float       Opacity = 1.f;
		float       Reflectance = 0.1f;
	};

	struct MaterialInstanced
	{
		Maths::vec3 Translation[2] = { Maths::vec3(0.0), Maths::vec3(0.0) };
		Maths::vec3 Rotation[2] = { Maths::vec3(0.0), Maths::vec3(0.0) };
		Maths::vec3 Scale[2] = { Maths::vec3(1.0), Maths::vec3(1.0) };
		Maths::vec4 Tint[2] = { Maths::vec4(1.0), Maths::vec4(1.0) };
		float       AngleOfRot[2] = { 0.f, 0.f };
		float       Opacity[2] = { 1.f, 1.f };
		float       Reflectance[2] = { 0.1f, 0.1f };
	};
}