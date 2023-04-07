 #pragma once

// NOTE: Instead of having large shaders with a bunch of if checks to determine
// what functions to run, the code should analyze the materials(a shader + uniforms)
// and create a shader based on the required functionality.

#include "Magnefu/Renderer/Shader.h"
#include "CacheableResource.h"
#include "Magnefu/Tests/Test.h"

#include <string>


namespace Magnefu
{
	enum class ShadingTechnique
	{
		PHONG
	};


	struct ShaderProgramSource
	{
		String VertexSource;
		String FragmentSource;
	};


	class OpenGLShader : public Shader, public CacheableResource
	{
	public:
		OpenGLShader(const String& filepath);
		~OpenGLShader();

		void Bind() const override ;
		void Unbind() const override ;

		void SetUniformMatrix4fv(const String& name, const Maths::mat4& value) override;
		void SetUniform4fv(const String& name, const Maths::vec4& value) override;
		void SetUniform3fv(const String& name, const Maths::vec3& value) override;
		void SetUniform1f(const String& name, const float value) override;
		void SetUniform1i(const String& name, const int value) override;
		void SetUniform1i(const String& name, const bool value) override;
		void SetUniform1iv(const String& name, const int* value) override;
		void SetUniform1ui(const String& name, const unsigned int value) override;


		void UploadUniforms(const Ref<SceneData>&  data) override;
		const String& GetFilepath() const override { return m_Filepath; }

		void ClearCache();

	private:
		ShaderProgramSource ParseShader(const String& filepath);

		void CreateShader(const String& vShaderSource, const String& fShaderSource);
		unsigned int CompileShader(unsigned int type, const String& source);

		int GetUniformLocation(const String& name);

	private:
		String m_Filepath;
		unsigned int m_RendererID;
		std::unordered_map<String, int> m_uniformLocationAndNameMap;


	};
}

