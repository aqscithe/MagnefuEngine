#pragma once


namespace Magnefu
{
	struct Material
	{
		Maths::vec3 Tint = Maths::vec3(1.0f);
		float       Opacity = 1.f;
		float       Reflectance = 0.1f;
	};
}