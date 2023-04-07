#include "mfpch.h"
#include "Shader.h"
#include "Platform/OpenGL/OpenGLShader.h"
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
		case RendererAPI::API::OPENGL:
		{
			Application& app = Application::Get();
			return app.GetResourceCache().RequestResource<OpenGLShader>(filepath);
		}

		}

		MF_CORE_ASSERT(false, "SHADER - Unknown Renderer API");
		return nullptr;
	}
}