#pragma once

// -- Graphcis Includes -- //
#include "Magnefu/Graphics/BindGroup.h"
#include "VulkanCommon.h"
#include "Magnefu/Graphics/ResourceManagement/ResourceManager.h"



namespace Magnefu
{

	class VulkanBindGroup : public BindGroup
	{
	public:
		VulkanBindGroup(const BindGroupDesc&);
		~VulkanBindGroup();

		inline Handle<Buffer>& GetUniformsHandle() { return m_Uniforms; }
		inline VkDescriptorSet& GetFrameDescriptorSet(uint32_t currentFrame) { return m_DescriptorSets[currentFrame]; }
		inline VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_DescriptorSetLayout; }

	private:
		void CreateDescriptorSetLayout(const BindingLayout&);
		void CreateBindingBuffers(const BindingBufferDescs&);
		void CreateBindingTextures(const BindingTextureDescs&);
		void CreateDescriptorPool(const BindingLayoutType&, bool isTextured);
		void CreateDescriptorSets(const BindingLayoutType& type, bool isTextured);

		VkDescriptorType GetDescriptorType(const BindingType&);
		

	private:
		VkDescriptorSetLayout        m_DescriptorSetLayout;
		VkDescriptorPool             m_DescriptorPool;
		std::vector<VkDescriptorSet> m_DescriptorSets;

		Handle<Buffer>   m_Uniforms;
		Handle<Texture>  m_DiffuseTexture;
		Handle<Texture>  m_ARMTexture;
		Handle<Texture>  m_NormalTexture;
		Handle<Texture>  m_LTC1_Texture;
		Handle<Texture>  m_LTC2_Texture;
	};
}