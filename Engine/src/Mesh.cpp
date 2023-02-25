#include "Mesh.h"





Mesh::Mesh(const std::string& filepath, std::vector<MaterialData>& materialList)
    : m_Filepath(filepath)
{
    std::ifstream stream(m_Filepath);
    std::string line;
    
    std::string mtl;
    std::string matFile;

    std::vector<SubMaterialStream> ss;
    while (std::getline(stream, line))
    {
        if (line.find("mtllib ") == 0)
        {
            matFile = line.substr(7);
            std::string filepath = "res/materials/" + matFile;
            ParseMaterial(filepath, ss);

            for (auto& matData : ss)
            {
                materialList.emplace_back(matFile, matData.SubMatName, CreateMaterial(matFile, matData.StrData, matData.SubMatName));
                m_MaterialIndices.emplace(matData.SubMatName, materialList.size() - 1);
            }
        }
        else
        {
            if (line.find("v ") == 0)
                m_Positions.emplace_back(GetVertexData(line, 3));
            else if (line.find("vn ") == 0)
                m_Normals.emplace_back(GetVertexData(line, 3));
            else if (line.find("vt ") == 0)
                m_TexCoords.emplace_back(GetVertexData(line, 2).uv);
            else if (line.find("f ") == 0)
                m_Faces.emplace_back(GetFaceData(line), m_MaterialIndices[mtl]);
            else if (line.find("usemtl ") == 0)
                mtl = line.substr(7);
        }
    }

    std::cout << "Vertex Postions: " << m_Positions.size() << " | " << "Vertex Normals: " << m_Normals.size() << " | " << "Texture Coords: " <<
        m_TexCoords.size() << " | " << "Faces: " << m_Faces.size() << std::endl;
}

Maths::vec3 Mesh::GetVertexData(std::string& line, int elementCount)
{
    std::string vertexLine = line.substr(3);
    Maths::vec3 v;

    std::stringstream ss;
    ss.str(vertexLine);

    int index = 0;
    std::string del;
    while (std::getline(ss, del, ' '))
    {
        v.e[index] = std::stof(del);
        index++;
    }
    return v;
}

std::array<Maths::vec3i, 4> Mesh::GetFaceData(std::string& line)
{
    std::string faceLine = line.substr(2);
    // maybe i shoudl make a 3x4 matrix
    int faceData[12];

    std::stringstream ss;
    ss.str(faceLine);

    int vertexIndex = 0;
    std::string del;
    while (std::getline(ss, del, ' '))
    {
        int elementIndex = 0;
        std::stringstream ssB(del);
        std::string delB;
        while (std::getline(ssB, delB, '/'))
        {
            // subtract 1 b/c face indices start at 1 instead of 0 in obj file
            faceData[vertexIndex * 3 + elementIndex] = std::stoi(delB) - 1;
            elementIndex++;
        }
        vertexIndex++;
    }

    return { 
        Maths::vec3i(faceData[0], faceData[1], faceData[2]), 
        Maths::vec3i(faceData[3], faceData[4], faceData[5]), 
        Maths::vec3i(faceData[6], faceData[7], faceData[8]), 
        Maths::vec3i(faceData[9], faceData[10], faceData[11])
    };

}

void Mesh::ParseMaterial(const std::string& filepath, std::vector<SubMaterialStream>& ss)
{
    using String = std::string;

    std::ifstream stream(filepath);

    String line;
    String newmtl;
    std::stringstream matProperties;
    while (std::getline(stream, line))
    {
        if (line.find("newmtl ") != String::npos)
        {
            if (!newmtl.empty())
            {
                ss.push_back(SubMaterialStream{ newmtl, matProperties.str() });
                matProperties.clear();
            }
            newmtl = line.substr(7);
        }
        else if (!newmtl.empty())
            matProperties << line << '\n';
    }
    ss.push_back(SubMaterialStream{ newmtl, matProperties.str() });
}

Material<std::shared_ptr<Texture>> Mesh::CreateMaterial(const std::string& matFile, const std::string& matData, const std::string& matName)
{
    using String = std::string;

    std::stringstream stream(matData);

    String line;


    float Ns = 32.f, Ni = 1.f, Opacity = 1.f;
    Maths::vec3 Tf{0.f}, Ka{ 0.f }, Kd{ 0.f }, Ke{ 0.f }, Ks{ 0.f };
    int Illum = 0;
    std::shared_ptr<Texture> Ambient = nullptr;
    std::shared_ptr<Texture> Diffuse = nullptr;
    std::shared_ptr<Texture> Specular = nullptr;

    while (std::getline(stream, line))
    {
        // this looks horrendous
        // can i make this work with a switch?


        size_t pos;
        if ((pos = line.find("Ns ")) == 1)
        {
            std::cout << std::stof(line.substr(pos + 3)) << std::endl;
            Ns = std::stof(line.substr(pos + 3));
        }

        else if ((pos = line.find("Ni ")) == 1)
            Ni = std::stof(line.substr(pos + 3));

        else if ((pos = line.find("d ")) == 1)
            Opacity = std::stof(line.substr(pos + 2));

        else if ((pos = line.find("Tr ")) == 1 && Opacity < 0.f)
            Opacity = 1.f - std::stof(line.substr(pos + 3));

        else if ((pos = line.find("Tf ")) == 1)
            Tf = Maths::StrtoVec3(line.substr(pos + 3));

        else if ((pos = line.find("illum ")) == 1)
            Illum = std::stoi(line.substr(pos + 6));

        else if ((pos = line.find("Ka ")) == 1)
            Ka = Maths::StrtoVec3(line.substr(pos + 3));

        else if ((pos = line.find("Kd ")) == 1)
            Kd = Maths::StrtoVec3(line.substr(pos + 3));

        else if ((pos = line.find("Ks ")) == 1)
            Ks = Maths::StrtoVec3(line.substr(pos + 3));

        else if ((pos = line.find("Ke ")) == 1)
            Ke = Maths::StrtoVec3(line.substr(pos + 3));

        else if ((pos = line.find("map_")) == 1)
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
            if (line.find("map_Ka") == 1) type = TextureType::AMBIENT;
            else if (line.find("map_Kd") == 1) type = TextureType::DIFFUSE;
            else if (line.find("map_Ks") == 1) type = TextureType::SPECULAR;
            else if (line.find("map_Ke") == 1) type = TextureType::EMISSIVE;
            else if (line.find("map_bump") == 1)
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

            // i should use a struct for the key!!! {string, TextureType}
            if (!m_TextureCache.contains(file))
                m_TextureCache.emplace(file, std::make_shared<Texture>(filepath, true));
            m_Textures.emplace_back(file, type);
                

            switch (type)
            {
            case NONE:
                break;
            case AMBIENT:
                Ambient = m_TextureCache[file];
                break;
            case DIFFUSE:
                Diffuse = m_TextureCache[file];
                break;
            case SPECULAR:
                Specular = m_TextureCache[file];
                break;
            case EMISSIVE:
                break;
            case BUMP:
                break;
            default:
                break;
            }
                
        }
    }

    return {
        false, false, 99,
        Ambient, Diffuse, Specular,
        Ka, Kd, Ks, Ke,
        Ni, Ns, Opacity, Tf, Illum
    };
}

