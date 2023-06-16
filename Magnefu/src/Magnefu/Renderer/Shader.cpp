#include "mfpch.h"
#include "Shader.h"
#include "Renderer.h"
#include "Magnefu/Application.h"


namespace Magnefu
{
	Ref<Shader> Shader::Create(const String& filepath)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::NONE:
			MF_CORE_ASSERT(false, "RendererAPI::API::NONE Shader not supported");
			return nullptr;

		}

		MF_CORE_ASSERT(false, "SHADER - Unknown Renderer API");
		return nullptr;
	}

	Ref<Shader> ShaderLibrary::Get(const String& name)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::NONE:
			MF_CORE_ASSERT(false, "RendererAPI::API::NONE Shader not supported");
			return nullptr;

		}
	}
}