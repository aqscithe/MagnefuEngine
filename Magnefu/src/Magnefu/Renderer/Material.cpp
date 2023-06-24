#include "mfpch.h"
#include "Material.h"
#include "Magnefu/Application.h"
#include "Magnefu/Renderer/Renderer.h"

namespace Magnefu
{
	Ref<Material> Material::Create(const String& shaderPath, const MaterialOptions& options)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::NONE:
			{
				MF_CORE_ASSERT(false, "RendererAPI::API::NONE Shader not supported");
				return nullptr;
			}

		}

		MF_CORE_ASSERT(false, "SHADER - Unknown Renderer API");
		return nullptr;
	}
}