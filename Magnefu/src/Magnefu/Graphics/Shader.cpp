#include "mfpch.h"
#include "Shader.h"
#include "Renderer.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Magnefu/Application.h"


namespace Magnefu
{
	
	Shader::Shader(const ShaderDesc&)
	{

	}


	Shader* ShaderFactory::CreateShader(const ShaderDesc& desc)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::VULKAN:
			{
				return new VulkanShader(desc);
			}

			default:
			{
				MF_CORE_ASSERT(false, "SHADER - Unknown Renderer API");
				return nullptr;
			}

		}
	}

}