#include "TestLighting.h"

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

#include <array>
#include <iostream>
#include <chrono>

namespace test
{

	TestLighting::TestLighting()
	{
        /* --Creating data and prepping buffer with data-- */

        
        m_cubeCount = 1;
        ASSERT(m_cubeCount > 0);

        m_bShowTransform = false;

        auto cube = Primitive::CreateCube();

        m_VBO = std::make_unique<VertexBuffer>(sizeof(Primitive::Cube) * 1, cube.Vertices.data());

        VertexBufferAttribsLayout layout;
        layout.Push<float>(3);
        layout.Push<float>(3);
        layout.Push<float>(4);

        m_ModelCubeVAO = std::make_unique<VertexArray>();
        m_ModelCubeVAO->AddBuffer(*m_VBO, layout);
        m_ModelCubeVAO->Unbind();

        m_ModelCubeShader = std::make_unique <Shader>("res/shaders/TestLighting.shader");

        m_LightCubeVAO = std::make_unique<VertexArray>();
        m_LightCubeVAO->AddBuffer(*m_VBO, layout);
        m_LightCubeVAO->Unbind();

        m_LightCubeShader = std::make_unique <Shader>("res/shaders/LightCube.shader");

        m_AvailableMaterials = {
            {"Custom", { 
                         false,
                         { 1.f, 1.f, 1.f },
                         { 1.f, 1.f, 1.f },
                         { 1.f, 1.f, 1.f },
                         32.f
                       }
            },
            {"Emerald", { 
                          true,
                          { 0.0215f, 0.1745f, 0.0215f },
                          { 0.07568f, 0.61424f, 0.07568f},
                          { 0.633f, 0.727811f, 0.633f },
                          0.6f * 128
                        }
            }
        };

        m_ActiveMaterial = &m_AvailableMaterials["Emerald"]; 

        // Lights

        m_LightModel = static_cast<int>(LightModel::PHONG);

        m_light.K_a = 0.1f; 
        m_light.K_d = 1.f;
        m_light.K_s = 1.f;
        m_light.Position = { -0.27f, 0.67f, 1.56f };
        m_light.Ambient = { 1.f, 1.f, 1.f};
        m_light.Diffuse = { 1.f, 1.f, 1.f};
        m_light.Specular = { 1.f, 0.f, 1.f };
        m_lightScaling = { 0.1f, 0.1f, 0.1f };

        
        m_angleRot = 0.f;
        m_rotationAxis = { 0.f, 0.f, 0.f };
        m_translation = { 1.f, 0.f, 0.f };
        m_scaling = { 1.f, 1.f, 1.f };


        m_Quat = std::make_unique<Maths::Quaternion>(m_angleRot, m_rotationAxis);

        m_Camera = std::make_unique<Camera>();

        Globals& global = Globals::Get();

        m_aspectRatio = (float)global.WIDTH / (float)global.HEIGHT;
        m_near = 0.01f;
        m_far = 100.f;
        m_top = 10.f;
        m_bottom = -m_top;
        m_right = m_top * m_aspectRatio;
        m_left = -m_right;
        m_IsOrtho = false;

        m_MVP = Maths::identity();
        

        //Set Uniforms
        m_ModelCubeShader->Bind();
        SetShaderUniforms();
        m_ModelCubeShader->Unbind();

        // clear buffers
        m_ModelCubeVAO->Unbind();
        
        m_LightCubeVAO->Unbind();
        m_VBO->Unbind();
        
        GLCall(glEnable(GL_DEPTH_TEST));
	}

	TestLighting::~TestLighting()
	{
        GLCall(glDisable(GL_DEPTH_TEST));
	}

	void TestLighting::OnUpdate(GLFWwindow* window, float deltaTime)
	{
        Globals& global = Globals::Get();

        m_bottom = -m_top;
        m_right = m_top * m_aspectRatio;
        m_left = -m_right;

        float moveRate = .000000001;
        float moveScale = 1.5f;  // 1 means default sine wave of -1 to 1
        float moveAdjustment = 1.f;

        m_light.Position.x = moveAdjustment + Maths::sin(std::chrono::high_resolution_clock::now().time_since_epoch().count() * moveRate) * moveScale;

        // Cube Matrix Update
        Maths::mat4 modelMatrix = Maths::translate(m_translation) * m_Quat->UpdateRotMatrix(m_angleRot, m_rotationAxis) * Maths::scale(m_scaling);
        m_Camera->CalculateView();
        Maths::mat4 projMatrix = m_IsOrtho ? Maths::orthographic(m_left, m_right, m_bottom, m_top, m_near, m_far)
            : Maths::perspective(Maths::toRadians(global.fovY), m_aspectRatio, m_near, m_far);

        m_MVP = projMatrix * m_Camera->GetView() * modelMatrix;

        // Light Matrix Update
        Maths::mat4 lightCubeMVP = projMatrix * m_Camera->GetView() * Maths::translate(m_light.Position) * Maths::scale(m_lightScaling);
        m_LightCubeShader->Bind();
        m_LightCubeShader->SetUniformMatrix4fv("u_MVP", lightCubeMVP);
        m_LightCubeShader->Unbind();



        // for accurate lighting when model isn't scaled uniformly
        Maths::mat4 inverted;
        Maths::mat4 normalMatrix = Maths::identity();
        if(Maths::invert(modelMatrix.e, inverted.e))
             normalMatrix = Maths::transpose(inverted);

        m_ModelCubeShader->Bind();
        m_ModelCubeShader->SetUniformMatrix4fv("u_ModelMatrix", modelMatrix);
        m_ModelCubeShader->SetUniformMatrix4fv("u_NormalMatrix", normalMatrix);
        m_ModelCubeShader->SetUniform3fv("u_CameraPos", m_Camera->GetPosition());
        m_ModelCubeShader->Unbind();


        m_Camera->ProcessInput(window, deltaTime);
	}

