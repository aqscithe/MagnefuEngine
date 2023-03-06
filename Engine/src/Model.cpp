#include "Model.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"



Model::Model(std::string& filepath, Cache& matCache, std::mutex& mutex)
{
    m_Filepath = "res/meshes/" + filepath;
    std::ifstream stream(m_Filepath);
    std::string line;

    std::string mtl;
    std::string matFile;

    std::vector<Maths::vec3> tempPositions;
    std::vector<Maths::vec3> tempNormals;
    std::vector<Maths::vec2> tempTexCoords;
    std::vector<Face> tempFaces;

    VertexCount vCount = { 0, 0, 0 };
    VertexCount vCountLast = vCount;

    std::vector<SubMaterialStream> ss;
    while (std::getline(stream, line))
    {
        if (line.find("mtllib ") == 0)
        {
            matFile = line.substr(7);
            std::string filepath = "res/materials/" + matFile;
            ParseMaterial(filepath, ss);

            
            std::lock_guard<std::mutex> guard(mutex);
            
            for (auto& matData : ss)
            {
                m_MaterialList.emplace_back(matFile, matData.SubMatName, CreateMaterial(matFile, matData.StrData, matData.SubMatName, matCache.size()));
                matCache.emplace(matData.SubMatName, matCache.size());
            }
        }
        else
        {
            // need to create a new mesh every time we finish getting data for a set of vertices
            if (line.find("v ") == 0)
            {
                if (!tempFaces.empty())
                {
                    LoadMesh(tempPositions, tempNormals, tempTexCoords, tempFaces);
                    vCountLast.normalCount += vCount.normalCount;
                    vCountLast.posCount += vCount.posCount;
                    vCountLast.uvCount += vCount.uvCount;

                    vCount = { 0,0,0 };
                }
                    
                tempPositions.emplace_back(GetVertexData(line, 3));
                vCount.posCount++;
            }
                
            else if (line.find("vn ") == 0)
            {
                tempNormals.emplace_back(GetVertexData(line, 3));
                vCount.normalCount++;
            }
            else if (line.find("vt ") == 0)
            {
                tempTexCoords.emplace_back(GetVertexData(line, 2).uv);
                vCount.uvCount++;
            }
            else if (line.find("f ") == 0)
            {
                uint32_t vertexCount;
                auto faceData = GetFaceData(line, vertexCount);
                if (m_Meshes.size() > 0)
                {
                    for (uint32_t i = 0; i < vertexCount; i++)
                    {
                        if (faceData[i].v < vCountLast.posCount)
                            int j = 1;
                        faceData[i].v -= vCountLast.posCount; //- faceData[i].v;
                        faceData[i].vn -= vCountLast.normalCount; //- faceData[i].vn;
                        faceData[i].vt -= vCountLast.uvCount; //- faceData[i].vt;
                    }
                }
                tempFaces.emplace_back(faceData, matCache[mtl], vertexCount);
            }
                
            else if (line.find("usemtl ") == 0)
                mtl = line.substr(7);
        }
    }
    // For the last mesh
    if (!tempFaces.empty())
        LoadMesh(tempPositions, tempNormals, tempTexCoords, tempFaces);

}

void Model::Draw(std::unique_ptr<Shader>& shader, Cache& textureCache, Cache& materialCache)
{

    BindTextures(textureCache);
    SetShaderUniforms(shader, materialCache);

    for (auto& mesh : m_Meshes)
        mesh->Draw(shader);

    // TRY UNBINDING TEXTURES
    //UnbindTextures(textureCache);
    for (auto& material : m_MaterialList)
    {
        if (material.MaterialProperties.Ambient)
            material.MaterialProperties.Ambient->Unbind();
        if (material.MaterialProperties.Diffuse)
            material.MaterialProperties.Diffuse->Unbind();
        if (material.MaterialProperties.Specular)
            material.MaterialProperties.Specular->Unbind();
    }
}

void Model::BindTextures(Model::Cache& textureCache)
{
    for (auto& material : m_MaterialList)
    {
        if (material.MaterialProperties.Ambient)
            material.MaterialProperties.Ambient->Bind(textureCache[material.MaterialProperties.Ambient->GetFilepath()]);
        if (material.MaterialProperties.Diffuse)
            material.MaterialProperties.Diffuse->Bind(textureCache[material.MaterialProperties.Diffuse->GetFilepath()]);
        if (material.MaterialProperties.Specular)
            material.MaterialProperties.Specular->Bind(textureCache[material.MaterialProperties.Specular->GetFilepath()]);
    }
}

