#include "mfpch.h"

#include "Shader.h"
#include <GLAD/glad.h>

#include <fstream>



Shader::Shader(const String& filepath)
	: m_filepath(filepath), m_RendererID(0)
{
    ShaderProgramSource source = ParseShader(m_filepath);
    CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::~Shader()
{
    glDeleteProgram(m_RendererID);
}  


ShaderProgramSource Shader::ParseShader(const String& filepath)
{

    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };

    String line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (std::getline(stream, line))
    {
        if (line.find("#shader") != String::npos)
        {
            if (line.find("vertex") != String::npos)
            {
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != String::npos)
            {
                type = ShaderType::FRAGMENT;
            }
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }

    return { ss[0].str(), ss[1].str() };

}

void Shader::CreateShader(const String& vShaderSource, const String& fShaderSource)
{
    unsigned int vShader = CompileShader(GL_VERTEX_SHADER, vShaderSource);
    unsigned int fShader = CompileShader(GL_FRAGMENT_SHADER, fShaderSource);

    m_RendererID = glCreateProgram();

    glAttachShader(m_RendererID, vShader);
    glAttachShader(m_RendererID, fShader);
    glLinkProgram(m_RendererID);
    glValidateProgram(m_RendererID);

    glDeleteShader(vShader);
    glDeleteShader(fShader);
}


unsigned int Shader::CompileShader(unsigned int type, const String& source)
{
    unsigned int shaderObjID = glCreateShader(type);
    const char* sourceStr = source.c_str();

    glShaderSource(shaderObjID, 1, &sourceStr, nullptr);
    glCompileShader(shaderObjID);

    int success;
    glGetShaderiv(shaderObjID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        int logLength;
        glGetShaderiv(shaderObjID, GL_INFO_LOG_LENGTH, &logLength);

        char* infoLog = (char*)malloc(sizeof(char) * logLength);
        glGetShaderInfoLog(shaderObjID, logLength, nullptr, infoLog);

        MF_CORE_ERROR("Failed to compile shader: {}", type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");
        MF_CORE_ERROR(infoLog);
        return 0;
    }

    return shaderObjID;
}


void Shader::Bind() const
{
    glUseProgram(m_RendererID);
}

void Shader::Unbind() const
{
    glUseProgram(0);
}

int Shader::GetUniformLocation(const String& name)
{
    if (m_uniformLocationAndNameMap.find(name) != m_uniformLocationAndNameMap.end())
        return m_uniformLocationAndNameMap[name];

    int uniformLocation = glGetUniformLocation(m_RendererID, name.c_str());

    if (uniformLocation == -1)
        MF_CORE_WARN("Warning: uniform '{}' doesn't exist!", name);

    m_uniformLocationAndNameMap.insert({ name, uniformLocation });
    return uniformLocation;
}


void Shader::SetUniformMatrix4fv(const String& name, const Maths::mat4& value)
{
	glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, (const GLfloat*)value.e);
}

void Shader::SetUniform4fv(const String& name, const Maths::vec4& value)
{
	glUniform4fv(GetUniformLocation(name), 1, (const GLfloat*)value.e);
}

void Shader::SetUniform3fv(const String& name, const Maths::vec3& value)
{
	glUniform3fv(GetUniformLocation(name), 1, (const GLfloat*)value.e);
}

void Shader::SetUniform1f(const String& name, const float value)
{
	glUniform1f(GetUniformLocation(name), (GLfloat)value);
}

void Shader::SetUniform1i(const String& name, const int value)
{
	glUniform1i(GetUniformLocation(name), (GLint)value);
}

void Shader::SetUniform1i(const String& name, const bool value)
{
    glUniform1i(GetUniformLocation(name), (GLboolean)value);
}

void Shader::SetUniform1iv(const String& name, const int* value)
{
    glUniform1iv(GetUniformLocation(name), 2, (const GLint*)value);
}

void Shader::SetUniform1ui(const String& name, const unsigned int value)
{
    glUniform1ui(GetUniformLocation(name), (GLuint)value);
}
