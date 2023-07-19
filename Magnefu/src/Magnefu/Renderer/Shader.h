#pragma once

#include "Magnefu/ResourceManagement/Pool.h"


namespace Magnefu
{
	enum ShaderStage
	{
		SHADER_STAGE_VERTEX,
		SHADER_STAGE_FRAGMENT,
		SHADER_STAGE_GEOMETRY,
		SHADER_STAGE_COMPUTE,
		SHADER_STAGE_TESSELATION,
		SHADER_STAGE_VERTEX_AND_FRAGMENT
	};

	enum DynamicState
	{
		DYNAMIC_STATE_VIEWPORT = 0,
		DYNAMIC_STATE_SCISSOR,
	};

	struct ViewportInfo
	{
		uint32_t ViewportCount;
		uint32_t Scissorcount;
	};


	enum PolygonMode
	{
		POLYGON_MODE_FILL = 0,
		POLYGON_MODE_LINE,
		POLYGON_MODE_POINT
	};

	enum CullMode
	{
		CULL_MODE_NONE,
		CULL_MODE_FRONT,
		CULL_MODE_BACK,
		CULL_MODE_FRONT_AND_BACK
	};

	struct RasterizerInfo
	{
		PolygonMode PolygonMode;  // If LINE or POINT, will need to check enabled features for fillModeNonSolid ( enabled features determined when physical device is created)
		CullMode CullMode;
	};

	enum MSAASampleCountFlag
	{
		SAMPLE_COUNT_1_BIT = 0x00000001,
		SAMPLE_COUNT_2_BIT = 0x00000002,
		SAMPLE_COUNT_4_BIT = 0x00000004,
		SAMPLE_COUNT_8_BIT = 0x00000008,
		SAMPLE_COUNT_16_BIT = 0x00000010,
		SAMPLE_COUNT_32_BIT = 0x00000020,
		SAMPLE_COUNT_64_BIT = 0x00000040
	};

	struct MSAAInfo
	{
		MSAASampleCountFlag SampleCount;
		bool EnableSampleShading;
		float MinSampleShading;
	};

	enum DepthCompareOp
	{
		COMPARE_OP_NEVER = 0,
		COMPARE_OP_LESS = 1,
		COMPARE_OP_EQUAL = 2,
		COMPARE_OP_LESS_OR_EQUAL = 3,
		COMPARE_OP_GREATER = 4,
		COMPARE_OP_NOT_EQUAL = 5,
		COMPARE_OP_GREATER_OR_EQUAL = 6,
		COMPARE_OP_ALWAYS = 7,
	};
	
	struct DepthAndStencilInfo
	{
		bool DepthTestEnable;
		bool DepthWriteEnable;
		DepthCompareOp CompareOp;
		bool DepthBoundsTestEnable;
		bool StencilTestEnable;

	};

	struct PushConstantInfo
	{
		bool Enabled;
		ShaderStage Stages;
		uint32_t Offset;
		uint32_t Size;
	};

	struct GraphicsPipelineState
	{
		std::vector<DynamicState> DynamicStates;
		ViewportInfo ViewportInfo;
		RasterizerInfo RasterizerInfo;
		MSAAInfo MSAAInfo;
		DepthAndStencilInfo DepthAndStencilInfo;
		PushConstantInfo PushConstantInfo;
	};


	struct ShaderStageDesc
	{
		const char* EntryPoint;
		ShaderStage Stage;
	};

	const ShaderStageDesc DefaultVertexShaderDesc = {
		"main",
		ShaderStage::SHADER_STAGE_VERTEX
	};

	const ShaderStageDesc DefaultFragmentShaderDesc = {
		"main",
		ShaderStage::SHADER_STAGE_FRAGMENT
	};

	struct ShaderStageDescs
	{
		ShaderStageDesc VS;
		ShaderStageDesc FS;
	};


	struct ShaderDesc
	{
		const char*      DebugName;
		const char*      Path;
		ShaderStageDescs StageDescriptions;
		Handle<BindGroup> BindGroups[2];
		GraphicsPipelineState GraphicsPipeline;
	};


	class Shader
	{
	public:
		Shader(const ShaderDesc&);
		virtual ~Shader() = default;
	};

	class ShaderFactory
	{
	public:
		static Shader* CreateShader(const ShaderDesc&);
	};
}