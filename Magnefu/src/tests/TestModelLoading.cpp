#include "mfpch.h"

#include "TestModelLoading.h"
#include "Magnefu/Application.h"
#include "ResourceCache.h"

#include "imgui/imgui.h"

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

#include <cstdio>
#include <filesystem>

namespace Magnefu
{
    static std::unique_ptr<Model> LoadModel(std::string& filepath, std::unordered_map<std::string, int>& matCache, std::mutex& mutex)
    {
        Timer timer;
        return std::make_unique<Model>(filepath, matCache, mutex);
    }

	TestModelLoading::TestModelLoading()
	{
        Timer timer;

        namespace fs = std::filesystem;

        std::error_code ec;
        const fs::path objPath = fs::relative(fs::path("res\\meshes"), ec);
        
        if (ec.value() != 0)
        {
            std::cout << "Error getting mesh path: " << ec.message() << std::endl;
            return;
        }

        for (const auto& entry : fs::directory_iterator(objPath))
        {
            const auto& filename = entry.path().filename().string();
            if (entry.is_regular_file() && entry.path().extension() == ".obj")
                m_Objs.push_back(filename);
        }

        // LOAD MESHES
        std::vector<std::string> objFiles = {
            "wooden_watch_tower2.obj",
            //"santa_hat(DEFAULT).obj",
            //12221_Cat_v1_l3.obj",
        };
                
        for(std::string& obj : objFiles)
            m_ModelWorkers[m_ModelWorkers.size()] = Worker<Model>{ false, std::async(std::launch::async, LoadModel, std::ref(obj), std::ref(m_MaterialCache), std::ref(m_ModelMutex) )};

        // TRANSFORM
        m_bShowTransform = false;

        m_MVP = Maths::identity();

        m_translation = { 6.4f, -1.2f, -4.f };
        m_scaling = { 1.f, 1.f, 1.f };
        m_angleRot = 0.f;
        m_rotationAxis = { 1.f, 0.f, 0.f };
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
        m_ReflectionModel = static_cast<int>(ReflectionModel::PHONG);
        m_ShadingTechnique = static_cast<int>(ShadingTechnique::PHONG);

        m_RadiantFlux = 3.f;
        m_Reflectance = 1.f;

        m_PointLights.emplace_back(CreatePointLight(), Maths::identity());
        m_PointLights.back().Enabled = true;

        m_DirectionLights.reserve(1);
        m_DirectionLights.emplace_back(CreateDirLight(), Maths::identity());
        m_DirectionLights.back().Enabled = false;

        m_SpotLights.reserve(1);
        m_SpotLights.emplace_back(CreateSpotLight(), Maths::identity());
        m_SpotLights.back().Enabled = false;

        m_lightScaling = { 0.1f, 0.1f, 0.1f };

        // SHADERS & TEXTURES

        Application& app = Application::Get();
        ResourceCache& cache = app.GetResourceCache();

        // should this be an async call?
        std::string shader1 = "res/shaders/TestModelLoading.shader";
        m_Shader = cache.RequestResource <Shader>(shader1); 
        m_Shader->Bind();
        SetShaderUniforms();
        m_Shader->Unbind();


        // RENDER OPTIONS
        m_Renderer.EnableDepthTest();
	}

	TestModelLoading::~TestModelLoading()
	{
        m_Renderer.DisableDepthTest();
	}

	void TestModelLoading::OnUpdate(GLFWwindow* window, float deltaTime)
	{
        OnHandleThreads();
        UpdateMVP();
        m_Camera->ProcessInput(window, deltaTime);
	}

