#pragma once


// -- Graphics Includes --------------- //
#include "Texture.h"
#include "Buffer.h"
#include "Shader.h"

namespace Magnefu
{
	

	enum BindingType
	{
		BINDING_TYPE_UNIFORM_BUFFER,
		BINDING_TYPE_COMBINED_IMAGE_SAMPLER,
	};

	enum BindingLayoutType
	{
		LAYOUT_RENDERPASS,
		LAYOUT_MATERIAL_DEFAULT,
	};

	struct Binding
	{
		uint32_t BindingPos;
		uint32_t Count;
		BindingType Type;
		ShaderStage Stage;

	};

	struct BindingLayout
	{
		std::vector<Binding> Bindings;
	};

	const BindingLayout DEFAULT_RENDERPASS_BINDING_LAYOUT = BindingLayout({
			{0, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER, ShaderStage::SHADER_STAGE_VERTEX_AND_FRAGMENT},
			{ 1, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT },  // LTC1
			{ 2, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT },   // LTC2
			{ 3, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER, ShaderStage::SHADER_STAGE_VERTEX_AND_FRAGMENT} // Area Light Info
		});

	const BindingLayout DEFAULT_MATERIAL_BINDING_LAYOUT = BindingLayout({
			{ 0, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER,         ShaderStage::SHADER_STAGE_VERTEX_AND_FRAGMENT },    // UBO
			{ 1, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT },  // DIFFUSE
			{ 2, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT },  // ARM
			{ 3, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT }   // NORMAL
		});

	const BindingLayout NONTEXTURED_MATERIAL_BINDING_LAYOUT = BindingLayout({
			{ 0, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER,         ShaderStage::SHADER_STAGE_VERTEX_AND_FRAGMENT } // UBO
		});

	const BindingLayout DEFAULT_SHADER_BINDING_LAYOUT = BindingLayout({

		});

	struct BindingTextureDescs
	{
		TextureDesc Diffuse = {};
		TextureDesc ARM = {};
		TextureDesc Normal = {};
		TextureDesc LTC1 = {};
		TextureDesc LTC2 = {};
	};

	struct BindingBufferDescs
	{
		BufferDesc Uniforms;
	};

	struct BindGroupDesc
	{
		const char*         DebugName;
		uint32_t            Index;
		BindingLayoutType   LayoutType;
		BindingLayout       Layout;
		bool                IsTextured;
		BindingTextureDescs Textures;
		BindingBufferDescs  Buffers;
	};

	class BindGroup
	{
	public:
		BindGroup(const BindGroupDesc& desc);
		~BindGroup();

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

	class BindGroupFactory
	{
	public:
		static BindGroup* CreateBindGroup(const BindGroupDesc& desc);
	};

}