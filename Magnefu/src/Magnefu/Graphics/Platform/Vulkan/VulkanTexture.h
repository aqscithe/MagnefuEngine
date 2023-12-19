#pragma once

#include "VulkanCommon.h"
#include "Magnefu/Renderer/Texture.h"

namespace Magnefu
{
	struct SamplerInfo
	{
		VkSampler Sampler;
		uint32_t  UseCount;
	};

	class VulkanTexture : public Texture
	{
	public:
		VulkanTexture(const TextureDesc& desc);
		~VulkanTexture();

		inline const VkImageView& GetImageView() const { return m_ImageView; }

		inline const VkSampler& GetSampler() const { return m_TextureSampler; }

	private:
		void CreateTextureImage(const TextureDesc& desc);
		void CreateTextureImageView(const TextureDesc& desc);
		void CreateTextureSampler(const TextureDesc& desc);

		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);


	private:
		TextureType        m_Type;
		uint32_t           m_MipLevels;
		VkImage            m_Image;
		VkImageView        m_ImageView;
		VkDeviceMemory     m_BufferMemory;
		VkSampler          m_TextureSampler;

		VmaAllocation     m_Allocation;
		VmaAllocationInfo m_AllocInfo;

		static SamplerInfo s_ImageTexSamplerInfo;
		static SamplerInfo s_DataTexSamplerInfo;
	};
}
