#include "mfpch.h"
#include "Texture.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Renderer.h"

namespace Magnefu
{
	

	Texture::Texture(const TextureDesc& desc)
	{
		
	}

	Texture* TextureFactory::CreateTexture(const TextureDesc& desc)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::VULKAN:
			{
				return new VulkanTexture(desc);
			}

			default:
			{
				MF_CORE_ASSERT(false, "TEXTURE - Unknown Renderer API");
				return nullptr;
			}
			
		}

		
		
	}
}