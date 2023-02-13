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

struct SpotLight : public Light
{
	Maths::vec3 Direction;
	Maths::vec3 Position;
	float innerCutoff;
	float outerCutoff;
};

struct PointLightModel : public PointLight
{
	Maths::mat4 MVP;
};

struct DirLightModel : public DirectionLight
{
	Maths::mat4 MVP;
};

struct SpotLightModel : public SpotLight
{
	Maths::mat4 MVP;
};

PointLight CreatePointLight();
DirectionLight CreateDirLight();
SpotLight CreateSpotLight();

