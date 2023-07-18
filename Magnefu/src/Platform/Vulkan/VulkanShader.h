#pragma once
#include "Magnefu/Renderer/Shader.h"
#include "VulkanCommon.h"

namespace Magnefu
{
	class VulkanShader : public Shader
	{
	public:
		VulkanShader(const ShaderDesc&);
		~VulkanShader();
	};
	
}