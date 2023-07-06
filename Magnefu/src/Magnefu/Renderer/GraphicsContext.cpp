#include "mfpch.h"
#include "GraphicsContext.h"
#include "Magnefu/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanContext.h"

#include "GLFW/glfw3.h"

namespace Magnefu
{
	GraphicsContext* Magnefu::GraphicsContext::Create(void* windowHandle)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::VULKAN:
			{
				return new VulkanContext(static_cast<GLFWwindow*>(windowHandle));
			}

			case RendererAPI::API::NONE:
			{
				MF_CORE_ASSERT(false, "RendererAPI::API::NONE Graphics Context not supported");
				return nullptr;
			}

		}

		MF_CORE_ASSERT(false, "GRAPHICS CONTEXT - Unknown Renderer API");
		return nullptr;
	}
}

