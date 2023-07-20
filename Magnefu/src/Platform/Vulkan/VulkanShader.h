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

		inline const VkPipelineLayout& GetPipelineLayout() { return m_PipelineLayout; }
		inline const VkPipeline& GetPipeline() { return m_Pipeline; }

	private: 
		void CreateGraphicsPipeline(const ShaderDesc&);
		//void CreateComputePipeline(const ShaderDesc&);

		ShaderList ParseShader(const char*);
		VkShaderModule CreateShaderModule(const ShaderSource&);

		VkDynamicState TranslateDynamicState(const DynamicState&);
		VkPolygonMode TranslatePolygonMode(const PolygonMode&);
		VkCullModeFlags TranslateCullMode(const CullMode&);
		VkCompareOp TranslateDepthCompareOp(const DepthCompareOp&);

	private:
		VkPipelineLayout m_PipelineLayout;
		VkPipeline       m_Pipeline;
	};
	
}