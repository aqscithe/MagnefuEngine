#pragma once

// NOTE: A material is a shader and a bunch of uniforms.

#include "Texture.h"
#include "CacheableResource.h"

#include "MathsCommon.h"
#include <string>


namespace Magnefu
{
	struct TextureProps
	{
		TextureProps();

		Texture* Ambient;
		Texture* Diffuse;
		Texture* Specular;
		Texture* Roughness;
		Texture* Metallic;
		Texture* Emissive;
	};

	class Material : public CacheableResource
	{
	public:
        Material() = default;
		Material(const TextureProps&, const std::string&);
        Material(const std::string& matFile, const std::string& matData, const std::string& matName);

		void Bind();
		void Unbind();
		void NullifyTextures();
		

	public:
		std::string Name;
		std::string Library;
		uint32_t      ID;
		Texture*      Ambient;
		Texture*      Diffuse;
		Texture*      Specular;
		Texture*	  Roughness;
		Texture*	  Metallic;
		Texture*      Emissive;
		Maths::vec3   Ka;
		Maths::vec3   Kd;
		Maths::vec3   Ks;
		Maths::vec3   Ke;
		float         Ni;       // Index of refraction
		float         Ns;
		float         Opacity;  // d OR 1 - Tr
		Maths::vec3   Tf;       // Transmission filter color
		int           Illum;
	};

	struct MaterialData
	{
		String Library;
		String Name;
		Material Props;
	};
}
