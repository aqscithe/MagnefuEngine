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

        auto cube = Primitive::CreateTextureCube();

        m_VBO = std::make_unique<VertexBuffer>(sizeof(Primitive::TextureCube) * 1, cube.Vertices.data());

        VertexBufferAttribsLayout layout;
        layout.Push<float>(3);
        layout.Push<float>(3);
        layout.Push<float>(2);
        layout.Push<unsigned int>(1);

        m_ModelCubeVAO = std::make_unique<VertexArray>();
        m_ModelCubeVAO->AddBuffer(*m_VBO, layout);
        m_ModelCubeVAO->Unbind();

        

        m_LightCubeVAO = std::make_unique<VertexArray>();
        m_LightCubeVAO->AddBuffer(*m_VBO, layout);
        m_LightCubeVAO->Unbind();

        

        m_AvailableMaterials = {
            {"Custom", { 
                         false, false, 99,
                         { 1.f, 1.f, 1.f },
                         { 1.f, 1.f, 1.f },
                         { 1.f, 1.f, 1.f },
                         32.f
                       }
            },
            {"Emerald", { 
                          true, false, 99,
                          { 0.0215f, 0.1745f, 0.0215f },
                          { 0.07568f, 0.61424f, 0.07568f},
                          { 0.633f, 0.727811f, 0.633f },
                          0.6f * 128
                        }
            },
            {"Jade", {
                          true, false, 99,
                          { 0.135f, 0.2225f, 0.1575f },
                          { 0.54f, 0.89f, 0.63f},
                          { 0.316228f, 0.316228f, 0.316228f },
                          0.1f * 128
                        }
            },
            {"Obsidian", {
                          true, false, 99,
                          { 0.05375f, 0.05f, 0.06625f },
                          { 0.18275f, 0.17f, 0.22525f},
                          { 0.332741f, 0.328634f, 0.346435f },
                          0.3f * 128
                        }
            },
            {"Pearl", {
                          true, false, 99,
                          { 0.25f, 0.20725f, 0.20725f },
                          { 1.f, 0.829f, 0.829f},
                          { 0.296648f, 0.296648f, 0.296648f },
                          0.088f * 128
                        }
            },
            {"Ruby", {
                          true, false, 99,
                          { 0.1745f, 0.01175f, 0.01175f },
                          { 0.61424f, 0.04136f, 0.04136f},
                          { 0.727811f, 0.626959f, 0.626959f },
                          0.6f * 128
                        }
            },
            {"Chrome", {
                          true, false, 99,
                          { 0.25f, 0.25f, 0.25f },
                          { 0.4f, 0.4f, 0.4f},
                          { 0.774597f, 0.774597f, 0.774597f },
                          0.6f * 128
                        }
            },
            {"White Plastic", {
                          true, false, 99,
                          { 0.f, 0.f, 0.f },
                          { 0.55f, 0.55f, 0.55f},
                          { 0.70f, 0.70f, 0.70f },
                          0.25f * 128
                        }
            },
            {"Black Rubber", {
                          true, false, 99,
                          { 0.02f, 0.02f, 0.02f },
                          { 0.01f, 0.01f, 0.01f},
                          { 0.4f, 0.4f, 0.4f },
                          0.078125f * 128
                        }
            },
            {"Water", {
                          true, true, 0,
                          { 1.f, 1.f, 1.f },
                          { 1.f, 1.f, 1.f },
                          { 1.f, 1.f, 1.f },
                          200.f
                        }
            },
            {"Grass", {
                          true, true, 1,
                          { 1.f, 1.f, 1.f },
                          { 1.f, 1.f, 1.f },
                          { 1.f, 1.f, 1.f },
                          200.f
                        }
            }
        };

        m_ActiveMaterial = &m_AvailableMaterials["Custom"]; 

        m_AmbientIntensity = { 0.1f, 0.1f, 0.1f };
        m_DiffusionIntensity = { 1.f, 1.f, 1.f };
        m_SpecularIntensity = { 1.f, 1.f, 1.f };

        // Lights

        m_ShadingTechnique = static_cast<int>(ShadingTechnique::PHONG);
        m_ReflectionModel = static_cast<int>(ReflectionModel::PHONG);

        m_light.K_d = 1.f;
        m_light.K_s = 1.f;
        m_light.Position = { -0.27f, 0.67f, 1.56f };
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
        
        m_ModelCubeShader = std::make_unique <Shader>("res/shaders/TestLighting.shader");
        m_ModelCubeShader->Bind();
        SetShaderUniforms();
        m_ModelCubeShader->Unbind();


        m_TextureCubeShader = std::make_unique <Shader>("res/shaders/TestLighting-Texture.shader");
        m_TextureCubeShader->Bind();
        m_Texture = std::make_unique<Texture>("res/textures/starlights/starlights_d.jpg");
        m_Texture1 = std::make_unique<Texture>("res/textures/darkmarble/dark-marbled-stone_d.jpg");
        m_Texture2 = std::make_unique<Texture>("res/textures/starlights/starlights_s.jpg");
        m_Texture3 = std::make_unique<Texture>("res/textures/darkmarble/dark-marbled-stone_s.jpg");
        m_Texture->Bind();
        m_Texture1->Bind(1);
        m_Texture2->Bind(2);
        m_Texture3->Bind(3);

        int diffuseTextureLocations[2] = { 0, 1 };
        m_TextureCubeShader->SetUniform1iv("u_material.Diffuse", diffuseTextureLocations);
        int specularTextureLocations[2] = { 2, 3 };
        m_TextureCubeShader->SetUniform1iv("u_material.Specular", specularTextureLocations);

        SetTextureShaderUniforms();
        m_Texture->Unbind();
        m_Texture1->Unbind();
        m_Texture2->Unbind();
        m_Texture3->Unbind();
        m_TextureCubeShader->Unbind();
        

        m_LightCubeShader = std::make_unique <Shader>("res/shaders/LightCube.shader");

        //Set Uniforms

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
        UpdateLights();
        UpdateMVP();
        m_Camera->ProcessInput(window, deltaTime);
	}

    void TestLighting::UpdateLights()
    {
        float moveRate = .000000001;
        float moveScale = 1.5f;  // 1 means default sine wave of -1 to 1
        float moveAdjustment = 1.f;

        m_light.Position.x = moveAdjustment + Maths::sin(std::chrono::high_resolution_clock::now().time_since_epoch().count() * moveRate) * moveScale;
        //m_light.Diffuse.r = 0.5f + Maths::sin(std::chrono::high_resolution_clock::now().time_since_epoch().count() * moveRate) * 0.4f;
        //m_light.Diffuse.g = 0.5f + Maths::sin(std::chrono::high_resolution_clock::now().time_since_epoch().count() * moveRate) * 0.5f;
        //m_light.Diffuse.b = 0.5f + Maths::sin(std::chrono::high_resolution_clock::now().time_since_epoch().count() * .0000000001f) * 0.2f;
    }

    void TestLighting::UpdateMVP()
    {
        Globals& global = Globals::Get();

        m_bottom = -m_top;
        m_right = m_top * m_aspectRatio;
        m_left = -m_right;

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
        if (Maths::invert(modelMatrix.e, inverted.e))
            normalMatrix = Maths::transpose(inverted);

        m_ModelCubeShader->Bind();
        m_ModelCubeShader->SetUniformMatrix4fv("u_ModelMatrix", modelMatrix);
        m_ModelCubeShader->SetUniformMatrix4fv("u_NormalMatrix", normalMatrix);
        m_ModelCubeShader->SetUniform3fv("u_CameraPos", m_Camera->GetPosition());
        m_ModelCubeShader->Unbind();

        m_TextureCubeShader->Bind();
        m_TextureCubeShader->SetUniformMatrix4fv("u_ModelMatrix", modelMatrix);
        m_TextureCubeShader->SetUniformMatrix4fv("u_NormalMatrix", normalMatrix);
        m_TextureCubeShader->SetUniform3fv("u_CameraPos", m_Camera->GetPosition());
        m_TextureCubeShader->Unbind();
    }

	void TestLighting::OnRender()
	{

        m_Renderer.Clear();

        if (m_ActiveMaterial->Textured)
        {
            m_TextureCubeShader->Bind();
            m_Texture->Bind();
            m_Texture1->Bind(1);  
            m_Texture2->Bind(2);
            m_Texture3->Bind(3);
            SetTextureShaderUniforms();
            m_Renderer.DrawCube(*m_ModelCubeVAO, *m_TextureCubeShader);

            m_TextureCubeShader->Unbind();
        }
        else
        {
            m_ModelCubeShader->Bind();
            SetShaderUniforms();
            m_Renderer.DrawCube(*m_ModelCubeVAO, *m_ModelCubeShader);
            m_ModelCubeShader->Unbind();
        }
            
        m_LightCubeShader->Bind();
        m_Renderer.DrawCube(*m_LightCubeVAO, *m_LightCubeShader);
	}
	
	void TestLighting::OnImGUIRender()
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
                if (ImGui::Button("Phong ")) m_ReflectionModel = 0;
                ImGui::SameLine();
                if (ImGui::Button("Blinn-Phong")) m_ReflectionModel = 1;

                ImGui::Text("Shading Technique");
                ImGui::SameLine();
                if (ImGui::Button("Phong")) m_ShadingTechnique = 0;
                ImGui::SameLine();
                if (ImGui::Button("Goraud")) m_ShadingTechnique = 1;

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Material"))
            {
                if (ImGui::Button("Custom")) m_ActiveMaterial = &m_AvailableMaterials["Custom"];
                ImGui::SameLine();
                if (ImGui::ColorButton("Emerald", { 0.07568f, 0.61424f, 0.07568f, 1.f})) m_ActiveMaterial = &m_AvailableMaterials["Emerald"];
                ImGui::SameLine();
                if (ImGui::ColorButton("Jade", { 0.54f, 0.89f, 0.63f, 1.f})) m_ActiveMaterial = &m_AvailableMaterials["Jade"];
                ImGui::SameLine();
                if (ImGui::ColorButton("Obsidian", { 0.18275f, 0.17f, 0.22525f, 1.f})) m_ActiveMaterial = &m_AvailableMaterials["Obsidian"];
                ImGui::SameLine();
                if (ImGui::ColorButton("Pearl", { 1.f, 0.829f, 0.829f, 1.f})) m_ActiveMaterial = &m_AvailableMaterials["Pearl"];
                ImGui::SameLine();
                if (ImGui::ColorButton("Ruby", { 0.61424f, 0.04136f, 0.04136f, 1.f})) m_ActiveMaterial = &m_AvailableMaterials["Ruby"];
                ImGui::SameLine();
                if (ImGui::ColorButton("Chrome", { 0.4f, 0.4f, 0.4f, 1.f})) m_ActiveMaterial = &m_AvailableMaterials["Chrome"];
                ImGui::SameLine();
                if (ImGui::ColorButton("White Plastic", { 0.55f, 0.55f, 0.55f, 1.f})) m_ActiveMaterial = &m_AvailableMaterials["White Plastic"];
                ImGui::SameLine();
                if (ImGui::ColorButton("Black Rubber", { 0.01f, 0.01f, 0.01f, 1.f})) m_ActiveMaterial = &m_AvailableMaterials["Black Rubber"];

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
                else if(!m_ActiveMaterial->Preset)
                {
                    ImGui::ColorEdit3("Diffuse", m_ActiveMaterial->Diffuse.e);
                    ImGui::ColorEdit3("Specular", m_ActiveMaterial->Specular.e);
                    ImGui::SliderFloat("Shininess", &m_ActiveMaterial->Shininess, 0.01f, 255.f);
                }
                ImGui::SliderFloat3("Ambient Intensity", m_AmbientIntensity.e, 0.f, 1.f);
                ImGui::SliderFloat3("Diffuse Intensity", m_DiffusionIntensity.e, 0.f, 1.f);
                ImGui::SliderFloat3("Specular Intensity", m_SpecularIntensity.e, 0.f, 1.f);
                              
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Lights"))
            {
                ImGui::SliderFloat3("Light Position", m_light.Position.e, -10.f, 10.f);
                ImGui::SliderFloat3("Light Scale", m_lightScaling.e, -10.f, 10.f);
                ImGui::SliderFloat("Diffuse Strength", &m_light.K_d, 0.f, 1.f);
                ImGui::ColorEdit3("Diffuse Color", m_light.Diffuse.e);
                ImGui::SliderFloat("Specular Strength", &m_light.K_s, 0.f, 1.f);
                ImGui::ColorEdit3("Specular Color", m_light.Specular.e);

                ImGui::TreePop();
            }
            
            if (ImGui::TreeNode("Transformation"))
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
                ImGui::TreePop();
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

        m_ModelCubeShader->SetUniform3fv("u_Intensity.Ambient", m_AmbientIntensity);
        m_ModelCubeShader->SetUniform3fv("u_Intensity.Diffuse", m_DiffusionIntensity);
        m_ModelCubeShader->SetUniform3fv("u_Intensity.Specular", m_SpecularIntensity);

        m_ModelCubeShader->SetUniform1i("u_ShadingTechnique", m_ShadingTechnique);
        m_ModelCubeShader->SetUniform1i("u_ReflectionModel", m_ReflectionModel);

        m_ModelCubeShader->SetUniform1f("u_light.K_d", m_light.K_d);
        m_ModelCubeShader->SetUniform1f("u_light.K_s", m_light.K_s);
        m_ModelCubeShader->SetUniform3fv("u_light.Position", m_light.Position);
        m_ModelCubeShader->SetUniform3fv("u_light.Diffuse", m_light.Diffuse);
        m_ModelCubeShader->SetUniform3fv("u_light.Specular", m_light.Specular);
    }

    void TestLighting::SetTextureShaderUniforms()
    {
        m_TextureCubeShader->SetUniformMatrix4fv("u_MVP", m_MVP);

        m_TextureCubeShader->SetUniform1ui("u_material.TexID", m_ActiveMaterial->TexID);
        
        m_TextureCubeShader->SetUniform1f("u_material.Shininess", m_ActiveMaterial->Shininess);

        m_TextureCubeShader->SetUniform3fv("u_Intensity.Ambient", m_AmbientIntensity);
        m_TextureCubeShader->SetUniform3fv("u_Intensity.Diffuse", m_DiffusionIntensity);
        m_TextureCubeShader->SetUniform3fv("u_Intensity.Specular", m_SpecularIntensity);

        m_TextureCubeShader->SetUniform1i("u_ShadingTechnique", m_ShadingTechnique);
        m_TextureCubeShader->SetUniform1i("u_ReflectionModel", m_ReflectionModel);

        m_TextureCubeShader->SetUniform1f("u_light.K_d", m_light.K_d);
        m_TextureCubeShader->SetUniform1f("u_light.K_s", m_light.K_s);
        m_TextureCubeShader->SetUniform3fv("u_light.Position", m_light.Position);
        m_TextureCubeShader->SetUniform3fv("u_light.Diffuse", m_light.Diffuse);
        m_TextureCubeShader->SetUniform3fv("u_light.Specular", m_light.Specular);
    }
}