#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"


namespace Magnefu
{
	enum TextureType
	{
		NONE = -1,
		DIFFUSE,
		METAL,
		ROUGHNESS,
		NORMAL,
		AO,
		EMISSIVE,
		DISPLACEMENT
	};

	enum TextureOptions {
		TextureOptions_None = 0,
		TextureOptions_Skybox = BIT(0),
		TextureOptions_All = 1
	};

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual std::string GetFilepath() const = 0;
		virtual int GetHeight() const = 0;
		virtual int GetWidth() const = 0;
		virtual uint32_t GetSlot() const = 0;

		virtual void OnImGuiRender() const = 0;


		static Ref<Texture> Create(const TextureOptions& options = TextureOptions_None, const std::string& filepath = "res/textures/darkmarble/dark-marbled-stone_d.jpg");
		
	};

}
