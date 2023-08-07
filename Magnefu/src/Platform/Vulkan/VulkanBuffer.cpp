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


		VulkanMemory& vulkanMem = VulkanContext::Get().GetVulkanMemory();

		m_Buffer = vulkanMem.VBuffer;
		m_Range = desc.ByteSize;
		m_Offset = static_cast<VkDeviceSize>(desc.Offset);

		/*VkBuffer stagingBuffer;
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

		vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);*/
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
		VulkanMemory& vulkanMem = VulkanContext::Get().GetVulkanMemory();

		m_Buffers = vulkanMem.UniformBuffers;
		m_UniformType = desc.UniformType;
		m_Range = desc.ByteSize;

		m_Offset = (vulkanMem.UniformOffset + vulkanMem.UniformAlignment - 1) & ~(vulkanMem.UniformAlignment - 1);

		vulkanMem.UniformOffset = m_Offset + m_Range;
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		// Needs to be done in vulkan context now

		// Also, will need a system that says "hey the data at this offset is not being used
		// feel free to put a uniform's data here"
		// Will become more pertinent once I want to add and remove objects at runtime.

		// Perhaps this is what the offset allocator could be used for...

	}

	void VulkanUniformBuffer::UpdateUniformBuffer(const Material& mat)
	{
		VulkanContext& context = VulkanContext::Get();
		VulkanMemory& vulkanMem = context.GetVulkanMemory();
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

				
				assert(sizeof(ubo) == m_Range);

				void* data = static_cast<char*>(vulkanMem.UniformBuffersMapped[context.GetCurrentFrame()]) + m_Offset;
				//memcpy(m_BuffersMapped[context.GetCurrentFrame()], &ubo, ubo_size);
				memcpy(data, &ubo, m_Range);

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

				assert(sizeof(ubo) == m_Range);
				void* data = static_cast<char*>(vulkanMem.UniformBuffersMapped[context.GetCurrentFrame()]) + m_Offset;
				//memcpy(m_BuffersMapped[context.GetCurrentFrame()], &ubo, sizeof(ubo));
				memcpy(data, &ubo, m_Range);

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

