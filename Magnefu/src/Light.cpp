#include "mfpch.h"

#include "Light.h"


PointLight CreatePointLight()
{
	Maths::vec3 position = { -0.27f, 0.22f, 0.f };
	Maths::vec3 color = { 1.f, 1.f, 1.f };
	float constant = 1.f;
	float linear = 0.09f;
	float quadratic = 0.032f;

	return {
		true,
		color,
		constant,
		linear,
		quadratic,
		position
	};
}

DirectionLight CreateDirLight()
{
	Maths::vec3 direction = { -0.2f, -1.f, -0.3f };
	Maths::vec3 color = { 1.f, 1.f, 1.f };
	float constant = 1.f;
	float linear = 0.09f;
	float quadratic = 0.032f;

	return {
		true,
		color,
		constant,
		linear,
		quadratic,
		direction
	};
}

SpotLight CreateSpotLight()
{
	Maths::vec3 position = { 0.951f, 0.35f, 0.951f };
	Maths::vec3 direction = { 0.f, 0.f, -1.f };
	Maths::vec3 color = { 0.f, 0.82f, 0.26f };
	float constant = 1.f;
	float linear = 0.09f;
	float quadratic = 0.032f;
	float innerCutoff = Maths::cos(Maths::toRadians(15.f));
	float outerCutoff = Maths::cos(Maths::toRadians(25.f));


	return {
		true,
		color,
		constant,
		linear,
		quadratic,
		direction,
		position,
		innerCutoff,
		outerCutoff
	};
}