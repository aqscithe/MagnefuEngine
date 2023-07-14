#pragma once

#include "VulkanCommon.h"
#include "Magnefu/Renderer/Texture.h"

namespace Magnefu
{
	class VulkanTexture : public Texture
	{
	public:
		VulkanTexture(const TextureDesc& desc);
		~VulkanTexture();

		inline const VkImageView& GetImageView() const { return m_ImageView; }

		static inline VkSampler& GetSampler() { return *s_TextureSampler.get(); }

	private:
		void CreateTextureImage(const TextureDesc& desc);
		void CreateTextureImageView(const TextureDesc& desc);
		void CreateTextureSampler();

		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);


		

	public:
		TextureType        m_Type;
		uint32_t           m_MipLevels;
		VkImage            m_Image;
		VkImageView        m_ImageView;
		VkDeviceMemory     m_BufferMemory;
		static Ref<VkSampler>  s_TextureSampler;
		static bool s_SamplerCreated;
	};
}
