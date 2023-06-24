#include "mfpch.h"
#include "RenderPipeline.h"
#include "Magnefu/Renderer/Renderer.h"
#include "Platform/VK/VKGraphicsPipeline.h"
#include "Magnefu/Application.h"

namespace Magnefu
{
	Ref<RenderPipeline> RenderPipeline::Create(int temp)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::VULKAN:
			{
				Application& app = Application::Get();
				return app.GetResourceCache().RequestResource<VKGraphicsPipeline>(temp);
			}

			case RendererAPI::API::NONE:
			{
				MF_CORE_ASSERT(false, "RendererAPI::API::NONE Render Pipeline not supported");
				return nullptr;
			}
		}

		MF_CORE_ASSERT(false, "SHADER - Unknown Renderer API");
		return nullptr;
	}
	
}