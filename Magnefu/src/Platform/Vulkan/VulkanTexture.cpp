#include "mfpch.h"
#include "VulkanTexture.h"
#include "VulkanContext.h"
#include "Magnefu/Application.h"
#include "Magnefu/ResourceManagement/ResourcePaths.h"
#include "Magnefu/ResourceManagement/ResourceManager.h"
#include "Magnefu/Renderer/LTCMatrix.h"



namespace Magnefu
{

	SamplerInfo VulkanTexture::s_ImageTexSamplerInfo = { VK_NULL_HANDLE, 0 };
	SamplerInfo VulkanTexture::s_DataTexSamplerInfo = { VK_NULL_HANDLE, 0 };
	

	VulkanTexture::VulkanTexture(const TextureDesc& desc) : Texture(desc)
	{
		switch (desc.Type)
		{
			case TextureType::NONE:
			{
				MF_CORE_ASSERT(false, "Invalid Texture Type");
				return;
			}

			default:
				break;
		}

		m_Type = desc.Type;

		CreateTextureImage(desc);
		CreateTextureImageView(desc);
		CreateTextureSampler(desc);

	}

	VulkanTexture::~VulkanTexture()
	{
		VkDevice device = VulkanContext::Get().GetDevice();

		switch (m_Type)
		{
			case Magnefu::DIFFUSE:
			case Magnefu::ARM:
			case Magnefu::NORMAL:
			{
				if (s_ImageTexSamplerInfo.Sampler != VK_NULL_HANDLE)
				{
					if (s_ImageTexSamplerInfo.UseCount <= 1)
					{
						vkDestroySampler(device, s_ImageTexSamplerInfo.Sampler, nullptr);
						s_ImageTexSamplerInfo.UseCount = 0;
					}
					else
					{
						s_ImageTexSamplerInfo.UseCount--;
					}
				}
			
				break;
			}

			case Magnefu::LTC1:
			case Magnefu::LTC2:
			{
				if (s_DataTexSamplerInfo.Sampler != VK_NULL_HANDLE)
				{
					if (s_DataTexSamplerInfo.UseCount <= 1)
					{
						vkDestroySampler(device, s_DataTexSamplerInfo.Sampler, nullptr);
						s_DataTexSamplerInfo.UseCount = 0;
					}
					else
					{
						s_DataTexSamplerInfo.UseCount--;
					}
				}
				break;
			}
			default:
			{
				break;
			}

		}

		vkDestroyImageView(device, m_ImageView, nullptr);
		vkDestroyImage(device, m_Image, nullptr);
		vkFreeMemory(device, m_BufferMemory, nullptr);
		

	}

