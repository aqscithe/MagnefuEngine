#include "TestModelLoading.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Texture.h"
#include "Shader.h"
#include "VertexBufferAttribsLayout.h"
#include "Renderer.h"
#include "Globals.h"

#include "Primitive3D.h"
#include "Timer.h"

#include <iostream>
#include <chrono>
#include <numeric>
//#include <mutex>

namespace test
{

    static std::vector<unsigned int> SetIndices(int quadCount)
    {
        int IndexPattern[6] = { 0, 1, 2, 2, 3, 0 };

        std::vector<unsigned int> indices;
        for (int quadNum = 0; quadNum < quadCount; quadNum++)
        {
            for (int i = 0; i < 6; i++)
                indices.push_back(IndexPattern[i] + 4 * quadNum);
        }
        return indices;
    }

    static void  LoadMesh(std::unique_ptr<Mesh>& mesh, std::string& filepath, std::vector<ObjModelVertex>& vertices, std::vector<unsigned int>& indices, std::vector<MaterialData>& materialList)
    {
        Timer timer;
        mesh = std::make_unique<Mesh>(filepath, materialList);

        vertices.clear();
        indices.clear();
        vertices.reserve(mesh->m_Faces.size() * 4);
        indices = SetIndices(static_cast<int>(mesh->m_Faces.size()));
        
        for (Face& face : mesh->m_Faces)
        {
            for (Maths::vec3i& Index : face.Indices)
                vertices.emplace_back(mesh->m_Positions[Index.v], mesh->m_Normals[Index.vn], mesh->m_TexCoords[Index.vt], face.TexID);
        }
    }

	TestModelLoading::TestModelLoading()
	{
        Timer timer;

        // LOAD MESHES
        std::string filepath = "res/meshes/cat/12221_Cat_v1_l3.obj";
        
        m_bFutureAccessed = false;
        m_Future = std::async(std::launch::async, LoadMesh, std::ref(m_Mesh), std::ref(filepath), std::ref(m_TempVertices), std::ref(m_TempIndices), std::ref(m_MaterialList));

        m_VBO = std::make_unique<VertexBuffer>(sizeof(ObjModelVertex) * 35288 * 4);

        m_IBO = std::make_unique<IndexBuffer>(sizeof(unsigned int) * 35288 * 6, nullptr);

        VertexBufferAttribsLayout layout;
        layout.Push<float>(3);
        layout.Push<float>(3);
        layout.Push<float>(2);
        layout.Push<unsigned int>(1);

        m_VAO = std::make_unique<VertexArray>();
        m_VAO->AddBuffer(*m_VBO, layout);

        


        // TRANSFORM
        m_bShowTransform = false;

        m_MVP = Maths::identity();

        m_translation = { 1.f, 0.f, 0.f };
        m_scaling = { 1.f, 1.f, 1.f };
        m_angleRot = 0.f;
        m_rotationAxis = { 0.f, 0.f, 0.f };
        m_Quat = std::make_unique<Maths::Quaternion>(m_angleRot, m_rotationAxis);

        m_Camera = std::make_unique<Camera>();

        Globals& global = Globals::Get();

        m_Camera->m_Properties.AspectRatio = (float)global.WIDTH / (float)global.HEIGHT;
        m_Camera->m_Properties.Near = 0.01f;
        m_Camera->m_Properties.Far = 100.f;
        m_Camera->m_Properties.Top = 10.f;
        m_Camera->m_Properties.Bottom = -m_Camera->m_Properties.Top;
        m_Camera->m_Properties.Right = m_Camera->m_Properties.Top * m_Camera->m_Properties.AspectRatio;
        m_Camera->m_Properties.Left = -m_Camera->m_Properties.Right;
        m_Camera->m_Properties.IsOrtho = false;

        // LIGHTS
        m_ReflectionModel = ReflectionModel::PHONG;
        m_ShadingTechnique = ShadingTechnique::PHONG;

        m_DirectionLights.reserve(1);
        m_DirectionLights.emplace_back(CreateDirLight(), Maths::identity());

        m_SpotLights.reserve(1);
        m_SpotLights.emplace_back(CreateSpotLight(), Maths::identity());

        m_lightScaling = { 0.1f, 0.1f, 0.1f };


        // SHADERS & TEXTURES
        m_Shader = std::make_unique <Shader>("res/shaders/TestModelLoading.shader");
        m_Shader->Bind();
        SetTextureShaderUniforms();
        m_Shader->Unbind();



        
        GLCall(glEnable(GL_DEPTH_TEST));
        std::cout << "Test Model Loading Constructor Finished" << std::endl;
	}

