#include "Light.h"


PointLight CreatePointLight()
{
	Maths::vec3 position = { -0.27f, 0.67f, 1.56f };
	Maths::vec3 diffuse = { 1.f, 1.f, 1.f };
	Maths::vec3 specular = { 1.f, 0.f, 1.f };
	float constant = 1.f;
	float linear = 0.7f;
	float quadratic = 1.8f;

	return {
		diffuse,
		specular,
		constant,
		linear,
		quadratic,
		position
	};
}