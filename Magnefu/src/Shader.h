 #pragma once

// NOTE: Instead of having large shaders with a bunch of if checks to determine
// what functions to run, the code should analyze the materials(a shader + uniforms)
// and create a shader based on the required functionality.


#include "CacheableResource.h"

#include <string>

using String = std::string;

enum class ShadingTechnique
{
	PHONG
};


struct ShaderProgramSource
{
	String VertexSource;
	String FragmentSource;
};


class Shader : public CacheableResource
{
public:
	Shader() = default;
	Shader(const String& filepath);
	Shader(const Shader&) = delete;
	~Shader();

	void Bind()   const;
	void Unbind() const;

	void SetUniformMatrix4fv(const String& name, const Maths::mat4& value);
	void SetUniform4fv(const String& name, const Maths::vec4& value);
	void SetUniform3fv(const String& name, const Maths::vec3& value);
	void SetUniform1f(const String& name, const float value);
	void SetUniform1i(const String& name, const int value);
	void SetUniform1i(const String& name, const bool value);
	void SetUniform1iv(const String& name, const int* value);
	void SetUniform1ui(const String& name, const unsigned int value);

	void ClearCache();

private:
	ShaderProgramSource ParseShader(const String& filepath);

	void CreateShader(const String& vShaderSource, const String& fShaderSource);
	unsigned int CompileShader(unsigned int type, const String& source);

	int GetUniformLocation(const String& name);

private:
	String m_filepath;
	unsigned int m_RendererID;
	std::unordered_map<String, int> m_uniformLocationAndNameMap;

};