	TestModelLoading::~TestModelLoading()
	{
        GLCall(glDisable(GL_DEPTH_TEST));
	}

	void TestModelLoading::OnUpdate(GLFWwindow* window, float deltaTime)
	{
        OnHandleThreads();
        UpdateMVP();
        m_Camera->ProcessInput(window, deltaTime);
	}

    void TestModelLoading::OnHandleThreads()
    {
        if (m_Future.wait_for(std::chrono::seconds(0)) == std::future_status::ready && !m_bFutureAccessed)
        {
            // do something to invalidate the future so this if statement is not accessed a second time
            m_bFutureAccessed = true;

            m_VBO->Bind();
            GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ObjModelVertex) * m_TempVertices.size(), m_TempVertices.data()));

            m_IBO->Bind();
            GLCall(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int) * m_TempIndices.size(), m_TempIndices.data()));
            m_IBO->Unbind();
            m_VBO->Unbind();

            // CAN GENERATE TEXTURE IMAGES AND BIND THEM HERE

            //bind shader
            //generate textures
                //create method in texture class to generate texture images
            // bind textures
            // set diffuse, ambient and specular material uniforms - arrays
            // unbind textures
            // unbind shaders
            
            m_Mesh;
            m_MaterialList;
        }
    }

    void TestModelLoading::UpdateLights()
    {
        float moveRate = .000000001;
        float moveScale = 1.5f;  // 1 means default sine wave of -1 to 1
        float moveAdjustment = 1.f;

        //m_PointLight.Position.x = moveAdjustment + Maths::sin(std::chrono::high_resolution_clock::now().time_since_epoch().count() * moveRate) * moveScale;
        //m_PointLight.Diffuse.r = 0.5f + Maths::sin(std::chrono::high_resolution_clock::now().time_since_epoch().count() * moveRate) * 0.4f;
        //m_PointLight.Diffuse.g = 0.5f + Maths::sin(std::chrono::high_resolution_clock::now().time_since_epoch().count() * moveRate) * 0.5f;
        //m_PointLight.Diffuse.b = 0.5f + Maths::sin(std::chrono::high_resolution_clock::now().time_since_epoch().count() * .0000000001f) * 0.2f;
    }

    void TestModelLoading::UpdateMVP()
    {
        Globals& global = Globals::Get();

        m_Camera->m_Properties.Bottom = -m_Camera->m_Properties.Top;
        m_Camera->m_Properties.Right = m_Camera->m_Properties.Top * m_Camera->m_Properties.AspectRatio;
        m_Camera->m_Properties.Left = -m_Camera->m_Properties.Right;

        // Cube Matrix Update
        Maths::mat4 modelMatrix = Maths::translate(m_translation) * m_Quat->UpdateRotMatrix(m_angleRot, m_rotationAxis) * Maths::scale(m_scaling);
        m_Camera->CalculateView();
        Maths::mat4 projMatrix = m_Camera->m_Properties.IsOrtho ? Maths::orthographic(m_Camera->m_Properties.Left, m_Camera->m_Properties.Right, m_Camera->m_Properties.Bottom, m_Camera->m_Properties.Top, m_Camera->m_Properties.Near, m_Camera->m_Properties.Far)
            : Maths::perspective(Maths::toRadians(global.fovY), m_Camera->m_Properties.AspectRatio, m_Camera->m_Properties.Near, m_Camera->m_Properties.Far);
        m_MVP = projMatrix * m_Camera->GetView() * modelMatrix;

        // Light Matrix Update

        for (auto& pointlight : m_PointLights)
            pointlight.MVP = projMatrix * m_Camera->GetView() * Maths::translate(pointlight.Position) * Maths::scale(m_lightScaling);

        for(auto& spotlight : m_SpotLights)
            spotlight.MVP = projMatrix * m_Camera->GetView() * Maths::translate(spotlight.Position) * Maths::scale(m_lightScaling);
       

        // for accurate lighting when model isn't scaled uniformly
        Maths::mat4 inverted;
        Maths::mat4 normalMatrix = Maths::identity();
        if (Maths::invert(modelMatrix.e, inverted.e))
            normalMatrix = Maths::transpose(inverted);

        m_Shader->Bind();
        m_Shader->SetUniformMatrix4fv("u_ModelMatrix", modelMatrix);
        m_Shader->SetUniformMatrix4fv("u_NormalMatrix", normalMatrix);
        m_Shader->SetUniform3fv("u_CameraPos", m_Camera->GetPosition());
        m_Shader->Unbind();
    }

	void TestModelLoading::OnRender()
	{

        m_Renderer.Clear();

        if (m_Future.wait_for(std::chrono::seconds(0)) == std::future_status::ready && m_bFutureAccessed)
        {
            m_Shader->Bind();


            SetTextureShaderUniforms();
            m_Renderer.Draw(*m_VAO, *m_IBO, *m_Shader);

            m_Shader->Unbind();
        }


        // Render light models
        // can comment out if don't need models representing their position
        /*m_LightCubeShader->Bind();
        for (auto& pointlight : m_PointLights)
        {
            m_LightCubeShader->SetUniformMatrix4fv("u_MVP", pointlight.MVP);
            m_Renderer.DrawCube(*m_LightCubeVAO, *m_LightCubeShader);
        }
        for (auto& spotlight : m_SpotLights)
        {
            m_LightCubeShader->SetUniformMatrix4fv("u_MVP", spotlight.MVP);
            m_Renderer.DrawCube(*m_LightCubeVAO, *m_LightCubeShader);
        }
        m_LightCubeShader->Unbind();*/
        
	}
	
	void TestModelLoading::OnImGUIRender()
	{
        Globals& global = Globals::Get();

        if (ImGui::CollapsingHeader("Test Lighting", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::TreeNode("Options"))
            {
                /*if (ImGui::BeginMenu("MenuTest", true))
                {
                    ImGui::MenuItem("Hello", "Um...", &menu);
                    ImGui::MenuItem("Yahh", "uhh...", &munu);
                    ImGui::EndMenu();
                }*/
                ImGui::Text("Reflection Model: ");
                ImGui::SameLine();
                if (ImGui::Button("Phong ")) m_ReflectionModel = ReflectionModel::PHONG;
                ImGui::SameLine();
                if (ImGui::Button("Blinn-Phong")) m_ReflectionModel = ReflectionModel::BLINN_PHONG;

                ImGui::Text("Shading Technique");
                ImGui::SameLine();
                if (ImGui::Button("Phong")) m_ShadingTechnique = ShadingTechnique::PHONG;

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Material"))
            {

                
                ImGui::SliderFloat3("Ambient Intensity", m_AmbientIntensity.e, 0.f, 1.f);
                ImGui::SliderFloat3("Diffuse Intensity", m_DiffusionIntensity.e, 0.f, 1.f);
                ImGui::SliderFloat3("Specular Intensity", m_SpecularIntensity.e, 0.f, 1.f);
                
                              
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Scene Lights"))
            {
                if (ImGui::TreeNode("Directional Lights"))
                {
                    for (int i = 0; i < m_DirectionLights.size(); i++)
                    {
                        std::string name = "Directional Light: " + std::to_string(i);
                        if (ImGui::TreeNode(name.c_str()))
                        {
                            ImGui::Checkbox("Toggle Light", &m_DirectionLights[i].Enabled);
                            if (m_DirectionLights[i].Enabled)
                            {
                                ImGui::SliderFloat3("Light Direction ", m_DirectionLights[i].Direction.e, -1.f, 1.f);
                                ImGui::ColorEdit3("Diffuse Color", m_DirectionLights[i].Diffuse.e);
                                ImGui::ColorEdit3("Specular Color", m_DirectionLights[i].Specular.e);
                                ImGui::Text("Attenuation Controls");
                                ImGui::Text("Att Constant (Should be 1.0): %f", m_DirectionLights[i].constant);
                                ImGui::SliderFloat("Att Linear", &m_DirectionLights[i].linear, 0.f, 1.f);
                                ImGui::SliderFloat("Att Quadratic", &m_DirectionLights[i].quadratic, 0.f, 2.f);
                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Point Lights"))
                {
                    for (int i = 0; i < m_PointLights.size(); i++)
                    {
                        std::string name = "Point Light: " + std::to_string(i);
                        if (ImGui::TreeNode(name.c_str()))
                        {
                            ImGui::Checkbox("Toggle Light", &m_PointLights[i].Enabled);
                            if (m_PointLights[i].Enabled)
                            {
                                ImGui::SliderFloat3("Light Position ", m_PointLights[i].Position.e, -10.f, 10.f);
                                ImGui::SliderFloat3("Light Scale", m_lightScaling.e, -10.f, 10.f);
                                ImGui::ColorEdit3("Diffuse Color", m_PointLights[i].Diffuse.e);
                                ImGui::ColorEdit3("Specular Color", m_PointLights[i].Specular.e);
                                ImGui::Text("Attenuation Controls");
                                ImGui::Text("Att Constant (Should be 1.0): %f", m_PointLights[i].constant);
                                ImGui::SliderFloat("Att Linear", &m_PointLights[i].linear, 0.f, 1.f);
                                ImGui::SliderFloat("Att Quadratic", &m_PointLights[i].quadratic, 0.f, 2.f);
                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Spot Lights"))
                {
                    for (int i = 0; i < m_SpotLights.size(); i++)
                    {
                        std::string name = "Spot Light: " + std::to_string(i);
                        if (ImGui::TreeNode(name.c_str()))
                        {
                            ImGui::Checkbox("Toggle Light", &m_SpotLights[i].Enabled);
                            if (m_SpotLights[i].Enabled)
                            {
                                ImGui::SliderFloat3("Light Direction ", m_SpotLights[i].Direction.e, -10.f, 10.f);
                                ImGui::SliderFloat3("Light Position ", m_SpotLights[i].Position.e, -10.f, 10.f);
                                ImGui::ColorEdit3("Diffuse Color", m_SpotLights[i].Diffuse.e);
                                ImGui::ColorEdit3("Specular Color", m_SpotLights[i].Specular.e);
                                ImGui::SliderFloat("Inner Cutoff", &m_SpotLights[i].innerCutoff, 0.f, 1.f);
                                ImGui::SliderFloat("Outer Cutoff", &m_SpotLights[i].outerCutoff, 0.f, 1.f);
                                ImGui::Text("Attenuation Controls");
                                ImGui::Text("Att Constant (Should be 1.0): %f", m_SpotLights[i].constant);
                                ImGui::SliderFloat("Att Linear", &m_SpotLights[i].linear, 0.f, 1.f);
                                ImGui::SliderFloat("Att Quadratic", &m_SpotLights[i].quadratic, 0.f, 2.f);
                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
            
            if (ImGui::TreeNode("Transform"))
            {
                ImGui::Checkbox("Edit Transform", &m_bShowTransform);
                if (m_bShowTransform)
                {
                    ImGui::Text("Transform");
                    ImGui::SliderFloat3("Model Translation", m_translation.e, -10.f, 10.f);
                    ImGui::SliderFloat3("Model Rotation", m_rotationAxis.e, 0.f, 1.f);
                    ImGui::SliderFloat("Model Rotation Angle", &m_angleRot, -360.f, 360.f);
                    ImGui::SliderFloat3("Model Scale", m_scaling.e, 0.f, 10.f);

                    ImGui::Text("Camera");
                    ImGui::DragFloat("CameraSpeed", &m_Camera->m_Speed, 0.05f, 4.f);
                    ImGui::SliderFloat3("Camera Translation", m_Camera->m_Position.e, -10.f, 10.f);

                    ImGui::Text("Projection");
                    ImGui::SliderFloat("Near", &m_Camera->m_Properties.Near, 0.01f, 10.f);
                    ImGui::SliderFloat("Far", &m_Camera->m_Properties.Far, 10.f, 100.f);
                }

                ImGui::Checkbox("Toggle Projection Mode", &m_Camera->m_Properties.IsOrtho);
                if (m_Camera->m_Properties.IsOrtho)
                {
                    ImGui::SliderFloat("Top", &m_Camera->m_Properties.Top, 3.f, 15.f);
                    ImGui::Text("Left %.2f", m_Camera->m_Properties.Left);
                    ImGui::Text("Right %.2f", m_Camera->m_Properties.Right);
                    ImGui::Text("Bottom %.2f", m_Camera->m_Properties.Bottom);
                }
                else
                {
                    ImGui::SliderFloat("FOV", &global.fovY, 1.f, 100.f);
                }
                ImGui::TreePop();
            }
        }
	}

    void TestModelLoading::SetTextureShaderUniforms()
    {
        m_Shader->SetUniformMatrix4fv("u_MVP", m_MVP);



        m_Shader->SetUniform3fv("u_Intensity.Ambient",  m_AmbientIntensity);
        m_Shader->SetUniform3fv("u_Intensity.Diffuse",  m_DiffusionIntensity);
        m_Shader->SetUniform3fv("u_Intensity.Specular", m_SpecularIntensity);

        m_Shader->SetUniform1i("u_ShadingTechnique", static_cast<int>(m_ShadingTechnique));
        m_Shader->SetUniform1i("u_ReflectionModel", static_cast<int>(m_ReflectionModel));

        for (int i = 0; i < m_PointLights.size(); i++)
        {
            std::string lightLabel = "u_PointLights[" + std::to_string(i) + "].";
            m_Shader->SetUniform1i(lightLabel + "Enabled", m_PointLights[i].Enabled);
            m_Shader->SetUniform3fv(lightLabel + "Position", m_PointLights[i].Position);
            m_Shader->SetUniform3fv(lightLabel + "Diffuse", m_PointLights[i].Diffuse);
            m_Shader->SetUniform3fv(lightLabel + "Specular", m_PointLights[i].Specular);
            m_Shader->SetUniform1f(lightLabel + "Constant", m_PointLights[i].constant);
            m_Shader->SetUniform1f(lightLabel + "Linear", m_PointLights[i].linear);
            m_Shader->SetUniform1f(lightLabel + "Quadratic", m_PointLights[i].quadratic);
        }

        for (int i = 0; i < m_DirectionLights.size(); i++)
        {
            std::string lightLabel = "u_DirectionLights[" + std::to_string(i) + "].";
            m_Shader->SetUniform1i(lightLabel + "Enabled", m_DirectionLights[i].Enabled);
            m_Shader->SetUniform3fv(lightLabel + "Direction", m_DirectionLights[i].Direction);
            m_Shader->SetUniform3fv(lightLabel + "Diffuse", m_DirectionLights[i].Diffuse);
            m_Shader->SetUniform3fv(lightLabel + "Specular", m_DirectionLights[i].Specular);
        }

        for (int i = 0; i < m_SpotLights.size(); i++)
        {
            std::string lightLabel = "u_SpotLights[" + std::to_string(i) + "].";
            m_Shader->SetUniform1i(lightLabel + "Enabled", m_SpotLights[i].Enabled);
            m_Shader->SetUniform3fv(lightLabel + "Direction", m_SpotLights[i].Direction);
            m_Shader->SetUniform3fv(lightLabel + "Position", m_SpotLights[i].Position);
            m_Shader->SetUniform3fv(lightLabel + "Diffuse", m_SpotLights[i].Diffuse);
            m_Shader->SetUniform3fv(lightLabel + "Specular", m_SpotLights[i].Specular);
            m_Shader->SetUniform1f(lightLabel + "Constant", m_SpotLights[i].constant);
            m_Shader->SetUniform1f(lightLabel + "Linear", m_SpotLights[i].linear);
            m_Shader->SetUniform1f(lightLabel + "Quadratic", m_SpotLights[i].quadratic);
            m_Shader->SetUniform1f(lightLabel + "InnerCutoff", m_SpotLights[i].innerCutoff);
            m_Shader->SetUniform1f(lightLabel + "OuterCutoff", m_SpotLights[i].outerCutoff);
        }
    }
    
}