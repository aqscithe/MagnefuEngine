#include "mfpch.h"

#include "OpenGLShader.h"
#include <GLAD/glad.h>

#include <fstream>
#include "imgui.h"

namespace Magnefu
{
    OpenGLShader::OpenGLShader(const String& filepath)
        : m_Filepath(filepath), m_RendererID(0)
    {

        ShaderProgramSource source = ParseShader(m_Filepath);
        CreateShader(source.VertexSource, source.FragmentSource);

        m_Recompile = false;
    }

    OpenGLShader::~OpenGLShader()
    {
        glDeleteProgram(m_RendererID);
    }


    ShaderProgramSource OpenGLShader::ParseShader(const String& filepath)
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
        m_Uniforms.clear();
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

            if (line.find("uniform") != String::npos && line.find("uniform") == 0)
            {
                String type;
                String name;
                size_t pos = line.find(" ");
                if (pos != String::npos)
                {
                    pos++;
                    size_t end_pos = line.find(" ", pos);
                    if (end_pos != String::npos)
                        type = line.substr(pos, end_pos - pos);
                    else
                        type = line.substr(pos);

                    pos = end_pos + 1;
                    end_pos = line.find(" ", pos);
                    if (end_pos != String::npos)
                        name = line.substr(pos, end_pos - pos - 1);
                    else
                    {
                        name = line.substr(pos);
                        name = name.substr(0, name.length() - 1);
                    }
                        
                    MF_CORE_ASSERT(type.size() > 0 && name.size() > 0, "Shader uniform name or type not found");
                    m_Uniforms.emplace_back(name, type);
                    
                }
            }
        }
        return { ss[0].str(), ss[1].str() };
    }

    void OpenGLShader::CreateShader(const String& vShaderSource, const String& fShaderSource)
    {
        unsigned int vShader = CompileShader(GL_VERTEX_SHADER, vShaderSource);
        unsigned int fShader = CompileShader(GL_FRAGMENT_SHADER, fShaderSource);

        // essentially i want to check if this is a new or existing shader.
        // i don't want to create a new program if im just recompiling the shader.
        m_RendererID = glCreateProgram();

        glAttachShader(m_RendererID, vShader);
        glAttachShader(m_RendererID, fShader);

        glLinkProgram(m_RendererID);
        GLint linkSuccess;
        glGetProgramiv(m_RendererID, GL_LINK_STATUS, &linkSuccess);
        if (!linkSuccess)
        {
            GLint logLength = 96;
            std::vector<GLchar> infoLog(logLength);
            glGetProgramInfoLog(m_RendererID, logLength, &logLength, infoLog.data());
            MF_CORE_ERROR("SHADER -- Failed to link shader program: ");
            MF_CORE_ERROR("\t{0}", infoLog.data());
        }

        glValidateProgram(m_RendererID);

        glDeleteShader(vShader);
        glDeleteShader(fShader);
    }


    unsigned int OpenGLShader::CompileShader(unsigned int type, const String& source)
    {
        unsigned int shaderObjID = glCreateShader(type);
        const char* sourceStr = source.c_str();

        glShaderSource(shaderObjID, 1, &sourceStr, nullptr);
        glCompileShader(shaderObjID);

        GLint success;
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

    void OpenGLShader::Recompile()
    {
        glDeleteProgram(m_RendererID);
        ShaderProgramSource source = ParseShader(m_Filepath);
        CreateShader(source.VertexSource, source.FragmentSource);
    }


    void OpenGLShader::Bind() const
    {
        glUseProgram(m_RendererID);
    }

    void OpenGLShader::Unbind() const
    {
        glUseProgram(0);
    }

    int OpenGLShader::GetUniformLocation(const String& name)
    {
        if (m_uniformLocationAndNameMap.find(name) != m_uniformLocationAndNameMap.end())
            return m_uniformLocationAndNameMap[name];

        int uniformLocation = glGetUniformLocation(m_RendererID, name.c_str());

        if (uniformLocation == -1)
            MF_CORE_WARN("Warning: uniform '{}' doesn't exist!", name);

        m_uniformLocationAndNameMap.insert({ name, uniformLocation });
        return uniformLocation;
    }


    void OpenGLShader::SetUniformMatrix4fv(const String& name, const Maths::mat4& value)
    {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, (const GLfloat*)value.e);
    }

    void OpenGLShader::SetUniform4fv(const String& name, const Maths::vec4& value)
    {
        glUniform4fv(GetUniformLocation(name), 1, (const GLfloat*)value.e);
    }

    void OpenGLShader::SetUniform3fv(const String& name, const Maths::vec3& value)
    {
        glUniform3fv(GetUniformLocation(name), 1, (const GLfloat*)value.e);
    }

    void OpenGLShader::SetUniform1f(const String& name, const float value)
    {
        glUniform1f(GetUniformLocation(name), (GLfloat)value);
    }

    void OpenGLShader::SetUniform1i(const String& name, const int value)
    {
        glUniform1i(GetUniformLocation(name), (GLint)value);
    }

    void OpenGLShader::SetUniform1i(const String& name, const bool value)
    {
        glUniform1i(GetUniformLocation(name), (GLboolean)value);
    }

    void OpenGLShader::SetUniform1iv(const String& name, const int* value)
    {
        glUniform1iv(GetUniformLocation(name), 2, (const GLint*)value);
    }

    void OpenGLShader::SetUniform1ui(const String& name, const unsigned int value)
    {
        glUniform1ui(GetUniformLocation(name), (GLuint)value);
    }

    void OpenGLShader::UploadUniforms(const Ref<SceneData>& data)
    {
        if (!data)
            return;

        for (auto& Mat4 : data->Mat4)
            SetUniformMatrix4fv(Mat4.first, Mat4.second);

        for (auto& Vec4 : data->Vec4)
            SetUniform4fv(Vec4.first, Vec4.second);

        for (auto& Vec3 : data->Vec3)
            SetUniform3fv(Vec3.first, Vec3.second);

        for (auto& Float : data->Float)
            SetUniform1f(Float.first, Float.second);

        for (auto& Int : data->Int)
            SetUniform1i(Int.first, Int.second);

        for (auto& IntPtr : data->IntPtr)
            SetUniform1iv(IntPtr.first, IntPtr.second);

        for (auto& Bool : data->Bool)
            SetUniform1i(Bool.first, Bool.second);

        for (auto& UInt32 : data->UInt32)
            SetUniform1ui(UInt32.first, UInt32.second);

    }

    void OpenGLShader::OnImGuiRender()
    {
        ImGui::Text("Shader: %s", m_Filepath.c_str());
        if (ImGui::Button("Recompile"))
        {
            Recompile();
        }
    }

    void OpenGLShader::ClearCache()
    {
        m_uniformLocationAndNameMap.clear();
    }
}


