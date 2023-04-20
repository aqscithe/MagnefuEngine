#include "mfpch.h"

#include "OpenGLMaterial.h"
#include "Magnefu/Application.h"

#include "imgui.h"

namespace Magnefu
{
    std::unordered_map<TextureType, String> TextureTypeNameMap = {
        {TextureType::AMBIENT, "Ambient"},
        {TextureType::DIFFUSE, "Diffuse"},
        {TextureType::BUMP, "Bump"},
        {TextureType::ROUGHNESS, "Roughness"},
        {TextureType::SPECULAR, "Specular"},
        {TextureType::EMISSIVE, "Emissive"},
        {TextureType::METALLIC, "Metallic"}
    };

    

    OpenGLMaterial::OpenGLMaterial(const String& shaderFile)
    {
        m_Shader = Shader::Create(shaderFile);

        InitUniforms();

        m_ID = static_cast<uint32_t>(Application::Get().GetResourceCache().size<OpenGLMaterial>());
    }

    void OpenGLMaterial::InitUniforms()
    {
        auto& uniformData = m_Shader->GetUniforms();

        for (auto& data : uniformData)
        {
            if (data.second == "mat4")
                SetUniformValue(data.first, Maths::mat4());
            else if (data.second == "vec3")
                SetUniformValue(data.first, Maths::vec3());
            if (data.second == "sampler2D" || data.second == "int")
                SetUniformValue(data.first, int());
            else if (data.second == "float")
                SetUniformValue(data.first, float());
            else if (data.second == "bool")
                SetUniformValue(data.first, true);
            else if (data.second == "vec2")
                SetUniformValue(data.first, Maths::vec2());
            else if (data.second == "vec4")
                SetUniformValue(data.first, Maths::vec4());
            else if (data.second == "int*")
                SetUniformValue(data.first, (int*)nullptr);

        }
        uniformData.clear();

        m_Shader->Bind();

        // SET DEFAULT TEXTURES
        m_TextureMap[TextureType::DIFFUSE] = Texture::Create();
        for (auto& texture : m_TextureMap)
        {
            String uniformName = "u_" + TextureTypeNameMap[texture.first] + "Texture";
            m_Shader->SetUniform1i(uniformName, (int)texture.second->GetSlot());
        }

        // TODO: Textures do not change often. Any way to only send the texture uniform if there was an update
        // to the selected texture slot?
    }

    /*void OpenGLMaterial::InitRenderData(const Ref<SceneData>& renderData)
    {
        m_RenderData = renderData;
    }*/

