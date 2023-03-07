#pragma once

#include "Vectors.h"
#include "Texture.h"
#include <string>
#include <memory>


template <typename T>
struct Material
{
	bool		 Preset;
	bool		 Textured;
	unsigned int ID;
	T            Ambient;
	T            Diffuse;
	T            Specular;
	Maths::vec3  Ka;
	Maths::vec3  Kd;
	Maths::vec3  Ks;
	Maths::vec3  Ke;
	float        Ni;       // Index of refraction
	float        Ns;
	float        Opacity;  // d OR 1 - Tr
	Maths::vec3  Tf;       // Transmission filter color
	int          Illum;
	// AO
	//float       Roughness;
	//float		  Displacement
	
	//float       Metallic	
};


struct MaterialData
{
	std::string MaterialLibrary;
	std::string SubMaterialName;
	Material<std::shared_ptr<Texture>> MaterialProperties;
};