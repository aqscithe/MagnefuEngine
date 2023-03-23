#pragma once

#include "CacheableResource.h"
#include <string>

namespace Magnefu
{
	using String = std::string;

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

	struct TextureLabel
	{
		std::string Name;
		TextureType Type;
	};

	class Texture : public CacheableResource
	{
	private:
		unsigned int m_RendererID;
		String m_filepath;
		unsigned char* m_texData;
		int m_width, m_height, m_BPP;

		bool m_HasGenerated;
		bool m_Bound;
		uint32_t m_Slot;

	public:
		Texture() = default;
		Texture(const String& filepath, bool async = false);
		~Texture();

		void Bind(unsigned int slot = 0);
		void Unbind();

		unsigned int GetRendererID() const { return m_RendererID; };
		bool GetGenerationStatus() const { return m_HasGenerated; };
		String GetFilepath() const { return m_filepath; };
		uint32_t GetSlot() const { return m_Slot; }

		void GenerateTexture();

	private:
		void SetTextureOptions();
		void LoadTexture();
		void GenerateTexImage();


	};
}

