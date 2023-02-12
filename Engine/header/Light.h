#pragma once

#include "Vectors.h"
#include "Matrices.h"

struct Light
{
	Maths::vec3 Diffuse;
	Maths::vec3 Specular;
	float constant;
	float linear;
	float quadratic;
};

struct DirectionLight : public Light
{
	Maths::vec3 Direction;
};

struct PointLight : public Light
{
	Maths::vec3 Position;
};

struct SpotLight : public DirectionLight, PointLight
{
	float cutoff;
};

struct PointLightModel : public PointLight
{
	Maths::mat4 MVP;
};

PointLight CreatePointLight();

