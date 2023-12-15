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
		VkDeviceSize GetRange() { return m_Range; }
		VkDeviceSize GetOffset() { return m_Offset; }


	private:
		void CreateVertexBuffer(const BufferDesc& desc);
		void CreateIndexBuffer(const BufferDesc& desc);

	protected:
		VkDeviceSize  m_Offset;
		VkDeviceSize  m_Range;


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
		void UpdateUniformBuffer(const MaterialInstanced& mat, uint32_t instanceCount);

		Array<VkBuffer>& GetBuffers() { return m_Buffers; }
		
		
	private:
		Array<VkBuffer>        m_Buffers;
		

		UniformBufferType            m_UniformType = UniformBufferType::UNIFORM_NONE;

	};
}
