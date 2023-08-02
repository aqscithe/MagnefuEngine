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
		
		
	};


	class VulkanUniformBuffer : public VulkanBuffer
	{
	public:
		VulkanUniformBuffer(const BufferDesc& desc);
		~VulkanUniformBuffer();

		void UpdateUniformBuffer(const Material& mat);

		std::vector<VkBuffer>& GetBuffers() { return m_Buffers; }
		
	private:
		std::vector<VkBuffer>        m_Buffers;
		std::vector<VkDeviceMemory>  m_BuffersMemory;
		std::vector<void*>           m_BuffersMapped;
		//std::vector< OffsetAllocator::Allocation>  m_Allocations;

		UniformBufferType            m_UniformType = UniformBufferType::UNIFORM_NONE;

		
	};
}
