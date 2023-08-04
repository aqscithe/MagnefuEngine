#include "mfpch.h"
#include "VulkanBuffer.h"
#include "VulkanContext.h"
#include "Magnefu/Renderer/RenderConstants.h"
#include "Magnefu/Application.h"
#include "Magnefu/Core/Maths/Quaternion.h"
#include "Magnefu/Renderer/Light.h"
#include "Magnefu/Renderer/Material.h"


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
		VmaAllocator allocator = VulkanContext::Get().GetVmaAllocator();
		vmaDestroyBuffer(allocator, m_Buffer, m_Allocation);
	}

	void VulkanBuffer::CreateVertexBuffer(const BufferDesc& desc)
	{
		VkDevice device = VulkanContext::Get().GetDevice();
		VmaAllocator allocator = VulkanContext::Get().GetVmaAllocator();

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
		VmaAllocation stagingAllocation;
		VmaAllocationInfo stagingAllocInfo;

		VulkanCommon::CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			stagingBuffer,
			VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
			stagingAllocation,
			stagingAllocInfo
		);


		void* data;
		vmaMapMemory(allocator, stagingAllocation, &data);
		memcpy(data, desc.InitData.GetData(), (size_t)bufferSize);
		vmaUnmapMemory(allocator, stagingAllocation);

		VulkanCommon::CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			m_Buffer,
			VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
			m_Allocation,
			m_AllocInfo
		);

		VulkanCommon::CopyBuffer(stagingBuffer, m_Buffer, bufferSize);

		vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
	}

	void VulkanBuffer::CreateIndexBuffer(const BufferDesc& desc)
	{
		VkDevice device = VulkanContext::Get().GetDevice();
		VmaAllocator allocator = VulkanContext::Get().GetVmaAllocator();

		VkDeviceSize bufferSize = desc.ByteSize;

		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;
		VmaAllocationInfo stagingAllocInfo;

		VulkanCommon::CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			stagingBuffer,
			VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
			stagingAllocation,
			stagingAllocInfo
		);

		void* data;
		vmaMapMemory(allocator, stagingAllocation, &data);
		memcpy(data, desc.InitData.GetData(), (size_t)bufferSize);
		vmaUnmapMemory(allocator, stagingAllocation);

		VulkanCommon::CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			m_Buffer,
			VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
			m_Allocation,
			m_AllocInfo
		);

		VulkanCommon::CopyBuffer(stagingBuffer, m_Buffer, bufferSize);

		vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
	}


	// -- Uniform Buffer -- //

	VulkanUniformBuffer::VulkanUniformBuffer(const BufferDesc& desc) : VulkanBuffer(desc)
	{
		m_UniformType = desc.UniformType;

		VkDeviceSize bufferSize = desc.ByteSize;

		VmaAllocator allocator = VulkanContext::Get().GetVmaAllocator();

		m_Buffers.resize(MAX_FRAMES_IN_FLIGHT);
		m_BuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
		m_Allocation.resize(MAX_FRAMES_IN_FLIGHT);
		m_AllocInfo.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
		{
			VulkanCommon::CreateBuffer(
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				m_Buffers[i],
				VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
				VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
				m_Allocation[i],
				m_AllocInfo[i]
			);

			vmaMapMemory(allocator, m_Allocation[i], &m_BuffersMapped[i]);
		}
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		VmaAllocator allocator = VulkanContext::Get().GetVmaAllocator();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			vmaDestroyBuffer(allocator, m_Buffers[i], m_Allocation[i]);
	}

	void VulkanUniformBuffer::UpdateUniformBuffer(const Material& mat)
	{
		VulkanContext& context = VulkanContext::Get();
		VkExtent2D swapChainExtent = context.GetSwapChainExtent();

		Application& app = Application::Get();

		switch (m_UniformType)
		{
			case Magnefu::UNIFORM_RENDERPASS:
			{
				auto& camera = app.GetWindow().GetSceneCamera();
				camera->SetAspectRatio((float)swapChainExtent.width / (float)swapChainExtent.height);
				
				auto& lights = app.GetLightData();

				RenderPassUniformBufferObject ubo{};
				ubo.ViewMatrix = camera->CalculateView();
				ubo.ProjMatrix = camera->CalculateProjection();

				ubo.ProjMatrix.c[1].e[1] *= -1; // I don't remember why I am doing this.... :(

				ubo.CameraPos = camera->GetData().Position;
				for (size_t i = 0; i < lights.size(); i++)
				{
					ubo.Lights[i] = lights[i];
				}
				ubo.LightCount = lights.size();

				
				size_t ubo_size = sizeof(ubo);
				memcpy(m_BuffersMapped[context.GetCurrentFrame()], &ubo, ubo_size);

				break;
			}

			case Magnefu::UNIFORM_MATERIAL:
			{
				/*static auto startTime = std::chrono::high_resolution_clock::now();

				auto currentTime = std::chrono::high_resolution_clock::now();
				float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();*/

				MaterialUniformBufferObject ubo{};
				//ubo.model = Maths::Quaternion::CalculateRotationMatrix(time * 45.f, Maths::vec3(0.0f,		1.0f, 0.0f));
				ubo.ModelMatrix = Maths::translate(mat.Translation) * Maths::Quaternion::CalculateRotationMatrix(mat.AngleOfRot, mat.Rotation) * Maths::scale(mat.Scale);   // Model Matrix = T * R * S
				ubo.Tint = mat.Tint;
				ubo.Reflectance = mat.Reflectance;
				ubo.Opacity = mat.Opacity;

				memcpy(m_BuffersMapped[context.GetCurrentFrame()], &ubo, sizeof(ubo));

				break;
			}
			
			case Magnefu::UNIFORM_SHADER:
			{
				break;
			}

			default:
			{
				break;
			}
		}		
	}
}

