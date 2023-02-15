#pragma once

#include "Vectors.h"


struct Material
{
	bool		 Preset;
	bool		 Textured;
	unsigned int TexID;
	Maths::vec3  Ambient;
	Maths::vec3  Diffuse;
	Maths::vec3  Specular;
	float		 K_d;
	float		 K_s;
	float        Shininess;
	// some of this info may be passed - instead - via vertices
	// into vertex shader
	// AO
	//float       Roughness;
	//float		  Displacement
	//float       Opacity;
	//float       Metallic
	
};