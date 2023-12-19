// -- PCH -- //
#include "mfpch.h"

// -- header -- //
#include "Buffer.h"


// -- Graphics Includes ------------------- //
#include "Magnefu/Graphics/RenderConstants.h"
#include "Magnefu/Graphics/Material.h"


// -- Core Includes --------------------- //
#include "Magnefu/Core/Maths/Quaternion.h"




namespace Magnefu
{

	// -- Buffer ------------------- //

	Buffer::Buffer(const BufferDesc& desc)
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

	Buffer::~Buffer()
	{

	}

	Buffer* BufferFactory::CreateBuffer(const BufferDesc& desc)
	{
		return new Buffer(desc);

	}


	void Buffer::CreateVertexBuffer(const BufferDesc& desc)
	{
		VulkanMemory& vulkanMem = VulkanContext::Get().GetVulkanMemory();

		m_Buffer = vulkanMem.VBuffer;
		m_Range = desc.ByteSize;
		m_Offset = static_cast<VkDeviceSize>(desc.Offset);
	}

	void Buffer::CreateIndexBuffer(const BufferDesc& desc)
	{
		VulkanMemory& vulkanMem = VulkanContext::Get().GetVulkanMemory();

		m_Buffer = vulkanMem.IBuffer;
		m_Range = desc.ByteSize;
		m_Offset = static_cast<VkDeviceSize>(desc.Offset);
	}

	

	// -- UniformBuffer --------------------------------- //

	UniformBuffer::UniformBuffer(const BufferDesc& desc)
	{
		VulkanMemory& vulkanMem = VulkanContext::Get().GetVulkanMemory();

		m_Buffers = vulkanMem.UniformBuffers;
		m_UniformType = desc.UniformType;
		m_Range = desc.ByteSize;

		m_Offset = (vulkanMem.UniformOffset + vulkanMem.UniformAlignment - 1) & ~(vulkanMem.UniformAlignment - 1);

		// TODO: Rename UniformOffset to UniformTotalSize
		vulkanMem.UniformOffset = m_Offset + m_Range;
	}

	UniformBuffer::~UniformBuffer()
	{

	}
	void UniformBuffer::UpdateUniformBuffer(const Material& mat)
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

			auto& areaLights = app.GetAreaLightData();
			auto& areaLightVertices = app.GetAreaLightVertices();
			int aCount = app.GetAreaLightCount();

			RenderPassUniformBufferObject ubo{};
			ubo.ViewMatrix = camera->CalculateView();
			ubo.ProjMatrix = camera->CalculateProjection();

			ubo.ProjMatrix.c[1].e[1] *= -1; // I don't remember why I am doing this.... :(

			ubo.CameraPos = camera->GetData().Position;

			// Copy area light data into UBO

			for (size_t i = 0; i < areaLights.size(); i++)
			{
				ubo.AreaLights[i] = areaLights[i];
			}
			ubo.AreaLightCount = areaLights.size();
			ubo.AreaLightVertices = areaLightVertices;


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

	void UniformBuffer::UpdateUniformBuffer(const MaterialInstanced& mat, uint32_t instanceCount)
	{
		VulkanContext& context = VulkanContext::Get();
		VulkanMemory& vulkanMem = context.GetVulkanMemory();
		VkExtent2D swapChainExtent = context.GetSwapChainExtent();

		Application& app = Application::Get();

		MaterialUniformBufferObjectInstanced ubo{};

		for (int instance = 0; instance < instanceCount; instance++)
		{
			ubo.ModelMatrix[instance] = Maths::translate(mat.Translation[instance]) *
				Maths::Quaternion::CalculateRotationMatrix(mat.AngleOfRot[instance], mat.Rotation[instance]) *
				Maths::scale(mat.Scale[instance]);
		}

		assert(sizeof(ubo) == m_Range);
		void* data = static_cast<char*>(vulkanMem.UniformBuffersMapped[context.GetCurrentFrame()]) + m_Offset;
		memcpy(data, &ubo, m_Range);
	}
}