#include "mfpch.h"
//
//#include "Model.h"
//#include "Magnefu/Application.h"
//#include "ResourceCache.h"
//
//#include "imgui/imgui.h"
//
//
//namespace Magnefu
//{
//
//    Model::Model(std::string& filepath)
//    {
//        Application& app = Application::Get();
//        ResourceCache& cache = app.GetResourceCache();
//
//        // TODO: Implement Fast Format for faster string parsing
//        m_Filepath = "res/meshes/" + filepath;
//        std::ifstream stream(m_Filepath);
//        std::string line;
//
//        std::string mtl;
//        std::string matFile;
//
//        std::unordered_map<std::string, uint32_t> matNameIDMap;
//        std::vector<Maths::vec3> tempPositions;
//        std::vector<Maths::vec3> tempNormals;
//        std::vector<Maths::vec2> tempTexCoords;
//        std::vector<Face> tempFaces;
//
//        VertexCount vCount = { 0, 0, 0 };
//        VertexCount vCountLast = vCount;
//
//        std::vector<SubMaterialStream> ss;
//        while (std::getline(stream, line))
//        {
//            if (line.find("mtllib ") == 0)
//            {
//                matFile = line.substr(7);
//                std::string filepath = "res/materials/" + matFile;
//                ParseMaterial(filepath, ss);
//
//                m_Materials.reserve(ss.size());
//                for (auto& matData : ss)
//                {
//                    m_Materials.emplace_back(cache.RequestResource<Material>(matFile, matData.StrData, matData.SubMatName));
//                    matNameIDMap[matData.SubMatName] = m_Materials.back()->ID;
//                }
//            }
//            else
//            {
//                // need to create a new mesh every time we finish getting data for a set of vertices
//                if (line.find("v ") == 0)
//                {
//                    if (!tempFaces.empty())
//                    {
//                        LoadMesh(tempPositions, tempNormals, tempTexCoords, tempFaces);
//                        vCountLast.normalCount += vCount.normalCount;
//                        vCountLast.posCount += vCount.posCount;
//                        vCountLast.uvCount += vCount.uvCount;
//
//                        vCount = { 0,0,0 };
//                    }
//
//                    tempPositions.emplace_back(GetVertexData(line, 3));
//                    vCount.posCount++;
//                }
//
//                else if (line.find("vn ") == 0)
//                {
//                    tempNormals.emplace_back(GetVertexData(line, 3));
//                    vCount.normalCount++;
//                }
//                else if (line.find("vt ") == 0)
//                {
//                    tempTexCoords.emplace_back(GetVertexData(line, 2).uv);
//                    vCount.uvCount++;
//                }
//                else if (line.find("f ") == 0)
//                {
//                    uint32_t vertexCount;
//                    auto faceData = GetFaceData(line, vertexCount);
//                    if (m_Meshes.size() > 0)
//                    {
//                        for (uint32_t i = 0; i < vertexCount; i++)
//                        {
//                            if (faceData[i].v < vCountLast.posCount)
//                                int j = 1;
//                            faceData[i].v -= vCountLast.posCount; //- faceData[i].v;
//                            faceData[i].vn -= vCountLast.normalCount; //- faceData[i].vn;
//                            faceData[i].vt -= vCountLast.uvCount; //- faceData[i].vt;
//                        }
//                    }
//                    tempFaces.emplace_back(faceData, matNameIDMap[mtl], vertexCount);
//                }
//
//                else if (line.find("usemtl ") == 0)
//                    mtl = line.substr(7);
//            }
//        }
//        // For the last mesh
//        if (!tempFaces.empty())
//            LoadMesh(tempPositions, tempNormals, tempTexCoords, tempFaces);
//
//    }
//
//    Model::~Model()
//    {
//    }
//
//    void Model::Draw(Shader* shader)
//    {
//
//        BindTextures();
//        SetShaderUniforms(shader);
//
//        for (auto& mesh : m_Meshes)
//            mesh->Draw(shader);
//
//        // TRY UNBINDING TEXTURES
//        UnbindTextures();
//    }
//
//    void Model::BindTextures()
//    {
//        for (auto& material : m_Materials)
//        {
//            if (material->Ambient)
//                material->Ambient->Bind();
//            if (material->Diffuse)
//                material->Diffuse->Bind();
//            if (material->Specular)
//                material->Specular->Bind();
//        }
//    }
//
//    void Model::UnbindTextures()
//    {
//        for (auto& material : m_Materials)
//        {
//            if (material->Ambient)
//                material->Ambient->Unbind();
//            if (material->Diffuse)
//                material->Diffuse->Unbind();
//            if (material->Specular)
//                material->Specular->Unbind();
//        }
//    }
//
//    void Model::SetShaderUniforms(Shader* shader)
//    {
//        for (auto& material : m_Materials)
//        {
//            std::string matLabel = "u_material[" + std::to_string(material->ID) + "].";
//
//            shader->SetUniform3fv(matLabel + "Ka", material->Ka);
//            shader->SetUniform3fv(matLabel + "Kd", material->Kd);
//            shader->SetUniform3fv(matLabel + "Ks", material->Ks);
//            shader->SetUniform1f(matLabel + "Ns", material->Ns);
//            shader->SetUniform1f(matLabel + "Ni", material->Ni);
//        }
//    }
//
//    void Model::OnImGUIRender()
//    {
//        ImGui::Text("Model Materials - %s", m_Filepath.c_str());
//
//        for (auto& material : m_Materials)
//        {
//            std::string name = "Material Name: " + material->Name;
//            if (ImGui::TreeNode(name.c_str()))
//            {
//                ImGui::Text("Material Lib: %s", material->Library.c_str());
//                ImGui::Text("Material ID: %d", material->ID);
//
//                if (material->Ambient)
//                    ImGui::Text("Ambient - Texture: %s | Render ID: %d", material->Ambient->GetFilepath().c_str(), material->Ambient->GetRendererID());
//                if (material->Diffuse)
//                    ImGui::Text("Diffuse - Texture: %s | Render ID: %d", material->Diffuse->GetFilepath().c_str(), material->Diffuse->GetRendererID());
//                if (material->Specular)
//                    ImGui::Text("Specular - Texture: %s | Render ID: %d", material->Specular->GetFilepath().c_str(), material->Specular->GetRendererID());
//                if (material->Roughness)
//                    ImGui::Text("Roughness - Texture: %s | Render ID: %d", material->Roughness->GetFilepath().c_str(), material->Roughness->GetRendererID());
//                if (material->Metallic)
//                    ImGui::Text("Metallic - Texture: %s | Render ID: %d", material->Metallic->GetFilepath().c_str(), material->Metallic->GetRendererID());
//
//                ImGui::SliderFloat3("Ka", material->Ka.e, 0.f, 1.f);
//                ImGui::SliderFloat3("Kd", material->Kd.e, 0.f, 1.f);
//                ImGui::SliderFloat3("Ks", material->Ks.e, 0.f, 1.f);
//                ImGui::SliderFloat("Ns", &material->Ns, 0.f, 256.f);
//                ImGui::SliderFloat("Ni", &material->Ni, 0.f, 256.f);
//                ImGui::TreePop();
//            }
//
//        }
//
//        for (auto& mesh : m_Meshes)
//            mesh->OnImGUIRender();
//    }
//
//    //void Model::ClearFromCache()
//    //{
//    //    for (auto& material : m_Materials)
//    //    {
//    //        materialCache.erase(material->Name);
//    //        MF_CORE_DEBUG("Clearing material {} from cache.", material->Name);
//
//    //        if (material->Ambient)
//    //        {
//    //            textureCache.erase(material->Ambient->GetFilepath());
//    //            MF_CORE_DEBUG("Clearing texture {} from cache", material->Ambient->GetFilepath());
//    //        }
//    //        if (material->Diffuse)
//    //        {
//    //            textureCache.erase(material->Diffuse->GetFilepath());
//    //            MF_CORE_DEBUG("Clearing texture {} from cache", material->Diffuse->GetFilepath());
//    //        }
//    //        if (material->Specular)
//    //        {
//    //            textureCache.erase(material->Specular->GetFilepath());
//    //            MF_CORE_DEBUG("Clearing texture {} from cache", material->Specular->GetFilepath());
//    //        }
//    //        if (material->Roughness)
//    //        {
//    //            textureCache.erase(material->Roughness->GetFilepath());
//    //            MF_CORE_DEBUG("Clearing texture {} from cache", material->Roughness->GetFilepath());
//    //        }
//    //        if (material->Metallic)
//    //        {
//    //            textureCache.erase(material->Metallic->GetFilepath());
//    //            MF_CORE_DEBUG("Clearing texture {} from cache", material->Metallic->GetFilepath());
//    //        }
//
//    //    }
//    //}
//
//    void Model::LoadMesh(std::vector<Maths::vec3>& tempPositions, std::vector<Maths::vec3>& tempNormals, std::vector<Maths::vec2>& tempTexCoords, std::vector<Face>& tempFaces)
//    {
//        MeshData meshData = {
//            tempPositions, tempNormals, tempTexCoords, tempFaces
//        };
//
//        m_Meshes.emplace_back(std::make_unique<Mesh>(meshData));
//
//        tempPositions.clear();
//        tempNormals.clear();
//        tempTexCoords.clear();
//        tempFaces.clear();
//    }
//
//    void Model::Init(Shader* shader)
//    {
//        shader->Bind();
//        for (auto& mesh : m_Meshes)
//            mesh->Init();
//
//        // generate textures
//        for (auto& material : m_Materials)
//        {
//            if (material->Ambient && !material->Ambient->GetGenerationStatus())
//                material->Ambient->GenerateTexture();
//            if (material->Diffuse && !material->Diffuse->GetGenerationStatus())
//                material->Diffuse->GenerateTexture();
//            if (material->Specular && !material->Specular->GetGenerationStatus())
//                material->Specular->GenerateTexture();
//            if (material->Roughness && !material->Roughness->GetGenerationStatus())
//                material->Roughness->GenerateTexture();
//            if (material->Metallic && !material->Metallic->GetGenerationStatus())
//                material->Metallic->GenerateTexture();
//        }
//
//
//        // bind textures
//        for (auto& material : m_Materials)
//        {
//            if (material->Ambient) //should i check if texture has been bound
//            {
//                material->Ambient->Bind();
//                MF_CORE_DEBUG("TEXTURE -- Ambient texture {0} bound to slot {1}", material->Ambient->GetFilepath(), material->Ambient->GetSlot());
//            }
//
//            if (material->Diffuse)
//            {
//                material->Diffuse->Bind();
//                MF_CORE_DEBUG("TEXTURE -- Diffuse texture {0} bound to slot {1}", material->Diffuse->GetFilepath(), material->Diffuse->GetSlot());
//            }
//
//            if (material->Specular)
//            {
//                material->Specular->Bind();
//                MF_CORE_DEBUG("TEXTURE -- Specular texture {0} bound to slot {1}", material->Specular->GetFilepath(), material->Specular->GetSlot());
//            }
//
//            if (material->Roughness)
//            {
//                material->Roughness->Bind();
//                MF_CORE_DEBUG("TEXTURE -- Roughness texture {0} bound to slot {1}", material->Roughness->GetFilepath(), material->Roughness->GetSlot());
//            }
//
//            if (material->Metallic)
//            {
//                material->Metallic->Bind();
//                MF_CORE_DEBUG("TEXTURE -- Metallic texture {0} bound to slot {1}", material->Metallic->GetFilepath(), material->Metallic->GetSlot());
//            }
//        }
//
//        // send texture uniforms
//        for (auto& material : m_Materials)
//        {
//            std::string matLabel = "u_material[" + std::to_string(material->ID) + "].";
//
//            if (material->Ambient)
//            {
//                shader->SetUniform1i(matLabel + "Ambient", (int)material->Ambient->GetSlot());
//                MF_CORE_DEBUG(
//                    "SHADER -- Set Ambient Texture {0} from Material {1} to Material Index {2}",
//                    material->Ambient->GetFilepath(), material->Name, material->ID
//                );
//
//            }
//
//            if (material->Diffuse)
//            {
//                shader->SetUniform1i(matLabel + "Diffuse", (int)material->Diffuse->GetSlot());
//                //shader->SetUniform3fv(matLabel + "Kd", material->Kd);
//                MF_CORE_DEBUG(
//                    "SHADER -- Set Diffuse Texture {0} from Material {1} to Material Index {2}",
//                    material->Diffuse->GetFilepath(), material->Name, material->ID
//                );
//            }
//
//            if (material->Specular)
//            {
//                shader->SetUniform1i(matLabel + "Specular", (int)material->Specular->GetSlot());
//                //shader->SetUniform3fv(matLabel + "Ks", material->Ks);
//                MF_CORE_DEBUG(
//                    "SHADER -- Set Specular Texture {0} from Material {1} to Material Index {2}",
//                    material->Specular->GetFilepath(), material->Name, material->ID
//                );
//            }
//
//            if (material->Roughness)
//            {
//                shader->SetUniform1i(matLabel + "Roughness", (int)material->Roughness->GetSlot());
//                MF_CORE_DEBUG(
//                    "SHADER -- Set Roughness Texture {0} from Material {1} to Material Index {2}",
//                    material->Roughness->GetFilepath(), material->Name, material->ID
//                );
//            }
//
//            if (material->Metallic)
//            {
//                shader->SetUniform1i(matLabel + "Metallic", (int)material->Metallic->GetSlot());
//                MF_CORE_DEBUG(
//                    "SHADER -- Set Metallic Texture {0} from Material {1} to Material Index {2}",
//                    material->Metallic->GetFilepath(), material->Name, material->ID
//                );
//            }
//
//
//
//
//        }
//
//        // unbind textures
//        for (auto& material : m_Materials)
//        {
//            if (material->Ambient)
//                material->Ambient->Unbind();
//            if (material->Diffuse)
//                material->Diffuse->Unbind();
//            if (material->Specular)
//                material->Specular->Unbind();
//            if (material->Roughness)
//                material->Roughness->Unbind();
//            if (material->Metallic)
//                material->Metallic->Unbind();
//        }
//
//        shader->Unbind();
//    }
//
//
//    std::array<Maths::vec3i, 4> Model::GetFaceData(std::string& line, uint32_t& vertexCount)
//    {
//        std::string faceLine = line.substr(2);
//        // maybe i shoudl make a 3x4 matrix
//        int faceData[12];
//
//        // TODO: Implement Fast Format for faster string parsing
//        // http://www.fastformat.org/documentation/index.html
//        std::stringstream ss;
//        ss.str(faceLine);
//
//        int vertexIndex = 0;
//        std::string vertexData;
//        char vertexDataDelimiter = ' ';
//        while (std::getline(ss, vertexData, vertexDataDelimiter))
//        {
//            int elementIndex = 0;
//            std::stringstream ssB(vertexData);
//            std::string vertexElement;
//            char vertexElementDelimiter = '/';
//            while (std::getline(ssB, vertexElement, vertexElementDelimiter))
//            {
//                // subtract 1 b/c face indices start at 1 instead of 0 in obj file
//                faceData[vertexIndex * 3 + elementIndex] = std::stoi(vertexElement) - 1;
//                elementIndex++;
//            }
//            vertexIndex++;
//        }
//
//        vertexCount = vertexIndex;
//
//        return {
//            Maths::vec3i(faceData[0], faceData[1], faceData[2]),
//            Maths::vec3i(faceData[3], faceData[4], faceData[5]),
//            Maths::vec3i(faceData[6], faceData[7], faceData[8]),
//            Maths::vec3i(faceData[9], faceData[10], faceData[11])
//        };
//
//    }
//
//    void Model::ParseMaterial(const std::string& filepath, std::vector<SubMaterialStream>& ss)
//    {
//        using String = std::string;
//
//        std::ifstream stream(filepath);
//
//        String line;
//        String newmtl;
//        std::stringstream matProperties;
//        while (std::getline(stream, line))
//        {
//            if (line.find("newmtl ") != String::npos)
//            {
//                if (!newmtl.empty())
//                {
//                    ss.push_back(SubMaterialStream{ newmtl, matProperties.str() });
//                    matProperties.clear();
//                }
//                newmtl = line.substr(7);
//            }
//            else if (!newmtl.empty())
//                matProperties << line << '\n';
//        }
//        ss.push_back(SubMaterialStream{ newmtl, matProperties.str() });
//    }
//
//    Maths::vec3 Model::GetVertexData(std::string& line, int elementCount)
//    {
//        std::string vertexLine = line.substr(3);
//        Maths::vec3 v;
//
//        std::stringstream ss;
//        ss.str(vertexLine);
//
//        int index = 0;
//        std::string del;
//        while (std::getline(ss, del, ' '))
//        {
//            v.e[index] = std::stof(del);
//            index++;
//        }
//        return v;
//    }
//}
//
//    