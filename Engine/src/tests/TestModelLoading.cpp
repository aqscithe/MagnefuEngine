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

#include <array>
#include <iostream>
#include <chrono>

namespace test
{
	TestModelLoading::TestModelLoading()
	{
        Timer timer;

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


        // SHADERS & TEXTURES
        m_Shader = std::make_unique <Shader>("res/shaders/TestModelLoading.shader");
        m_Shader->Bind();
        SetTextureShaderUniforms();
        m_Shader->Unbind();


        // Load meshes
        m_Mesh = std::make_unique<Mesh>("res/meshes/12221_Cat_v1_l3.obj");


        
        GLCall(glEnable(GL_DEPTH_TEST));
	}

	TestModelLoading::~TestModelLoading()
	{
        GLCall(glDisable(GL_DEPTH_TEST));
	}

	void TestModelLoading::OnUpdate(GLFWwindow* window, float deltaTime)
	{

        UpdateMVP();
        m_Camera->ProcessInput(window, deltaTime);
        m_Mesh;
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

        if (m_ActiveMaterial && m_ActiveMaterial->Textured)
        {
            m_Shader->Bind();
            for (int i = 0; i < m_Textures.size(); i++)
                m_Textures[i]->Bind(i);
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
                if (m_ActiveMaterial)
                {
                    if (ImGui::Button("Custom")) m_ActiveMaterial = &m_AvailableMaterials["Custom"];
                    ImGui::SameLine();
                    if (ImGui::ColorButton("Emerald", { 0.07568f, 0.61424f, 0.07568f, 1.f })) m_ActiveMaterial = &m_AvailableMaterials["Emerald"];
                    ImGui::SameLine();
                    if (ImGui::ColorButton("Jade", { 0.54f, 0.89f, 0.63f, 1.f })) m_ActiveMaterial = &m_AvailableMaterials["Jade"];
                    ImGui::SameLine();
                    if (ImGui::ColorButton("Obsidian", { 0.18275f, 0.17f, 0.22525f, 1.f })) m_ActiveMaterial = &m_AvailableMaterials["Obsidian"];
                    ImGui::SameLine();
                    if (ImGui::ColorButton("Pearl", { 1.f, 0.829f, 0.829f, 1.f })) m_ActiveMaterial = &m_AvailableMaterials["Pearl"];
                    ImGui::SameLine();
                    if (ImGui::ColorButton("Ruby", { 0.61424f, 0.04136f, 0.04136f, 1.f })) m_ActiveMaterial = &m_AvailableMaterials["Ruby"];
                    ImGui::SameLine();
                    if (ImGui::ColorButton("Chrome", { 0.4f, 0.4f, 0.4f, 1.f })) m_ActiveMaterial = &m_AvailableMaterials["Chrome"];
                    ImGui::SameLine();
                    if (ImGui::ColorButton("White Plastic", { 0.55f, 0.55f, 0.55f, 1.f })) m_ActiveMaterial = &m_AvailableMaterials["White Plastic"];
                    ImGui::SameLine();
                    if (ImGui::ColorButton("Black Rubber", { 0.01f, 0.01f, 0.01f, 1.f })) m_ActiveMaterial = &m_AvailableMaterials["Black Rubber"];

                    if (ImGui::ColorButton("Water", { 0.f, 0.45f, 1.f, 1.f })) m_ActiveMaterial = &m_AvailableMaterials["Water"]; ImGui::SameLine();
                    if (ImGui::ColorButton("Grass", { 0.f, 1.f, 0.1f, 1.f })) m_ActiveMaterial = &m_AvailableMaterials["Grass"];

                    if (m_ActiveMaterial->Preset && !m_ActiveMaterial->Textured)
                    {
                        ImGui::Text("Ambient: %.4f %.4f %.4f", m_ActiveMaterial->Ambient.r, m_ActiveMaterial->Ambient.g, m_ActiveMaterial->Ambient.b);
                        ImGui::Text("Diffuse: %.4f %.4f %.4f", m_ActiveMaterial->Diffuse.r, m_ActiveMaterial->Diffuse.g, m_ActiveMaterial->Diffuse.b);
                        ImGui::Text("Specular: %.4f %.4f %.4f", m_ActiveMaterial->Specular.r, m_ActiveMaterial->Specular.g, m_ActiveMaterial->Specular.b);
                        ImGui::Text("Shininess: %.2f", m_ActiveMaterial->Shininess);
                        //ImGui::SliderFloat("Roughness")
                        //ImGui::SliderFloat("Opacity")  
                    }
                    else if (!m_ActiveMaterial->Preset)
                    {
                        ImGui::ColorEdit3("Diffuse", m_ActiveMaterial->Diffuse.e);
                        ImGui::ColorEdit3("Specular", m_ActiveMaterial->Specular.e);
                        ImGui::SliderFloat("Shininess", &m_ActiveMaterial->Shininess, 0.01f, 255.f);
                    }
                    ImGui::SliderFloat("Diffuse & Ambient Strength", &m_ActiveMaterial->K_d, 0.f, 1.f);
                    ImGui::SliderFloat("Specular Strength", &m_ActiveMaterial->K_s, 0.f, 1.f);
                }
                
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

    /*void TestModelLoading::SetShaderUniforms()
    {
        m_ModelCubeShader->SetUniformMatrix4fv("u_MVP", m_MVP);

        m_ModelCubeShader->SetUniform3fv("u_material.Ambient",  m_ActiveMaterial->Ambient);
        m_ModelCubeShader->SetUniform3fv("u_material.Diffuse",  m_ActiveMaterial->Diffuse);
        m_ModelCubeShader->SetUniform3fv("u_material.Specular", m_ActiveMaterial->Specular);
        m_ModelCubeShader->SetUniform1f("u_material.Shininess", m_ActiveMaterial->Shininess);
        m_ModelCubeShader->SetUniform1f("u_material.K_d",       m_ActiveMaterial->K_d);
        m_ModelCubeShader->SetUniform1f("u_material.K_s",       m_ActiveMaterial->K_s);

        m_ModelCubeShader->SetUniform3fv("u_Intensity.Ambient",  m_AmbientIntensity);
        m_ModelCubeShader->SetUniform3fv("u_Intensity.Diffuse",  m_DiffusionIntensity);
        m_ModelCubeShader->SetUniform3fv("u_Intensity.Specular", m_SpecularIntensity);

        m_ModelCubeShader->SetUniform1i("u_ShadingTechnique", m_ShadingTechnique);
        m_ModelCubeShader->SetUniform1i("u_ReflectionModel",  m_ReflectionModel);

        for (int i = 0; i < m_PointLights.size(); i++)
        {
            std::string lightLabel = "u_PointLights[" + std::to_string(i) + "].";
            m_ModelCubeShader->SetUniform1i(lightLabel + "Enabled", m_PointLights[i].Enabled);
            m_ModelCubeShader->SetUniform3fv(lightLabel + "Position", m_PointLights[i].Position);
            m_ModelCubeShader->SetUniform3fv(lightLabel + "Diffuse", m_PointLights[i].Diffuse);
            m_ModelCubeShader->SetUniform3fv(lightLabel + "Specular", m_PointLights[i].Specular);
            m_ModelCubeShader->SetUniform1f(lightLabel + "Constant", m_PointLights[i].constant);
            m_ModelCubeShader->SetUniform1f(lightLabel + "Linear", m_PointLights[i].linear);
            m_ModelCubeShader->SetUniform1f(lightLabel + "Quadratic", m_PointLights[i].quadratic);
        }

        for (int i = 0; i < m_DirectionLights.size(); i++)
        {
            std::string lightLabel = "u_DirectionLights[" + std::to_string(i) + "].";
            m_ModelCubeShader->SetUniform1i(lightLabel + "Enabled", m_DirectionLights[i].Enabled);
            m_ModelCubeShader->SetUniform3fv(lightLabel + "Direction", m_DirectionLights[i].Direction);
            m_ModelCubeShader->SetUniform3fv(lightLabel + "Diffuse", m_DirectionLights[i].Diffuse);
            m_ModelCubeShader->SetUniform3fv(lightLabel + "Specular", m_DirectionLights[i].Specular);
        }

        for (int i = 0; i < m_SpotLights.size(); i++)
        {
            std::string lightLabel = "u_SpotLights[" + std::to_string(i) + "].";
            m_ModelCubeShader->SetUniform1i(lightLabel + "Enabled", m_SpotLights[i].Enabled);
            m_ModelCubeShader->SetUniform3fv(lightLabel + "Direction", m_SpotLights[i].Direction);
            m_ModelCubeShader->SetUniform3fv(lightLabel + "Position", m_SpotLights[i].Position);
            m_ModelCubeShader->SetUniform3fv(lightLabel + "Diffuse", m_SpotLights[i].Diffuse);
            m_ModelCubeShader->SetUniform3fv(lightLabel + "Specular", m_SpotLights[i].Specular);
            m_ModelCubeShader->SetUniform1f(lightLabel + "Constant", m_SpotLights[i].constant);
            m_ModelCubeShader->SetUniform1f(lightLabel + "Linear", m_SpotLights[i].linear);
            m_ModelCubeShader->SetUniform1f(lightLabel + "Quadratic", m_SpotLights[i].quadratic);
            m_ModelCubeShader->SetUniform1f(lightLabel + "InnerCutoff", m_SpotLights[i].innerCutoff);
            m_ModelCubeShader->SetUniform1f(lightLabel + "OuterCutoff", m_SpotLights[i].outerCutoff);
        }
    }*/

    void TestModelLoading::SetTextureShaderUniforms()
    {
        m_Shader->SetUniformMatrix4fv("u_MVP", m_MVP);

        if (m_ActiveMaterial)
        {
            m_Shader->SetUniform1ui("u_material.TexID", m_ActiveMaterial->TexID);
            m_Shader->SetUniform1f("u_material.Shininess", m_ActiveMaterial->Shininess);
            m_Shader->SetUniform1f("u_material.K_d", m_ActiveMaterial->K_d);
            m_Shader->SetUniform1f("u_material.K_s", m_ActiveMaterial->K_s);
        }

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