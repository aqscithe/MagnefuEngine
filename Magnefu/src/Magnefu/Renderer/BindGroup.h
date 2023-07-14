#pragma once

#include "Magnefu/Renderer/Texture.h"
#include "Magnefu/Renderer/Buffer.h"

namespace Magnefu
{
	enum ShaderStage
	{
		SHADER_STAGE_VERTEX,
		SHADER_STAGE_FRAGMENT
	};

	enum BindingType
	{
		BINDING_TYPE_UNIFORM_BUFFER,
		BINDING_TYPE_COMBINED_IMAGE_SAMPLER,
	};

	struct Binding
	{
		uint32_t BindingPos;
		uint32_t Count;
		BindingType Type;
		ShaderStage Stage;

	};


	struct MaterialBindingLayout
	{
		Binding  UBO =                   { 0, 1, BindingType::BINDING_TYPE_UNIFORM_BUFFER,         ShaderStage::SHADER_STAGE_VERTEX };
		Binding  DiffuseTextureSampler = { 1, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT };
		Binding  ARMTextureSampler =     { 2, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT };
		Binding  NormalTextureSampler =  { 3, 1, BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER, ShaderStage::SHADER_STAGE_FRAGMENT };
	};

	struct BindingTextureDescs
	{
		TextureDesc Diffuse = DiffuseTextureDesc;
		TextureDesc ARM     = ARMTextureDesc;
		TextureDesc Normal  = NormalTextureDesc;
	};

	struct BindingBufferDescs
	{
		BufferDesc Uniforms = DefaultUniformBufferDesc;
	};

	struct BindGroupDesc
	{
		const char*           DebugName;
		MaterialBindingLayout Layout;
		BindingTextureDescs   Textures;
		BindingBufferDescs    Buffers;
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