	void VulkanTexture::CreateTextureImage(const TextureDesc& desc)
	{

		// EXPLORE HOW THIS FUNCTION CHANGES FOR THE LTC DATA.

		if (desc.Type == TextureType::LTC1 || desc.Type == TextureType::LTC2)
		{

			//m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(128, 128)))) + 1;
			m_MipLevels = 1;  
			MF_CORE_DEBUG("Mip Levels: {}", m_MipLevels);  

			VkDeviceSize imageSize = 128 * 128 * TextureChannels::CHANNELS_RGB_ALPHA;  // Image Size with this texture is still a question marke for me...

			VkBuffer stagingBuffer;
			VmaAllocation stagingAllocation;
			VmaAllocationInfo stagingAllocInfo;

			VmaAllocator allocator = VulkanContext::Get().GetVmaAllocator();


			VulkanCommon::CreateBuffer(
				imageSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				stagingBuffer,
				VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
				VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
				stagingAllocation,
				stagingAllocInfo
			);

			void* data;
			vmaMapMemory(allocator, stagingAllocation, &data);
			memcpy(data, desc.Type == TextureType::LTC1 ? LTC1_Matrix : LTC2_Matrix, static_cast<size_t>(imageSize));
			vmaUnmapMemory(allocator, stagingAllocation);

			VulkanCommon::CreateImage(
				128,  // HOW WILL WIDTH AND HEIGHT BE DETERMINED WITH LTC
				128,
				m_MipLevels,
				VK_SAMPLE_COUNT_1_BIT,
				static_cast<VkFormat>(desc.Format),
				VK_IMAGE_TYPE_2D,
				static_cast<VkImageTiling>(desc.Tiling),
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				m_Image,
				VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
				0,
				m_Allocation,
				m_AllocInfo
			);

			TransitionImageLayout(m_Image, static_cast<VkFormat>(desc.Format), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels);
			VulkanCommon::CopyBufferToImage(stagingBuffer, m_Image, 128, 128, VK_NULL_HANDLE);
			GenerateMipmaps(m_Image, static_cast<VkFormat>(desc.Format), 128, 128, m_MipLevels); 

			vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
		}
		else
		{
			auto& sceneObj = Application::Get().GetSceneObjects()[desc.Index];

			TextureDataBlock& texture = sceneObj.GetTextureData(desc.Type);


			m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texture.Dimensions.Width, texture.Dimensions.Height)))) + 1;

			MF_CORE_DEBUG("Mip Levels: {}", m_MipLevels);

			VkDeviceSize imageSize = texture.Pixels.span.GetSize();

			VkBuffer stagingBuffer;
			VmaAllocation stagingAllocation;
			VmaAllocationInfo stagingAllocInfo;

			VmaAllocator allocator = VulkanContext::Get().GetVmaAllocator();


			VulkanCommon::CreateBuffer(
				imageSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				stagingBuffer,
				VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
				VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
				stagingAllocation,
				stagingAllocInfo
			);

			void* data;
			vmaMapMemory(allocator, stagingAllocation, &data);

			memcpy(data, texture.Pixels.span.GetData(), static_cast<size_t>(imageSize));

			vmaUnmapMemory(allocator, stagingAllocation);

			VulkanCommon::CreateImage(
				static_cast<uint32_t>(texture.Dimensions.Width),  
				static_cast<uint32_t>(texture.Dimensions.Height),
				m_MipLevels,
				VK_SAMPLE_COUNT_1_BIT,
				static_cast<VkFormat>(desc.Format),
				VK_IMAGE_TYPE_2D,
				static_cast<VkImageTiling>(desc.Tiling),
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				m_Image,
				VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
				//0,
				//VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT, 
				VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, // I'm currently using 4k textures so this is perhaps the best option. Will need to make the flag choice programmatic.
				m_Allocation,
				m_AllocInfo
			);

			TransitionImageLayout(m_Image, static_cast<VkFormat>(desc.Format), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels);
			VulkanCommon::CopyBufferToImage(stagingBuffer, m_Image, static_cast<uint32_t>(texture.Dimensions.Width), static_cast<uint32_t>(texture.Dimensions.Height), VK_NULL_HANDLE);
			GenerateMipmaps(m_Image, static_cast<VkFormat>(desc.Format), texture.Dimensions.Width, texture.Dimensions.Height, m_MipLevels);

			vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);

			// Freeing the texture memory host-side from sceneObject
			texture.Pixels.data.clear();
			texture.Pixels.data.shrink_to_fit();
		}

		
	}

	void VulkanTexture::CreateTextureImageView(const TextureDesc& desc)
	{
		m_ImageView = VulkanCommon::CreateImageView(m_Image, static_cast<VkFormat>(desc.Format), VK_IMAGE_ASPECT_COLOR_BIT, m_MipLevels);
	}

	void VulkanTexture::CreateTextureSampler(const TextureDesc& desc)
	{
		VulkanContext& context = VulkanContext::Get();

		switch (m_Type)
		{
			case TextureType::DIFFUSE:
			case TextureType::ARM:
			case TextureType::NORMAL:
			{
				if (s_ImageTexSamplerInfo.Sampler == VK_NULL_HANDLE)
				{
					VkSamplerCreateInfo samplerInfo{};
					samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
					samplerInfo.magFilter = VK_FILTER_LINEAR;
					samplerInfo.minFilter = VK_FILTER_LINEAR;
					samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
					samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
					samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

					if (context.GetSupportedFeatures().samplerAnisotropy)
					{
						samplerInfo.anisotropyEnable = VK_TRUE;
						samplerInfo.maxAnisotropy = context.GetDeviceProperties().limits.maxSamplerAnisotropy;
					}
					else
					{
						samplerInfo.anisotropyEnable = VK_FALSE;
						samplerInfo.maxAnisotropy = 1.0f;
					}

					samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
					samplerInfo.unnormalizedCoordinates = VK_FALSE;
					samplerInfo.compareEnable = VK_FALSE;
					samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
					samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
					samplerInfo.minLod = 0.0f; // Optional
					samplerInfo.maxLod = static_cast<float>(m_MipLevels); // all pbr textures of the same mesh should have the same dimensions and thus equal miplevels
					samplerInfo.mipLodBias = 0.0f; // Optional

					if (vkCreateSampler(context.GetDevice(), &samplerInfo, nullptr, &s_ImageTexSamplerInfo.Sampler) != VK_SUCCESS)
						MF_CORE_ASSERT(false, "failed to create IMAGE texture sampler!");
				}

				m_TextureSampler = s_ImageTexSamplerInfo.Sampler;
				s_ImageTexSamplerInfo.UseCount++;

				break;
			}

			case TextureType::LTC1:
			case TextureType::LTC2:
			{
				if (s_DataTexSamplerInfo.Sampler == VK_NULL_HANDLE)
				{
					VkSamplerCreateInfo samplerInfo{};
					samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
					samplerInfo.magFilter = VK_FILTER_LINEAR;
					samplerInfo.minFilter = VK_FILTER_LINEAR;
					samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
					samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
					samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
					samplerInfo.anisotropyEnable = VK_FALSE;
					samplerInfo.maxAnisotropy = 1.0f;
					samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
					samplerInfo.unnormalizedCoordinates = VK_FALSE;
					samplerInfo.compareEnable = VK_FALSE;
					samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
					samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
					samplerInfo.minLod = 0.0f; // Optional
					samplerInfo.maxLod = static_cast<float>(m_MipLevels); // all pbr textures of the same mesh should have the same dimensions and thus equal miplevels
					samplerInfo.mipLodBias = 0.0f; // Optional

					if (vkCreateSampler(context.GetDevice(), &samplerInfo, nullptr, &s_DataTexSamplerInfo.Sampler) != VK_SUCCESS)
						MF_CORE_ASSERT(false, "failed to create IMAGE texture sampler!");
				}

				m_TextureSampler = s_DataTexSamplerInfo.Sampler;
				s_DataTexSamplerInfo.UseCount++;
				break;
			}
			default:
				break;
			}

		
	}


	void VulkanTexture::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
	{
		VkCommandBuffer commandBuffer = VulkanCommon::BeginSingleTimeCommands(VK_NULL_HANDLE);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			MF_CORE_ASSERT(false, "unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		VulkanCommon::EndSingleTimeCommands(commandBuffer, VK_NULL_HANDLE);
	}


	void VulkanTexture::GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
	{ 
		VkPhysicalDevice physicalDevice = VulkanContext::Get().GetPhysicalDevice();

		// Check if image format supports linear blitting(linear filtering)
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
			throw std::runtime_error("texture image format does not support linear blitting!");
		}

		VkCommandBuffer commandBuffer = VulkanCommon::BeginSingleTimeCommands(VK_NULL_HANDLE);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;


		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VulkanCommon::EndSingleTimeCommands(commandBuffer, VK_NULL_HANDLE);
	}

}