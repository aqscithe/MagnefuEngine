#pragma once

#include <unordered_map>

#include "Globals.h"
#include "Vectors.h"
#include "Matrices.h"



struct ShaderProgramSource
{
	String VertexSource;
	String FragmentSource;
};


class Shader
{
private:
	String m_filepath;
	unsigned int m_RendererID;
	std::unordered_map<String, int> m_uniformLocationAndNameMap;


public:
	Shader(const String& filepath);
	~Shader();

	void Bind()   const;
	void Unbind() const;

	void SetUniformMatrix4fv(const String& name, const Maths::mat4& value);
	void SetUniform4fv(const String& name, const Maths::vec4& value);
	void SetUniform3fv(const String& name, const Maths::vec3& value);
	void SetUniform1f(const String& name, const float value);
	void SetUniform1i(const String& name, const int value);
	void SetUniform1iv(const String& name, const int* value);

private:
	ShaderProgramSource ParseShader(const String& filepath);

	void CreateShader(const String& vShaderSource, const String& fShaderSource);
	unsigned int CompileShader(unsigned int type, const String& source);

	int GetUniformLocation(const String& name);

};