#pragma once

#include "Magnefu/Renderer/BindGroup.h"
#include "Platform/Vulkan/VulkanCommon.h"
#include "Magnefu/ResourceManagement/ResourceManager.h"



namespace Magnefu
{

	class VulkanBindGroup : public BindGroup
	{
	public:
		VulkanBindGroup(const BindGroupDesc&);
		~VulkanBindGroup();

		Handle<Buffer>& GetUniformsHandle() { return m_Uniforms; }

	private:
		void CreateDescriptorSetLayout(const MaterialBindingLayout&);
		void CreateBindingBuffers(const BindingBufferDescs&);
		void CreateBindingTextures(const BindingTextureDescs&);
		void CreateDescriptorPool(const MaterialBindingLayout& layout);
		void CreateDescriptorSets();

		VkDescriptorType GetDescriptorType(const BindingType&);
		VkShaderStageFlags GetShaderStageFlags(const ShaderStage&);

	private:
		VkDescriptorSetLayout        m_DescriptorSetLayout;
		VkDescriptorPool             m_DescriptorPool;
		std::vector<VkDescriptorSet> m_DescriptorSets;

		Handle<Buffer>   m_Uniforms;
		Handle<Texture>  m_DiffuseTexture;
		Handle<Texture>  m_ARMTexture;
		Handle<Texture>  m_NormalTexture;
	};
}