    void TestModelLoading::OnHandleThreads()
    {
        for (uint32_t i = 0; i < m_ModelWorkers.size(); i++)
        {
            Worker<Model>& modelWorker = m_ModelWorkers[i];
            if (!modelWorker.WasAccessed && modelWorker.Thread.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                modelWorker.WasAccessed = true;
                m_InactiveThreads++;
                m_Models.push_back(modelWorker.Thread.get());
                m_Models.back()->Init(m_Shader, m_TextureCache, m_MaterialCache);
            }
        }

        // Clear worker threads
        if (m_ModelWorkers.size() > 0 && m_InactiveThreads == m_ModelWorkers.size())
        {
            m_ModelWorkers.clear();
            m_InactiveThreads = 0;
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
        m_Shader->Bind();
        SetShaderUniforms();
        for (auto& model : m_Models)
            model->Draw(m_Shader, m_TextureCache, m_MaterialCache);
        m_Shader->Unbind();
        
	}
	
    void TestModelLoading::OnImGUIRender()
    {

        Globals& global = Globals::Get();

        bool isopen = true;

        ImGui::Begin("Model Loader", &isopen, ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginTabBar("Model Load Options", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Select OBJ to Load: "))
            {
                for (auto& obj : m_Objs)
                {
                    if (ImGui::Button(obj.c_str()))
                        m_ModelWorkers[m_ModelWorkers.size()] = Worker<Model>{ false, std::async(std::launch::async, LoadModel, std::ref(obj), std::ref(m_MaterialCache), std::ref(m_ModelMutex)) };
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Select OBJ to Delete: "))
            {
                for (int i = 0; i < m_Models.size(); i++)
                {
                    if (ImGui::Button(m_Models[i]->m_Filepath.c_str()))
                    {
                        m_Models[i]->ClearFromCache(m_TextureCache, m_MaterialCache);
                        m_Models.erase(m_Models.begin() + i);
                    }
                        
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();

        if (ImGui::CollapsingHeader("Test Model Loading", ImGuiTreeNodeFlags_DefaultOpen))
        {
           
            if (ImGui::TreeNode("Options"))
            {
                if (ImGui::TreeNode("Camera Settings"))
                {
                    ImGui::Text("Camera");
                    ImGui::DragFloat("CameraSpeed", &m_Camera->m_Speed, 0.05f, 15.f);
                    ImGui::SliderFloat3("Camera Translation", m_Camera->m_Position.e, -10.f, 10.f);

                    ImGui::Text("Projection");
                    ImGui::SliderFloat("Near", &m_Camera->m_Properties.Near, 0.01f, 10.f);
                    ImGui::SliderFloat("Far", &m_Camera->m_Properties.Far, 10.f, 100.f);

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

                if (ImGui::TreeNode("Lighting & Shading"))
                {
                    ImGui::Text("Reflection Model: ");
                    ImGui::SameLine();
                    if (ImGui::Button("Phong ")) m_ReflectionModel = 0;
                    ImGui::SameLine();
                    if (ImGui::Button("Modified Phong")) m_ReflectionModel = 1;
                    ImGui::SameLine();
                    if (ImGui::Button("Blinn-Phong")) m_ReflectionModel = 2;
                    ImGui::SameLine();
                    if (ImGui::Button("Micro Facet")) m_ReflectionModel = 3;

                    ImGui::SliderFloat("Radiant Flux ", &m_RadiantFlux, 0.f, 100.f);

                    ImGui::Text("Shading Technique");
                    ImGui::SameLine();
                    if (ImGui::Button("Phong")) m_ShadingTechnique = 0;
                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }
            ImGui::Separator();

            if (ImGui::TreeNode("Scene Models"))
            {
                for (auto& model : m_Models)
                    model->OnImGUIRender();
                ImGui::TreePop();
            }
            ImGui::Separator();

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
                                ImGui::ColorEdit3("Color", m_DirectionLights[i].Color.e);
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
                                ImGui::ColorEdit3("Color", m_PointLights[i].Color.e);
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
                                ImGui::ColorEdit3("Color", m_SpotLights[i].Color.e);
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

    void TestModelLoading::SetShaderUniforms()
    {
        m_Shader->SetUniformMatrix4fv("u_MVP", m_MVP);

        m_Shader->SetUniform1i("u_ShadingTechnique", m_ShadingTechnique);
        m_Shader->SetUniform1i("u_ReflectionModel", m_ReflectionModel);

        //m_Shader->SetUniform1f("u_PointLightRadius", m_PointLightRadius);
        m_Shader->SetUniform1f("u_RadiantFlux", m_RadiantFlux);

        for (int i = 0; i < m_PointLights.size(); i++)
        {
            std::string lightLabel = "u_PointLights[" + std::to_string(i) + "].";
            m_Shader->SetUniform1i(lightLabel + "Enabled", m_PointLights[i].Enabled);
            m_Shader->SetUniform3fv(lightLabel + "Position", m_PointLights[i].Position);
            m_Shader->SetUniform3fv(lightLabel + "Color", m_PointLights[i].Color);
            m_Shader->SetUniform1f(lightLabel + "Constant", m_PointLights[i].constant);
            m_Shader->SetUniform1f(lightLabel + "Linear", m_PointLights[i].linear);
            m_Shader->SetUniform1f(lightLabel + "Quadratic", m_PointLights[i].quadratic);
        }

        for (int i = 0; i < m_DirectionLights.size(); i++)
        {
            std::string lightLabel = "u_DirectionLights[" + std::to_string(i) + "].";
            m_Shader->SetUniform1i(lightLabel + "Enabled", m_DirectionLights[i].Enabled);
            m_Shader->SetUniform3fv(lightLabel + "Direction", m_DirectionLights[i].Direction);
            m_Shader->SetUniform3fv(lightLabel + "Color", m_DirectionLights[i].Color);
        }

        for (int i = 0; i < m_SpotLights.size(); i++)
        {
            std::string lightLabel = "u_SpotLights[" + std::to_string(i) + "].";
            m_Shader->SetUniform1i(lightLabel + "Enabled", m_SpotLights[i].Enabled);
            m_Shader->SetUniform3fv(lightLabel + "Direction", m_SpotLights[i].Direction);
            m_Shader->SetUniform3fv(lightLabel + "Position", m_SpotLights[i].Position);
            m_Shader->SetUniform3fv(lightLabel + "Color", m_SpotLights[i].Color);
            m_Shader->SetUniform1f(lightLabel + "Constant", m_SpotLights[i].constant);
            m_Shader->SetUniform1f(lightLabel + "Linear", m_SpotLights[i].linear);
            m_Shader->SetUniform1f(lightLabel + "Quadratic", m_SpotLights[i].quadratic);
            m_Shader->SetUniform1f(lightLabel + "InnerCutoff", m_SpotLights[i].innerCutoff);
            m_Shader->SetUniform1f(lightLabel + "OuterCutoff", m_SpotLights[i].outerCutoff);
        }
    }
    
}