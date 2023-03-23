#include "mfpch.h"

#include "Material.h"
#include "Magnefu/Application.h"


namespace Magnefu 
{
    TextureProps::TextureProps()
    {
        Ambient = nullptr;
        Diffuse = nullptr;
        Specular = nullptr;
        Emissive = nullptr;
        Roughness = nullptr;
        Metallic = nullptr;
    }

    Material::Material(const TextureProps& textures, const std::string& name)
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

    void Material::Bind()
    {
        if (Ambient)
            Ambient->Bind();
        if (Diffuse)
            Diffuse->Bind();
        if (Emissive)
            Emissive->Bind();
        if (Specular)
            Specular->Bind();
        if (Roughness)
            Roughness->Bind();
        if (Metallic)
            Metallic->Bind();
    }

    void Material::Unbind()
    {
        if (Ambient)
            Ambient->Unbind();
        if (Diffuse)
            Diffuse->Unbind();
        if (Emissive)
            Emissive->Unbind();
        if (Specular)
            Specular->Unbind();
        if (Roughness)
            Roughness->Unbind();
        if (Metallic)
            Metallic->Unbind();
    }

    void Material::NullifyTextures()
    {
        Ambient = nullptr;
        Diffuse = nullptr;
        Specular = nullptr;
        Emissive = nullptr;
        Roughness = nullptr;
        Metallic = nullptr;
    }
}
