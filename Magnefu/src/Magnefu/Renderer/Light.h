#pragma once

#include "Magnefu/Core/Maths/Vectors.h"
#include "Magnefu/Core/Maths/Matrices.h"

namespace Magnefu
{
	enum class ReflectionModel
	{
		PHONG,
		MODIFIED_PHONG,
		BLINN_PHONG,
		MICRO_FACET,
	};

	struct DirectionalLight
	{
		Maths::vec3 Direction;
		Maths::vec3 Color;
		float Flux;
		bool Enabled;
	};
	
}


// TODO: add RadiantFlux value
//struct Light
//{
//	bool Enabled;
//	Maths::vec3 Color;
//	float constant;
//	float linear;
//	float quadratic;
//};

//struct DirectionLight : public Light
//{
//	Maths::vec3 Direction;
//};
//
//struct PointLight : public Light
//{
//	Maths::vec3 Position;
//};
//
//struct SpotLight : public Light
//{
//	Maths::vec3 Direction;
//	Maths::vec3 Position;
//	float innerCutoff;
//	float outerCutoff;
//};
//
//struct PointLightModel : public PointLight
//{
//	Maths::mat4 MVP;
//};
//
//struct DirLightModel : public DirectionLight
//{
//	Maths::mat4 MVP;
//};
//
//struct SpotLightModel : public SpotLight
//{
//	Maths::mat4 MVP;
//};
//
//
//PointLight CreatePointLight();
//DirectionLight CreateDirLight();
//SpotLight CreateSpotLight();

