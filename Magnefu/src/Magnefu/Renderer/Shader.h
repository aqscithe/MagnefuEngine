#pragma once


namespace Magnefu
{
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

	enum DynamicState
	{
		DYNAMIC_STATE_VIEWPORT = 0,
		DYNAMIC_STATE_SCISSOR,
	};

	struct GraphicsPipelineState
	{
		std::vector<DynamicState> DynamicStates;
		ViewportInfo ViewportInfo;
		RasterizerInfo RasterizerInfo;
	};



	enum ShaderStage
	{
		SHADER_STAGE_VERTEX,
		SHADER_STAGE_FRAGMENT,
		SHADER_STAGE_GEOMETRY,
		SHADER_STAGE_COMPUTE,
		SHADER_STAGE_TESSELATION,
		SHADER_STAGE_VERTEX_AND_FRAGMENT
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