void Model::SetShaderUniforms(std::unique_ptr<Shader>& shader, Cache& materialCache)
{
    for (auto& material : m_MaterialList)
    {
        std::string matLabel = "u_material[" + std::to_string(materialCache[material.SubMaterialName]) + "].";

        shader->SetUniform3fv(matLabel + "Ka", material.MaterialProperties.Ka);
        shader->SetUniform3fv(matLabel + "Kd", material.MaterialProperties.Kd);
        shader->SetUniform3fv(matLabel + "Ks", material.MaterialProperties.Ks);
        shader->SetUniform1f(matLabel + "Ns", material.MaterialProperties.Ns);
    }
}

void Model::OnImGUIRender()
{
    ImGui::Text("Model Materials - %s", m_Filepath.c_str());
    
    for (auto& material : m_MaterialList)
    {
        std::string name = "Material Name: " + material.SubMaterialName;
        if (ImGui::TreeNode(name.c_str()))
        {
            ImGui::Text("Material Lib: %s", material.MaterialLibrary.c_str());
            ImGui::Text("Material ID: %d", material.MaterialProperties.ID);

            if (material.MaterialProperties.Ambient)
                ImGui::Text("Ambient - Texture: %s | Render ID: %d", material.MaterialProperties.Ambient->GetFilepath().c_str(), material.MaterialProperties.Ambient->GetRendererID());
            if (material.MaterialProperties.Diffuse)
                ImGui::Text("Diffuse - Texture: %s | Render ID: %d", material.MaterialProperties.Diffuse->GetFilepath().c_str(), material.MaterialProperties.Diffuse->GetRendererID());
            if (material.MaterialProperties.Specular)
                ImGui::Text("Specular - Texture: %s | Render ID: %d", material.MaterialProperties.Specular->GetFilepath().c_str(), material.MaterialProperties.Specular->GetRendererID());

            ImGui::SliderFloat3("Ka", material.MaterialProperties.Ka.e, 0.f, 1.f);
            ImGui::SliderFloat3("Kd", material.MaterialProperties.Kd.e, 0.f, 1.f);
            ImGui::SliderFloat3("Ks", material.MaterialProperties.Ks.e, 0.f, 1.f);
            ImGui::SliderFloat("Ns", &material.MaterialProperties.Ns, 0.f, 128.f);
            ImGui::TreePop();
        }
            
    }

    for (auto& mesh : m_Meshes)
        mesh->OnImGUIRender();
}

void Model::LoadMesh(std::vector<Maths::vec3>& tempPositions, std::vector<Maths::vec3>& tempNormals, std::vector<Maths::vec2>& tempTexCoords, std::vector<Face>& tempFaces)
{
    MeshData meshData = {
        tempPositions, tempNormals, tempTexCoords, tempFaces
    };

    m_Meshes.emplace_back(std::make_unique<Mesh>(meshData));

    tempPositions.clear();
    tempNormals.clear();
    tempTexCoords.clear();
    tempFaces.clear();
}

