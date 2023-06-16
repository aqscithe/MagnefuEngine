#include "mfpch.h"
#include "GraphicsContext.h"
#include "Magnefu/Renderer/Renderer.h"
#include "Platform/VK/VKContext.h"


namespace Magnefu
{
	GraphicsContext* Magnefu::GraphicsContext::Create(GLFWwindow* windowHandle)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::VULKAN:
			{
				return new VKContext(windowHandle);
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

