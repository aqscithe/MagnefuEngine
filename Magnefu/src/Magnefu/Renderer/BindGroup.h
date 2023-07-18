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
			{0, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER, ShaderStage::SHADER_STAGE_VERTEX}, // View Matrix
			{1, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER, ShaderStage::SHADER_STAGE_VERTEX}, // Proj Matrix
			{2, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER, ShaderStage::SHADER_STAGE_VERTEX}, // Camera Pos
			{3, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER, ShaderStage::SHADER_STAGE_VERTEX}, // Light Pos
			{4, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER, ShaderStage::SHADER_STAGE_FRAGMENT}, // Light Color
			{5, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER, ShaderStage::SHADER_STAGE_FRAGMENT}, // Ambient constant
			{6, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER, ShaderStage::SHADER_STAGE_FRAGMENT}, // Max Light Dist
			{7, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER, ShaderStage::SHADER_STAGE_FRAGMENT}, // Radiant Flux
			{8, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER, ShaderStage::SHADER_STAGE_FRAGMENT}, // Light Enabled
		});

	const BindingLayout DEFAULT_MATERIAL_BINDING_LAYOUT = BindingLayout({
			{ 0, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER,         ShaderStage::SHADER_STAGE_VERTEX },    // UBO
			{ 1, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT },  // DIFFUSE
			{ 2, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT },  // ARM
			{ 3, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT }   // NORMAL
		});

	const BindingLayout DEFAULT_SHADER_BINDING_LAYOUT = BindingLayout({

		});

	struct BindingTextureDescs
	{
		TextureDesc Diffuse = DiffuseTextureDesc;
		TextureDesc ARM     = ARMTextureDesc;
		TextureDesc Normal  = NormalTextureDesc;
	};

	struct BindingBufferDescs
	{
		BufferDesc Uniforms;
	};

	struct BindGroupDesc
	{
		const char*         DebugName;
		BindingLayoutType   LayoutType;
		BindingLayout       Layout;
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