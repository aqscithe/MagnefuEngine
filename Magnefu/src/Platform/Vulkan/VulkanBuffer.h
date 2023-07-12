#pragma once

#include "Magnefu/Renderer/Buffer.h"
#include "VulkanCommon.h"


namespace Magnefu
{
	class VulkanBuffer : public Buffer
	{
	public:
		VulkanBuffer(const BufferDesc& desc);
		~VulkanBuffer();

		VkBuffer& GetBuffer() { return m_Buffer; }


	private:
		void CreateVertexBuffer(const BufferDesc& desc);
		void CreateIndexBuffer(const BufferDesc& desc);

		

	protected:
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);


	private:
		VkBuffer        m_Buffer;
		VkDeviceMemory  m_BufferMemory;
		
		
	};


	class VulkanUniformBuffer : public VulkanBuffer
	{
	public:
		VulkanUniformBuffer(const BufferDesc& desc);
		~VulkanUniformBuffer();

		void UpdateUniformBuffer();

		std::vector<VkBuffer>& GetBuffers() { return m_Buffers; }
		
	private:
		std::vector<VkBuffer>        m_Buffers;
		std::vector<VkDeviceMemory>  m_BuffersMemory;
		std::vector<void*>           m_BuffersMapped;
	};
}
