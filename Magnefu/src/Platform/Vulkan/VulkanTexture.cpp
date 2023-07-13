#include "mfpch.h"
#include "VulkanTexture.h"
#include "VulkanContext.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"


namespace Magnefu
{
	static const std::string BASE_TEXTURE_PATH = "res/textures/scificorridor/scene_1001_BaseColor.png";
	static const std::string METAL_TEXTURE_PATH = "res/textures/scificorridor/scene_1001_Metalness.png";
	static const std::string ROUGHNESS_TEXTURE_PATH = "res/textures/scificorridor/scene_1001_Roughness.png";
	static const std::string NORMAL_TEXTURE_PATH = "res/textures/scificorridor/scene_1001_Normal.png";
	static const std::string AO_TEXTURE_PATH = "res/textures/scificorridor/scene_1001_AO.png";
	static const std::string ARM_TEXTURE_PATH = "res/textures/scificorridor/scene_1001_ARM.png";

	static const std::array<std::string, 3> TEXTURE_PATHS
	{
		BASE_TEXTURE_PATH,
		ARM_TEXTURE_PATH,
		NORMAL_TEXTURE_PATH,
	};

	Ref<VkSampler> VulkanTexture::s_TextureSampler = CreateRef<VkSampler>();
	bool VulkanTexture::s_SamplerCreated = false;


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

		CreateTextureImage(desc);
		CreateTextureImageView(desc);

		if (!s_SamplerCreated)
		{
			CreateTextureSampler();
			s_SamplerCreated = true;
		}
	}

	VulkanTexture::~VulkanTexture()
	{
		VkDevice device = VulkanContext::Get().GetDevice();

		if (s_SamplerCreated)
		{
			vkDestroySampler(device, *s_TextureSampler.get(), nullptr);
			s_SamplerCreated = false;
		}
		vkDestroyImageView(device, m_ImageView, nullptr);
		vkDestroyImage(device, m_Image, nullptr);
		vkFreeMemory(device, m_BufferMemory, nullptr);
		

	}

	void VulkanTexture::CreateTextureImage(const TextureDesc& desc)
	{

		VkDevice device = VulkanContext::Get().GetDevice();
		
		int width, height, channels;
		stbi_set_flip_vertically_on_load(0);

		stbi_uc* pixels = stbi_load(
			TEXTURE_PATHS[static_cast<uint32_t>(desc.Type)].c_str(),
			&width, &height, &channels,
			desc.RequestedChannels
		);

		if (!pixels)
			MF_CORE_ASSERT(false, "failed to load texture image!");

		m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

		MF_CORE_DEBUG("Mip Levels: {0} | Width: {1} | Height: {2} | Channels: {3} | Requested Channels: {4}", m_MipLevels, width, height, channels, desc.RequestedChannels);

		VkDeviceSize imageSize = width * height * desc.RequestedChannels;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device, stagingBufferMemory);

		stbi_image_free(pixels);

		CreateImage(
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height),
			m_MipLevels,
			VK_SAMPLE_COUNT_1_BIT,
			static_cast<VkFormat>(desc.Format),
			VK_IMAGE_TYPE_2D,
			static_cast<VkImageTiling>(desc.Tiling),
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_Image,
			m_BufferMemory
		);

		TransitionImageLayout(m_Image, static_cast<VkFormat>(desc.Format), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels);
		CopyBufferToImage(stagingBuffer, m_Image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
		//TransitionImageLayout(m_TextureImage, texture.Format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_MipLevels);
		GenerateMipmaps(m_Image, static_cast<VkFormat>(desc.Format), width, height, m_MipLevels);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void VulkanTexture::CreateTextureImageView(const TextureDesc& desc)
	{
		m_ImageView = CreateImageView(m_Image, static_cast<VkFormat>(desc.Format), VK_IMAGE_ASPECT_COLOR_BIT, m_MipLevels);
	}

	void VulkanTexture::CreateTextureSampler()
	{
		VulkanContext& context = VulkanContext::Get();

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

		if (vkCreateSampler(context.GetDevice(), &samplerInfo, nullptr, s_TextureSampler.get()) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create texture sampler!");
	}

	
	void VulkanTexture::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageType imageType, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		VulkanContext& context = VulkanContext::Get();
		VkDevice device = context.GetDevice();

		VkPhysicalDeviceImageFormatInfo2 imageFormatInfo = {};
		imageFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
		imageFormatInfo.format = format;  // replace with the format you want to query
		imageFormatInfo.type = imageType; // or whatever image type you're interested in
		imageFormatInfo.tiling = tiling;
		imageFormatInfo.usage = usage;
		imageFormatInfo.flags = 0;

		VkImageFormatProperties2 imageFormatProperties = {};
		imageFormatProperties.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;

		if (vkGetPhysicalDeviceImageFormatProperties2(context.GetPhysicalDevice(), &imageFormatInfo, &imageFormatProperties) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Image format w/ specified properties not supported!");

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = imageType;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = numSamples;
		imageInfo.flags = 0; // Optional

		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to create image!");

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to allocate image memory!");

		vkBindImageMemory(device, image, imageMemory, 0);
	}

	VkImageView VulkanTexture::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // VK_IMAGE_VIEW_TYPE_CUBE -> for cube map
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;



		VkImageView imageView;
		if (vkCreateImageView(VulkanContext::Get().GetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to create image or texture image view!");

		return imageView;
	}

	void VulkanTexture::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

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

		EndSingleTimeCommands(commandBuffer);
	}

	void VulkanTexture::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		EndSingleTimeCommands(commandBuffer);
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

		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

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

		EndSingleTimeCommands(commandBuffer);
	}

	uint32_t VulkanTexture::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(VulkanContext::Get().GetPhysicalDevice(), &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		MF_CORE_ASSERT(false, "failed to find suitable memory type!");

		return 0;
	}

	void VulkanTexture::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkDevice device = VulkanContext::Get().GetDevice();

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		//bufferInfo.flags = 

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create vertex buffer!");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.allocationSize = memRequirements.size;
		memAllocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		// TODO:
		// It should be noted that in a real world application, you're not supposed to actually call
		// vkAllocateMemory for every individual buffer. The maximum number of simultaneous memory 
		// allocations is limited by the maxMemoryAllocationCount physical device limit, which may 
		// be as low as 4096 even on high end hardware like an NVIDIA GTX 1080. The right way to 
		// allocate memory for a large number of objects at the same time is to create a custom 
		// allocator that splits up a single allocation among many different objects by using the 
		// offset parameters that we've seen in many functions.

		// You can either implement such an allocator yourself, or use the VulkanMemoryAllocator 
		// library provided by the GPUOpen initiative.However, for this tutorial it's okay to use a 
		// separate allocation for every resource, because we won't come close to hitting any of 
		// these limits for now.
		if (vkAllocateMemory(device, &memAllocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to allocate vertex buffer memory!");

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	VkCommandBuffer VulkanTexture::BeginSingleTimeCommands()
	{
		VulkanContext& context = VulkanContext::Get();

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = context.GetCommandPool();
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(context.GetDevice(), &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void VulkanTexture::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		VulkanContext& context = VulkanContext::Get();

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(context.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);

		//A fence would allow you to schedule multiple transfers simultaneously
		// and wait for all of them complete, instead of executing one at a time.
		// That may give the driver more opportunities to optimize.
		// 
		//vkWaitForFences()
		vkQueueWaitIdle(context.GetGraphicsQueue());

		vkFreeCommandBuffers(context.GetDevice(), context.GetCommandPool(), 1, &commandBuffer);
	}

}