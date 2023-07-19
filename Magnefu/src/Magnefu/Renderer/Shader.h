#pragma once


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

	struct SpecializationInfo
	{

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