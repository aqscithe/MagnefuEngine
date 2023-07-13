#include "mfpch.h"
#include "Texture.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Renderer.h"

namespace Magnefu
{
	

	Texture::Texture(TextureDesc& desc)
	{
		
	}

	Texture* TextureFactory::CreateTexture(TextureDesc& desc)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::VULKAN:
			{
				return VulkanTexture(desc);
			}

			default:
			{
				MF_CORE_ASSERT(false, "TEXTURE - Unknown Renderer API");
				return nullptr;
			}
			
		}

		
		
	}
}