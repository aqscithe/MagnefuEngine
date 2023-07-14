#include "mfpch.h"
#include "VulkanCommon.h"
#include "VulkanContext.h"

namespace Magnefu
{
	namespace VulkanCommon
	{
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
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

		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
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

		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
		{
			VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

			VkBufferCopy copyRegion{};
			copyRegion.srcOffset = 0; // Optional
			copyRegion.dstOffset = 0; // Optional
			copyRegion.size = size;
			vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

			EndSingleTimeCommands(commandBuffer);
		}

		VkCommandBuffer BeginSingleTimeCommands()
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

		void EndSingleTimeCommands(VkCommandBuffer commandBuffer)
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


		// TODO(async):
	// All of the helper functions that submit commands so far have been set up to execute synchronously by waiting 
	// for the queue to become idle. For practical applications it is recommended to combine these operations in a 
	// single command buffer and execute them asynchronously for higher throughput, especially the transitions and 
	// copy in the createTextureImage function. Try to experiment with this by creating a setupCommandBuffer that 
	// the helper functions record commands into, and add a flushSetupCommands to execute the commands that have been 
	// recorded so far. It's best to do this after the texture mapping works to check if the texture resources are 
	// still set up correctly.

		void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageType imageType, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
		{
			VkDevice device = VulkanContext::Get().GetDevice();

			VkPhysicalDeviceImageFormatInfo2 imageFormatInfo = {};
			imageFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
			imageFormatInfo.format = format;  // replace with the format you want to query
			imageFormatInfo.type = imageType; // or whatever image type you're interested in
			imageFormatInfo.tiling = tiling;
			imageFormatInfo.usage = usage;
			imageFormatInfo.flags = 0;

			VkImageFormatProperties2 imageFormatProperties = {};
			imageFormatProperties.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;

			if (vkGetPhysicalDeviceImageFormatProperties2(VulkanContext::Get().GetPhysicalDevice(), &imageFormatInfo, &imageFormatProperties) != VK_SUCCESS)
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
			allocInfo.memoryTypeIndex = VulkanCommon::FindMemoryType(memRequirements.memoryTypeBits, properties);

			if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
				MF_CORE_ASSERT(false, "Failed to allocate image memory!");

			vkBindImageMemory(device, image, imageMemory, 0);
		}

		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
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

		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
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
	}
}