void Model::Init(std::unique_ptr<Shader>& shader, Cache& textureCache, Cache& materialCache)
{
    shader->Bind();
    for (auto& mesh : m_Meshes)
        mesh->Init();
      
    // generate textures
    for (auto& material : m_MaterialList)
    {
        if (material.MaterialProperties.Ambient && !material.MaterialProperties.Ambient->GetGenerationStatus())
            material.MaterialProperties.Ambient->GenerateTexture();
        if (material.MaterialProperties.Diffuse && !material.MaterialProperties.Diffuse->GetGenerationStatus())
            material.MaterialProperties.Diffuse->GenerateTexture();
        if (material.MaterialProperties.Specular && !material.MaterialProperties.Specular->GetGenerationStatus())
            material.MaterialProperties.Specular->GenerateTexture();
    }


    // bind textures
    //int textureSlot = m_MaterialList.size();
    int textureSlot = textureCache.size();
    for (auto& material : m_MaterialList)
    {
        if (material.MaterialProperties.Ambient && !textureCache.contains(material.MaterialProperties.Ambient->GetFilepath()))
        {
            material.MaterialProperties.Ambient->Bind(textureSlot);
            textureCache[material.MaterialProperties.Ambient->GetFilepath()] = textureSlot;
            textureSlot++;    
        }
               
        if (material.MaterialProperties.Diffuse && !textureCache.contains(material.MaterialProperties.Diffuse->GetFilepath()))
        {
            material.MaterialProperties.Diffuse->Bind(textureSlot);
            textureCache[material.MaterialProperties.Diffuse->GetFilepath()] = textureSlot;
            textureSlot++;        
        }

        if (material.MaterialProperties.Specular && !textureCache.contains(material.MaterialProperties.Specular->GetFilepath()))
        {
            material.MaterialProperties.Specular->Bind(textureSlot);
            textureCache[material.MaterialProperties.Specular->GetFilepath()] = textureSlot;
            textureSlot++;
        }
    }

    // send texture uniforms
    for (auto& material : m_MaterialList)
    {
        std::string matLabel = "u_material[" + std::to_string(materialCache[material.SubMaterialName]) + "].";
                
        if(material.MaterialProperties.Ambient)
            shader->SetUniform1i(matLabel + "Ambient", textureCache[material.MaterialProperties.Ambient->GetFilepath()]);
        if (material.MaterialProperties.Diffuse)
            shader->SetUniform1i(matLabel + "Diffuse", textureCache[material.MaterialProperties.Diffuse->GetFilepath()]);
        if (material.MaterialProperties.Specular)
            shader->SetUniform1i(matLabel + "Specular", textureCache[material.MaterialProperties.Specular->GetFilepath()]);
    }

    // unbind textures
    for (auto& material : m_MaterialList)
    {
        if (material.MaterialProperties.Ambient)
            material.MaterialProperties.Ambient->Unbind();
        if (material.MaterialProperties.Diffuse)
            material.MaterialProperties.Diffuse->Unbind();
        if (material.MaterialProperties.Specular)
            material.MaterialProperties.Specular->Unbind();
    }

    shader->Unbind();
}


std::array<Maths::vec3i, 4> Model::GetFaceData(std::string& line, uint32_t& vertexCount)
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

    vertexCount = vertexIndex;

    return {
        Maths::vec3i(faceData[0], faceData[1], faceData[2]),
        Maths::vec3i(faceData[3], faceData[4], faceData[5]),
        Maths::vec3i(faceData[6], faceData[7], faceData[8]),
        Maths::vec3i(faceData[9], faceData[10], faceData[11])
    };

}

void Model::ParseMaterial(const std::string& filepath, std::vector<SubMaterialStream>& ss)
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

Material<std::shared_ptr<Texture>> Model::CreateMaterial(const std::string& matFile, const std::string& matData, const std::string& matName, unsigned int matID)
{
    using String = std::string;

    std::stringstream stream(matData);

    String line;


    float Ns = 32.f, Ni = 1.f, Opacity = 1.f;
    Maths::vec3 Tf{ 0.f }, Ka{ 0.f }, Kd{ 0.f }, Ke{ 0.f }, Ks{ 0.f };
    int Illum = 0;
    std::shared_ptr<Texture> Ambient = nullptr;
    std::shared_ptr<Texture> Diffuse = nullptr;
    std::shared_ptr<Texture> Specular = nullptr;

    while (std::getline(stream, line))
    {
        // this looks horrendous
        // can i make this work with a switch?


        size_t pos;
        if ((pos = line.find("Ns ")) == 1 || (pos = line.find("Ns ")) == 0)
            Ns = std::stof(line.substr(pos + 3));

        else if ((pos = line.find("Ni ")) == 1 || (pos = line.find("Ni ")) == 0)
            Ni = std::stof(line.substr(pos + 3));

        else if ((pos = line.find("d ")) == 1 || (pos = line.find("d ")) == 0)
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
            else if (line.find("map_bump") == 1 || line.find("map_Bump") == 1)
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
        false, false, matID,
        Ambient, Diffuse, Specular,
        Ka, Kd, Ks, Ke,
        Ni, Ns, Opacity, Tf, Illum
    };
}

Maths::vec3 Model::GetVertexData(std::string& line, int elementCount)
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