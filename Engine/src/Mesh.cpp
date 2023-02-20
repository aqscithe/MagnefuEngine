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
            ParseMaterial(matFile, ss);

            for (auto& matData : ss)
            {
                Material<int> material = CreateMaterial(matData.StrData, matData.SubMatName);
                materialList.emplace_back(matFile, matData.SubMatName, material);
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

Material<int> Mesh::CreateMaterial(const std::string& matData, const std::string& matName)
{
    using String = std::string;

    std::ifstream stream(matData);

    String line;
    Material<int> material;


    while (std::getline(stream, line))
    {
        // this looks horrendous
        // can i make this work with a switch?


        size_t pos;
        if ((pos = line.find("Ns ")) != String::npos)
            material.Ns = std::stof(line.substr(pos + 3));

        else if ((pos = line.find("Ni ")) != String::npos)
            material.Ni = std::stof(line.substr(pos + 3));

        else if ((pos = line.find("d ")) != String::npos)
            material.Opacity = std::stof(line.substr(pos + 2));

        else if ((pos = line.find("Tr ")) != String::npos && material.Opacity < 0.f)
            material.Opacity = 1.f - std::stof(line.substr(pos + 3));

        else if ((pos = line.find("Tf ")) != String::npos)
            material.Tf = Maths::StrtoVec3(line.substr(pos + 3));

        else if ((pos = line.find("illum ")) != String::npos)
            material.Illum = std::stoi(line.substr(pos + 6));

        else if ((pos = line.find("Ka ")) != String::npos)
            material.Ka = Maths::StrtoVec3(line.substr(pos + 3));

        else if ((pos = line.find("Kd ")) != String::npos)
            material.Kd = Maths::StrtoVec3(line.substr(pos + 3));

        else if ((pos = line.find("Ks ")) != String::npos)
            material.Ks = Maths::StrtoVec3(line.substr(pos + 3));

        else if ((pos = line.find("Ke ")) != String::npos)
            material.Ke = Maths::StrtoVec3(line.substr(pos + 3));

        else if ((pos = line.find("map_ ")) != String::npos)
        {
            TextureType type;
            if (line.find("Ka")) type = TextureType::AMBIENT;
            else if (line.find("Kd")) type = TextureType::DIFFUSE;
            else if (line.find("Ks")) type = TextureType::SPECULAR;
            else if (line.find("Ke")) type = TextureType::EMISSIVE;
            else if (line.find("bump")) type = TextureType::BUMP;

            String file = line.substr(pos + 7); 
            String filepath = "res/textures/" + matName + "/" + file;
            if (!m_Textures.contains(file))
                m_Textures[file] = TextureData{ type, std::make_shared<Texture>(filepath, true) };
            else if (m_Textures[file].Type != type)
            {
                String fileSuffixed = file + "_" + std::to_string((int)type);
                m_Textures[fileSuffixed] = TextureData{ type, m_Textures[file].Texture };
            }

            // when do I set material.Ambient, diffuse, and specular?
            // after generating the texture image
                
        }
    }

    return material;
}

