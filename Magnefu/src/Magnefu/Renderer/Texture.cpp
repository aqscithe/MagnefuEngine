#include "mfpch.h"
#include "Texture.h"
#include "Magnefu/ResourceManagement/CacheableResource.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Renderer.h"
#include "Magnefu/Application.h"

namespace Magnefu
{

	Ref<Texture> Texture::Create(const std::string& filepath)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::NONE:
			{
				MF_CORE_ASSERT(false, "RendererAPI::API::NONE Texture not supported");
				return nullptr;
			}
			case RendererAPI::API::OPENGL:
			{
				Application& app = Application::Get();
				return app.GetResourceCache().RequestResource<OpenGLTexture>(filepath);
			}
			
		}

		MF_CORE_ASSERT(false, "TEXTURE - Unknown Renderer API");
		return nullptr;
	}
}