	void TestLighting::OnRender()
	{

        m_Renderer.Clear();

        m_ModelCubeShader->Bind();
        SetShaderUniforms();
        m_ModelCubeShader->Unbind();

        m_LightCubeShader->Bind();
        m_LightCubeShader->Unbind();

        m_Renderer.DrawCube(*m_ModelCubeVAO, *m_ModelCubeShader);
        m_Renderer.DrawCube(*m_LightCubeVAO, *m_LightCubeShader);
	}
	
	void TestLighting::OnImGUIRender()
	{
        Globals& global = Globals::Get();

        if (ImGui::CollapsingHeader("Test Lighting", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::TreeNode("Material"))
            {
                if (ImGui::Button("Custom")) m_ActiveMaterial = &m_AvailableMaterials["Custom"];
                ImGui::SameLine();
                if (ImGui::Button("Emerald")) m_ActiveMaterial = &m_AvailableMaterials["Emerald"];

                if (m_ActiveMaterial->Preset)
                {
                    ImGui::Text("Ambient: %.3f %.3f %.3f", m_ActiveMaterial->Ambient.r, m_ActiveMaterial->Ambient.g, m_ActiveMaterial->Ambient.b);
                    ImGui::Text("Diffuse: %.3f %.3f %.3f", m_ActiveMaterial->Diffuse.r, m_ActiveMaterial->Diffuse.g, m_ActiveMaterial->Diffuse.b);
                    ImGui::Text("Specular: %.3f %.3f %.3f", m_ActiveMaterial->Specular.r, m_ActiveMaterial->Specular.g, m_ActiveMaterial->Specular.b);
                    ImGui::Text("Shininess: %.3f", m_ActiveMaterial->Shininess);
                }
                else
                {
                    ImGui::ColorEdit3("Ambient", m_ActiveMaterial->Ambient.e);
                    ImGui::ColorEdit3("Diffuse", m_ActiveMaterial->Diffuse.e);
                    ImGui::ColorEdit3("Specular", m_ActiveMaterial->Specular.e);
                    ImGui::SliderFloat("Shininess", &m_ActiveMaterial->Shininess, 0.f, 255.f);
                }
                

                //ImGui::SliderFloat("Roughness")
                //ImGui::SliderFloat("Opacity")                
                ImGui::SliderInt("Cubes Count", &m_cubeCount, 0, 3);
                ImGui::TreePop();
            }


            ImGui::Text("Lights");
            ImGui::Text("Light Model");
            ImGui::SameLine();
            if (ImGui::Button("Phong")) m_LightModel = 0;
            ImGui::SameLine();
            if (ImGui::Button("Goraud")) m_LightModel = 1;
            ImGui::SliderFloat3("Light Position", m_light.Position.e, -10.f, 10.f);
            ImGui::SliderFloat3("Light Scale", m_lightScaling.e, -10.f, 10.f);
            ImGui::SliderFloat("Ambient Strength", &m_light.K_a, 0.f, 1.f);
            ImGui::ColorEdit3("Ambient Color", m_light.Ambient.e);
            ImGui::SliderFloat("Diffuse Strength", &m_light.K_d, 0.f, 1.f);
            ImGui::ColorEdit3("Diffuse Color", m_light.Diffuse.e);
            ImGui::SliderFloat("Specular Strength", &m_light.K_s, 0.f, 1.f);
            ImGui::ColorEdit3("Specular Color", m_light.Specular.e);



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
                ImGui::SliderFloat("Near", &m_near, 0.01f, 10.f);
                ImGui::SliderFloat("Far", &m_far, 10.f, 100.f);
            }

            ImGui::Checkbox("Toggle Projection Mode", &m_IsOrtho);
            if (m_IsOrtho)
            {
                ImGui::SliderFloat("Top", &m_top, 3.f, 15.f);
                ImGui::Text("Left %.2f", m_left);
                ImGui::Text("Right %.2f", m_right);
                ImGui::Text("Bottom %.2f", m_bottom);
            }
            else
            {
                ImGui::SliderFloat("FOV", &global.fovY, 1.f, 100.f);
            }
        }
	}

    void TestLighting::SetShaderUniforms()
    {
        m_ModelCubeShader->SetUniformMatrix4fv("u_MVP", m_MVP);
        m_ModelCubeShader->SetUniform3fv("u_material.Ambient", m_ActiveMaterial->Ambient);
        m_ModelCubeShader->SetUniform3fv("u_material.Diffuse", m_ActiveMaterial->Diffuse);
        m_ModelCubeShader->SetUniform3fv("u_material.Specular", m_ActiveMaterial->Specular);
        m_ModelCubeShader->SetUniform1f("u_material.Shininess", m_ActiveMaterial->Shininess);
        m_ModelCubeShader->SetUniform1i("u_LightModel", m_LightModel);
        m_ModelCubeShader->SetUniform1f("u_light.K_a", m_light.K_a);
        m_ModelCubeShader->SetUniform1f("u_light.K_d", m_light.K_d);
        m_ModelCubeShader->SetUniform1f("u_light.K_s", m_light.K_s);
        m_ModelCubeShader->SetUniform3fv("u_light.Position", m_light.Position);
        m_ModelCubeShader->SetUniform3fv("u_light.Ambient", m_light.Ambient);
        m_ModelCubeShader->SetUniform3fv("u_light.Diffuse", m_light.Diffuse);
        m_ModelCubeShader->SetUniform3fv("u_light.Specular", m_light.Specular);
    }
}