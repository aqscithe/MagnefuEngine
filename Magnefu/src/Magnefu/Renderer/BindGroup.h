#pragma once

#include "Magnefu/Renderer/Texture.h"
#include "Magnefu/Renderer/Buffer.h"
#include "Magnefu/Renderer/Shader.h"

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
		LAYOUT_MATERIAL
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
			{0, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER, ShaderStage::SHADER_STAGE_VERTEX_AND_FRAGMENT}
		});

	const BindingLayout DEFAULT_MATERIAL_BINDING_LAYOUT = BindingLayout({
			{ 0, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER,         ShaderStage::SHADER_STAGE_VERTEX_AND_FRAGMENT },    // UBO
			{ 1, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT },  // DIFFUSE
			{ 2, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT },  // ARM
			{ 3, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT }   // NORMAL
		});

	const BindingLayout OBJECT_LIT_BY_AREA_LIGHT_MATERIAL_BINDING_LAYOUT = BindingLayout({
			{ 0, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER,         ShaderStage::SHADER_STAGE_VERTEX_AND_FRAGMENT },    // UBO
			{ 1, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT },  // DIFFUSE
			{ 2, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT },  // ARM
			{ 3, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT },  // NORMAL
			{ 4, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT },  // LTC1
			{ 5, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT }   // LTC2	
		});


	const BindingLayout AREA_LIGHT_TEXTURED_MATERIAL_BINDING_LAYOUT = BindingLayout({
			{ 0, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER,         ShaderStage::SHADER_STAGE_VERTEX_AND_FRAGMENT },    // UBO
			{ 1, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT },  // DIFFUSE
			{ 2, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT },  // ARM
			{ 3, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT },  // NORMAL
			{ 4, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT },  // LTC1
			{ 5, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT }   // LTC2	
		});

	
	const BindingLayout AREA_LIGHT_NONTEXTURED_MATERIAL_BINDING_LAYOUT = BindingLayout({
			{ 0, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER,         ShaderStage::SHADER_STAGE_VERTEX_AND_FRAGMENT }, // UBO
			{ 1, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT },  // LTC1
			{ 2, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT },  // LTC2	
		});

	const BindingLayout NONTEXTURED_MATERIAL_BINDING_LAYOUT = BindingLayout({
			{ 0, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER,         ShaderStage::SHADER_STAGE_VERTEX_AND_FRAGMENT } // UBO
		});

	const BindingLayout DEFAULT_SHADER_BINDING_LAYOUT = BindingLayout({

		});

	struct BindingTextureDescs
	{
		TextureDesc Diffuse = DiffuseTextureDesc;
		TextureDesc ARM     = ARMTextureDesc;
		TextureDesc Normal  = NormalTextureDesc;
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
		virtual ~BindGroup();
	};

	class BindGroupFactory
	{
	public:
		static BindGroup* CreateBindGroup(const BindGroupDesc& desc);
	};

}