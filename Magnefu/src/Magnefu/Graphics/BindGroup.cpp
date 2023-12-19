#include "mfpch.h"
#include "BindGroup.h"
#include "Renderer.h"
#include "Platform/Vulkan/VulkanBindGroup.h"

namespace Magnefu
{
    BindGroup::BindGroup(const BindGroupDesc& desc)
    {

    }

    BindGroup::~BindGroup()
    {

    }

    BindGroup* BindGroupFactory::CreateBindGroup(const BindGroupDesc& desc)
    {
        switch (Renderer::GetAPI())
        {

            case RendererAPI::API::VULKAN:
            {
                return new VulkanBindGroup(desc);
            }

            default:
            {
                MF_CORE_ASSERT(false, "BIND GROUP - Unknown Renderer API");
                return nullptr;
            }
        }
    }
}