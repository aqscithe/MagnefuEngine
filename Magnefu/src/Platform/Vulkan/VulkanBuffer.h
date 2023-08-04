#pragma once

#include "Magnefu/Renderer/Buffer.h"
#include "VulkanCommon.h"
#include "Magnefu/Renderer/Material.h"


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


	private:
		VkBuffer        m_Buffer;
		VkDeviceMemory  m_BufferMemory;

		VmaAllocation     m_Allocation;
		VmaAllocationInfo m_AllocInfo;
		
		
	};


	class VulkanUniformBuffer : public VulkanBuffer
	{
	public:
		VulkanUniformBuffer(const BufferDesc& desc);
		~VulkanUniformBuffer();

		void UpdateUniformBuffer(const Material& mat);

		std::vector<VkBuffer>& GetBuffers() { return m_Buffers; }
		VkDeviceSize GetRange() { return m_Range; }
		VkDeviceSize GetOffset() { return m_Offset; }
		
	private:
		std::vector<VkBuffer>        m_Buffers;
		VkDeviceSize  m_Offset;
		VkDeviceSize  m_Range;

		UniformBufferType            m_UniformType = UniformBufferType::UNIFORM_NONE;
		std::vector<void*>           m_BuffersMapped;

		std::vector<VmaAllocation>     m_Allocation;
		std::vector<VmaAllocationInfo> m_AllocInfo;

	};
}
