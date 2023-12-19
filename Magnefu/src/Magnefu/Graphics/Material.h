#pragma once

#include "RenderConstants.h"

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
		Maths::vec3 Translation[MAX_AREA_LIGHTS];
		Maths::vec3 Rotation[MAX_AREA_LIGHTS];
		Maths::vec3 Scale[MAX_AREA_LIGHTS];
		float       AngleOfRot[MAX_AREA_LIGHTS];
	};
}