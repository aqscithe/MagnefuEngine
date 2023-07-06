#pragma once

#include "Magnefu/Renderer/Buffer.h"
#include "vulkan/vulkan.h"


namespace Magnefu
{
	class VulkanBuffer : public Buffer
	{
	public:
		VulkanBuffer(const BufferDesc& desc);

		~VulkanBuffer();

	protected:
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

	protected:

		std::vector<VkBuffer>        m_Buffers;
		std::vector<VkDeviceMemory>  m_BuffersMemory;
		
	};


	class VulkanUniformBuffer : public VulkanBuffer
	{
	public:
		VulkanUniformBuffer(const BufferDesc& desc);
		~VulkanUniformBuffer();

	private:
		std::vector<void*>           m_BuffersMapped;
	};
}
