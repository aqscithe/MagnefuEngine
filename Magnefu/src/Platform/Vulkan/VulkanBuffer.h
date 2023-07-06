#pragma once

#include "Magnefu/Renderer/Buffer.h"
#include "vulkan/vulkan.h"


namespace Magnefu
{
	class VulkanBuffer : public Buffer
	{
	public:
		VulkanBuffer(const BufferDesc& desc);

		~VulkanBuffer();

	private:
	};
}
