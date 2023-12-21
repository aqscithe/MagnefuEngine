#pragma once


// -- Graphics Includes -------------- //
#include "Magnefu/Graphics/Mesh.h"

// -- Core Includes -------------------- //
#include "Magnefu/Core/MemoryAllocation/VmaUsage.h"

// -- Other Includes ------------------ //
#if (_MSC_VER)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include "vulkan/vulkan.h"






namespace Magnefu
{
	static const VkDeviceSize ALIGNMENT_VERTEX_BUFFER = 4;
	static const VkDeviceSize ALIGNMENT_INDEX_BUFFER = 4;

	struct VulkanVertex : public Vertex
	{
		static VkVertexInputBindingDescription GetBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(VulkanVertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 6> GetAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions{};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(VulkanVertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(VulkanVertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(VulkanVertex, normal);

			attributeDescriptions[3].binding = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(VulkanVertex, tangent);

			attributeDescriptions[4].binding = 0;
			attributeDescriptions[4].location = 4;
			attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[4].offset = offsetof(VulkanVertex, bitangent);

			attributeDescriptions[5].binding = 0;
			attributeDescriptions[5].location = 5;
			attributeDescriptions[5].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[5].offset = offsetof(VulkanVertex, texCoord);


			return attributeDescriptions;
		}

	};

	struct VulkanMemory
	{
		// Stats
		VmaStatistics Stats;
		VmaTotalStatistics TotalStats;
		VmaBudget Budgets;

		// Vertex Buffer
		Array<VkDeviceSize> VBufferOffsets;
		VkBuffer                  VBuffer;
		VmaAllocation             VBufferAllocation;
		VmaAllocationInfo         VBufferAllocInfo;

		// Index Buffer
		Array<VkDeviceSize> IBufferOffsets;
		VkBuffer                  IBuffer;
		VmaAllocation             IBufferAllocation;
		VmaAllocationInfo         IBufferAllocInfo;


		// Uniforms
		VkDeviceSize                   UniformOffset = 0;
		VkDeviceSize                   UniformAlignment;
		Array<VkBuffer>          UniformBuffers;
		Array<void*>             UniformBuffersMapped;
		Array<VmaAllocation>     UniformAllocations;
		Array<VmaAllocationInfo> UniformAllocInfo;

		// Framebuffer Resources
		VkImage DepthImage;
		VkImage ColorImage;
		VmaAllocation DepthResAllocation;
		VmaAllocation ColorResAllocation;
		VmaAllocationInfo DepthResAllocInfo;
		VmaAllocationInfo ColorResAllocInfo;
	};


	enum ShaderStage;

	namespace VulkanCommon
	{
		// Buffers
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void     CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void     CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaMemoryUsage vmaUsage, VmaAllocationCreateFlags vmaFlags, VmaAllocation& allocation, VmaAllocationInfo& allocInfo);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool commandPool);
		VkCommandBuffer BeginSingleTimeCommands(VkCommandPool commandPool);
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool commandPool);


		// Image Manipulation
		void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageType imageType, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

		void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageType imageType, VkImageTiling tiling, VkImageUsageFlags usage, VkImage& image, VmaMemoryUsage vmaUsage, VmaAllocationCreateFlags vmaFlags, VmaAllocation& allocation, VmaAllocationInfo& allocInfo);

		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkCommandPool commandPool);

		VkShaderStageFlags GetShaderStageFlags(const ShaderStage&);
		VkShaderStageFlagBits GetShaderStageFlagBits(const ShaderStage& stage);
	}
}