#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"


namespace Magnefu
{
	enum TextureType
	{
		NONE = -1,
		AMBIENT,
		DIFFUSE,
		SPECULAR,
		EMISSIVE,
		BUMP,
		ROUGHNESS,
		METALLIC
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


		static Ref<Texture> Create(const std::string& filepath = "res/textures/pluto.png");
		
	};

}
