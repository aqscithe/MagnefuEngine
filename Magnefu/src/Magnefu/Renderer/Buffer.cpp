#include "mfpch.h"
#include "Buffer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace Magnefu
{
	Buffer::Buffer(const BufferDesc& desc)
	{
	
	}

	Buffer* BufferFactory::CreateBuffer(const BufferDesc& desc)
	{
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::VULKAN:
            {
                if (desc.Usage == BufferUsage::USAGE_UNIFORM)
                    return new VulkanUniformBuffer(desc);

                return new VulkanBuffer(desc);
            }

            default:
            {
                MF_CORE_ASSERT(false, "RendererAPI::API::NONE Buffer not supported");
                return nullptr;
            }
        }
        
	}
}