#pragma once
#include "Magnefu/Renderer/RenderPipeline.h"
#include "Magnefu/ResourceManagement/CacheableResource.h"

namespace Magnefu
{
	class VKGraphicsPipeline : public RenderPipeline, public CacheableResource
	{
	public:
		VKGraphicsPipeline(int temp);
		~VKGraphicsPipeline();
	};
}