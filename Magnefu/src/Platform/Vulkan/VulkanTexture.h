#pragma once

#include "VulkanCommon.h"
#include "Magnefu/Renderer/Texture.h"

namespace Magnefu
{
	class VulkanTexture : public Texture
	{
	public:
		VulkanTexture(TextureDesc& desc);
		~VulkanTexture();

	private:
		void CreateTextureImage(TextureDesc& desc);
		void CreateTextureImageView(TextureDesc& desc);
		void CreateTextureSampler();

		// Image Manipulation
		void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageType imageType, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);


		

	public:
		TextureType      m_Type;
		uint32_t         m_MipLevels;
		VkImage          m_Image;
		VkImageView      m_ImageView;
		VkDeviceMemory   m_BufferMemory;
		Ref<VkSampler>   m_TextureSampler;
	};
}