    void OpenGLMaterial::OnImGuiRender()
    {
        ImGui::Begin("Materials");
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("Materials", tab_bar_flags))
        {
            if (ImGui::BeginTabItem(std::to_string(m_ID).c_str()))
            {
                ImGui::Text("ID: %d", m_ID);
                ImGui::SeparatorText("Shader");
                if (ImGui::Button("Recompile"))
                {
                    m_Shader->Recompile();
                    InitUniforms();
                }
                m_Shader->OnImGuiRender();
                ImGui::SeparatorText("Textures");
                if (!m_TextureMap.empty())
                {
                    auto& Textures = m_TextureMap;
                    if (ImGui::CollapsingHeader("Diffuse"))
                        Textures[TextureType::DIFFUSE]->OnImGuiRender();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }

    void OpenGLMaterial::Bind()
    {
        m_Shader->Bind();

        // SET SHADER UNIFORMS
        m_Shader->UploadUniforms(m_Uniforms);

        // BIND TEXTURES
        for (auto& texture : m_TextureMap)
            texture.second->Bind();
    }

    void OpenGLMaterial::Unbind()
    {
        m_Shader->Unbind();

        for (auto& texture : m_TextureMap)
            texture.second->Unbind();
    }

    void OpenGLMaterial::SetUniformValueImpl(const std::string& name, const bool& value)
    {
        if (m_Uniforms.find(name) == m_Uniforms.end())
        {
            Uniform uniform = Uniform(name, typeid(value));
            m_Uniforms.emplace(name, std::move(uniform));
        }
        m_Uniforms[name].SetValue<bool>(value);
    }

    void OpenGLMaterial::SetUniformValueImpl(const std::string& name, const int& value)
    {
        if (m_Uniforms.find(name) == m_Uniforms.end())
        {
            Uniform uniform = Uniform(name, typeid(value));
            m_Uniforms.emplace(name, std::move(uniform) );
        }

        m_Uniforms[name].SetValue(value);
    }

    void OpenGLMaterial::SetUniformValueImpl(const std::string& name, const int* value)
    {
        if (m_Uniforms.find(name) == m_Uniforms.end())
        {
            Uniform uniform = Uniform(name, typeid(value));
            m_Uniforms.emplace(name, std::move(uniform));
        }

        m_Uniforms[name].SetValue(value);
    }

    void OpenGLMaterial::SetUniformValueImpl(const std::string& name, const float& value)
    {
        if (m_Uniforms.find(name) == m_Uniforms.end())
        {
            Uniform uniform = Uniform(name, typeid(value));
            m_Uniforms.emplace(name, std::move(uniform));
        }

        m_Uniforms[name].SetValue(value);
    }

    void OpenGLMaterial::SetUniformValueImpl(const std::string& name, const Maths::vec2& value)
    {
        if (m_Uniforms.find(name) == m_Uniforms.end())
        {
            Uniform uniform = Uniform(name, typeid(value));
            m_Uniforms.emplace(name, std::move(uniform));
        }

        m_Uniforms[name].SetValue(value);
    }

    void OpenGLMaterial::SetUniformValueImpl(const std::string& name, const Maths::vec3& value)
    {
        if (m_Uniforms.find(name) == m_Uniforms.end())
        {
            Uniform uniform = Uniform(name, typeid(value));
            m_Uniforms.emplace(name, std::move(uniform));
        }

        m_Uniforms[name].SetValue(value);
    }

    void OpenGLMaterial::SetUniformValueImpl(const std::string& name, const Maths::vec4& value)
    {
        if (m_Uniforms.find(name) == m_Uniforms.end())
        {
            Uniform uniform = Uniform(name, typeid(value));
            m_Uniforms.emplace(name, std::move(uniform));
        }

        m_Uniforms[name].SetValue(value);
    }

    void OpenGLMaterial::SetUniformValueImpl(const std::string& name, const Maths::mat4& value)
    {
        if (m_Uniforms.find(name) == m_Uniforms.end())
        {
            Uniform uniform = Uniform(name, typeid(value));
            m_Uniforms.emplace(name, std::move(uniform));
        }

        m_Uniforms[name].SetValue(value);
    }
}

   /* Material::Material(const TextureProps& textures, const std::string& name)
    {
        Name = name;
        ID = Application::Get().GetResourceCache().size<Material>();
        Library = "NONE";

        NullifyTextures();

        if (textures.Ambient)
            Ambient = textures.Ambient;
        if (textures.Diffuse)
            Diffuse = textures.Diffuse;
        if (textures.Emissive)
            Emissive = textures.Emissive;
        if (textures.Specular)
            Specular = textures.Specular;
        if (textures.Roughness)
            Roughness = textures.Roughness;
        if (textures.Metallic)
            Metallic = textures.Metallic;

        Ka = Maths::vec3(0.1f);
        Kd = Maths::vec3(0.8f);
        Ks = Maths::vec3(0.1f);
        Ke = Maths::vec3(0.f);
        Ni = 1.5;
        Ns = 10.f;
        Opacity = 1.f;
        Tf = Maths::vec3(0.f);
        Illum = 0;

    }

	Material::Material(const std::string& matFile, const std::string& matData, const std::string& matName)
    {
        Application& app = Application::Get();
        ResourceCache& cache = app.GetResourceCache();

        NullifyTextures();

        ID = cache.size<Material>();
        Name = matName;
        Library = matFile;

        using String = std::string;

        std::stringstream stream(matData);

        String line;

        while (std::getline(stream, line))
        {
            size_t pos;

            if ((pos = line.find("Ns ")) == 1 || (pos = line.find("Ns ")) == 0)
                Ns = std::stof(line.substr(pos + 3));

            else if ((pos = line.find("Ni ")) == 1 || (pos = line.find("Ni ")) == 0)
                Ni = std::stof(line.substr(pos + 3));

            else if ((pos = line.find("d ")) == 1 || (pos = line.find("d ")) == 0)
                Opacity = std::stof(line.substr(pos + 2));

            else if (((pos = line.find("Tr ")) == 1 || (pos = line.find("Tr ")) == 0) && Opacity < 0.f)
                Opacity = 1.f - std::stof(line.substr(pos + 3));

            else if ((pos = line.find("Tf ")) == 1 || (pos = line.find("Tf ")) == 0)
                Tf = Maths::StrtoVec3(line.substr(pos + 3));

            else if ((pos = line.find("illum ")) == 1 || (pos = line.find("illum ")) == 0)
                Illum = std::stoi(line.substr(pos + 6));

            else if ((pos = line.find("Ka ")) == 1 || (pos = line.find("Ka ")) == 0)
                Ka = Maths::StrtoVec3(line.substr(pos + 3));

            else if ((pos = line.find("Kd ")) == 1 || (pos = line.find("Kd ")) == 0)
                Kd = Maths::StrtoVec3(line.substr(pos + 3));

            else if ((pos = line.find("Ks ")) == 1 || (pos = line.find("Ks ")) == 0)
                Ks = Maths::StrtoVec3(line.substr(pos + 3));

            else if ((pos = line.find("Ke ")) == 1 || (pos = line.find("Ke ")) == 0)
                Ke = Maths::StrtoVec3(line.substr(pos + 3));

            else if ((pos = line.find("map_")) != String::npos)
            {
                std::stringstream ss;
                ss.str(line);
                String del;

                String file = line.substr(pos + 7);

                if (line.find("\\") != String::npos)
                {
                    while (std::getline(ss, del, '\\'))
                    {
                        file = del;
                    }
                }

                TextureType type;
                if (line.find("Ka") != String::npos) type = TextureType::AMBIENT;
                else if (line.find("Kd") != String::npos) type = TextureType::DIFFUSE;
                else if (line.find("Ks") != String::npos) type = TextureType::SPECULAR;
                else if (line.find("Ke") != String::npos) type = TextureType::EMISSIVE;
                else if (line.find("Ns") != String::npos) type = TextureType::ROUGHNESS;
                else if (line.find("refl") != String::npos)
                {
                    type = TextureType::METALLIC;
                    file = line.substr(pos + 9);
                }
                else if (line.find("bump") != String::npos || line.find("Bump") != String::npos)
                {
                    type = TextureType::BUMP;
                    file = line.substr(pos + 9);

                    if (line.find("\\") != String::npos)
                    {
                        while (std::getline(ss, del, '\\'))
                        {
                            file = del;
                        }
                    }
                }


                String filepath = "res/textures/" + matFile.substr(0, matFile.find(".")) + "/" + file;
                bool isAsync = true;
                switch (type)
                {
                case NONE:
                    break;
                case AMBIENT:
                    Ambient = cache.RequestResource<Texture>(filepath, isAsync);
                    break;
                case DIFFUSE:
                    Diffuse = cache.RequestResource<Texture>(filepath, isAsync);
                    break;
                case SPECULAR:
                    Specular = cache.RequestResource<Texture>(filepath, isAsync);
                    break;
                case EMISSIVE:
                    Emissive = cache.RequestResource<Texture>(filepath, isAsync);
                    break;
                case BUMP:
                    break;
                case ROUGHNESS:
                    Roughness = cache.RequestResource<Texture>(filepath, isAsync);
                    break;
                case METALLIC:
                    Metallic = cache.RequestResource<Texture>(filepath, isAsync);
                default:
                    break;
                }

            }
        }
    }



    void Material::NullifyTextures()
    {
        Ambient = nullptr;
        Diffuse = nullptr;
        Specular = nullptr;
        Emissive = nullptr;
        Roughness = nullptr;
        Metallic = nullptr;
    }*/

