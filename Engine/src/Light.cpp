#include "Light.h"


PointLight CreatePointLight()
{
	Maths::vec3 position = { -0.27f, 0.67f, 1.56f };
	Maths::vec3 diffuse = { 1.f, 1.f, 1.f };
	Maths::vec3 specular = { 1.f, 0.f, 1.f };
	float constant = 1.f;
	float linear = 0.09f;
	float quadratic = 0.032f;

	return {
		diffuse,
		specular,
		constant,
		linear,
		quadratic,
		position
	};
}

DirectionLight CreateDirLight()
{
	Maths::vec3 direction = { -0.2f, -1.f, -0.3f };
	Maths::vec3 diffuse = { 0.8f, 0.6f, 0.2f };
	Maths::vec3 specular = { 0.8f, 0.8f, 0.9f };
	float constant = 1.f;
	float linear = 0.09f;
	float quadratic = 0.032f;

	return {
		diffuse,
		specular,
		constant,
		linear,
		quadratic,
		direction
	};
}

SpotLight CreateSpotLight()
{
	Maths::vec3 position = { 0.5f, 0.5f, 1.25f };
	Maths::vec3 direction = { 0.f, 0.f, -1.f };
	Maths::vec3 diffuse = { 0.25f, 1.f, 1.f };
	Maths::vec3 specular = { 1.f, 0.f, 1.f };
	float constant = 1.f;
	float linear = 0.09f;
	float quadratic = 0.032;
	float innerCutoff = Maths::cos(Maths::toRadians(15.f));
	float outerCutoff = Maths::cos(Maths::toRadians(25.f));


	return {
		diffuse,
		specular,
		constant,
		linear,
		quadratic,
		direction,
		position,
		innerCutoff,
		outerCutoff
	};
}