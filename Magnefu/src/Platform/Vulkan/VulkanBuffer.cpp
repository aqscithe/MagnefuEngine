#include "mfpch.h"
#include "VulkanBuffer.h"
#include "VulkanContext.h"
#include "Magnefu/Renderer/RenderConstants.h"
#include "Magnefu/Application.h"
#include "Magnefu/Core/Maths/Quaternion.h"


namespace Magnefu
{
	VulkanBuffer::VulkanBuffer(const BufferDesc& desc) : Buffer(desc)
	{
		switch (desc.Usage)
		{
			case USAGE_VERTEX:
			{
				CreateVertexBuffer(desc);
				break;
			}

			case USAGE_INDEX:
			{
				CreateIndexBuffer(desc);
				break;
			}

		default:
			break;
		}
		
	}

	VulkanBuffer::~VulkanBuffer()
	{
		VkDevice device = VulkanContext::Get().GetDevice();
		vkDestroyBuffer(device, m_Buffer, nullptr);
		vkFreeMemory(device, m_BufferMemory, nullptr);
	}

	void VulkanBuffer::CreateVertexBuffer(const BufferDesc& desc)
	{
		VkDevice device = VulkanContext::Get().GetDevice();

		// TODO:
		// The previous chapter already mentioned that you should allocate multiple resources like 
		// buffers from a single memory allocation, but in fact you should go a step further. Driver 
		// developers recommend that you also store multiple buffers, like the vertex and index 
		// buffer, into a single VkBuffer and use offsets in commands like vkCmdBindVertexBuffers. 
		// The advantage is that your data is more cache friendly in that case, because it's closer 
		// together. It is even possible to reuse the same chunk of memory for multiple resources 
		// if they are not used during the same render operations, provided that their data is 
		// refreshed, of course. This is known as aliasing and some Vulkan functions have explicit 
		 // flags to specify that you want to do this.

		VkDeviceSize bufferSize = desc.ByteSize;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		VulkanCommon::CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);


		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, desc.InitData.GetData(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		VulkanCommon::CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_Buffer,
			m_BufferMemory
		);

		VulkanCommon::CopyBuffer(stagingBuffer, m_Buffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void VulkanBuffer::CreateIndexBuffer(const BufferDesc& desc)
	{
		VkDevice device = VulkanContext::Get().GetDevice();

		VkDeviceSize bufferSize = desc.ByteSize;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		VulkanCommon::CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, desc.InitData.GetData(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		VulkanCommon::CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_Buffer,
			m_BufferMemory
		);

		VulkanCommon::CopyBuffer(stagingBuffer, m_Buffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}


	// -- Uniform Buffer -- //

	VulkanUniformBuffer::VulkanUniformBuffer(const BufferDesc& desc) : VulkanBuffer(desc)
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		m_Buffers.resize(MAX_FRAMES_IN_FLIGHT);
		m_BuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		m_BuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VulkanCommon::CreateBuffer(
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_Buffers[i],
				m_BuffersMemory[i]
			);

			vkMapMemory(VulkanContext::Get().GetDevice(), m_BuffersMemory[i], 0, bufferSize, 0, &m_BuffersMapped[i]);
		}
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		VkDevice device = VulkanContext::Get().GetDevice();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(device, m_Buffers[i], nullptr);
			vkFreeMemory(device, m_BuffersMemory[i], nullptr);
		}
	}

	void VulkanUniformBuffer::UpdateUniformBuffer()
	{
		VulkanContext& context = VulkanContext::Get();
		VkExtent2D swapChainExtent = context.GetSwapChainExtent();

		/*static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();*/

		auto& camera = Application::Get().GetWindow().GetSceneCamera();
		camera->SetAspectRatio((float)swapChainExtent.width / (float)swapChainExtent.height);

		UniformBufferObject ubo{};
		//ubo.model = Maths::Quaternion::CalculateRotationMatrix(time * 45.f, Maths::vec3(0.0f,		1.0f, 0.0f));
		ubo.model = Maths::Quaternion::CalculateRotationMatrix(0.f, Maths::vec3(0.0f, 1.0f, 0.0f));
		ubo.view = camera->CalculateView();
		ubo.proj = camera->CalculateProjection();

		ubo.proj.c[1].e[1] *= -1;

		memcpy(m_BuffersMapped[context.GetCurrentFrame()], &ubo, sizeof(ubo));
	}
}

