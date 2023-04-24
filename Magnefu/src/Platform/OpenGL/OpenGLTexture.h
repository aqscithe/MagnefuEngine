#pragma once

#include "Magnefu/Renderer/Texture.h"
#include "Magnefu/ResourceManagement/CacheableResource.h"

namespace Magnefu
{
	struct TextureLabel
	{
		std::string Name;
		TextureType Type;
	};

	class OpenGLTexture : public Texture, public CacheableResource
	{
	private:
		std::string m_Filepath;
		unsigned char* m_texData;
		unsigned int m_RendererID;
		uint32_t m_Slot;
		TextureOptions m_Options;
		int m_Width, m_Height, m_BPP;
		bool m_Bound;
		

	public:
		OpenGLTexture(const TextureOptions& options, const std::string& filepath);
		~OpenGLTexture();

		void Bind() override;
		void Unbind() override;

		std::string GetFilepath() const override { return m_Filepath; }
		int GetHeight() const override { return m_Height; }
		int GetWidth() const override { return m_Width; }

		unsigned int GetRendererID() const { return m_RendererID; };
		//bool GetGenerationStatus() const { return m_HasGenerated; };
		uint32_t GetSlot() const override { return m_Slot; }

		void GenerateTexture();

		void OnImGuiRender() const override;

	private:
		void CreateTexture();
		void CreateCubeMapTexture();

		void SetTextureOptions();
		void LoadTexture();
		void GenerateTexImage();


	};
}

