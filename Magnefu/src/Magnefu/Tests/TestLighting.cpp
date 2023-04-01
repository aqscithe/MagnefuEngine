#include "mfpch.h"

#include "TestLighting.h"
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

#include "Primitive3D.h"
#include "Timer.h"

#include <array>


namespace Magnefu
{    
	TestLighting::TestLighting()
	{
        Timer timer;

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

        // Lights

        m_ShadingTechnique = static_cast<int>(ShadingTechnique::PHONG);
        m_ReflectionModel = static_cast<int>(ReflectionModel::PHONG);

        m_PointLightRadius = -1.f;

        m_PointLights.reserve(5);
        m_PointLights.emplace_back(CreatePointLight(), Maths::identity());
        m_PointLights.emplace_back(CreatePointLight(), Maths::identity());

        m_DirectionLights.reserve(1);
        m_DirectionLights.emplace_back(CreateDirLight(), Maths::identity());

        m_SpotLights.reserve(1);
        m_SpotLights.emplace_back(CreateSpotLight(), Maths::identity());

        m_lightScaling = { 0.1f, 0.1f, 0.1f };

        m_RadiantFlux = 3.f;

        m_Reflectance = 1.f;
        
        m_angleRot = 360.f;
        m_rotationAxis = { 0.f, 0.f, 0.f };
        m_translation = { 1.f, 0.f, 0.f };
        m_scaling = { 1.f, 1.f, 1.f };

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


        m_MVP = Maths::identity();

        Application& app = Application::Get();
        ResourceCache& cache = app.GetResourceCache();
        
        std::string shader2 = "res/shaders/TestLighting-Texture.shader";

        m_TextureCubeShader = cache.RequestResource<Shader>(shader2);
        m_TextureCubeShader->Bind();

        // need a way to read in all the textures I want by selecting a directory or something.
        // like in model loading

        std::array<std::string, 8> textureList = {
            //"res/textures/metal_plate/metal_plate_diff_2k.jpg",
            //"res/textures/old_copper/2K_old_copper_base_color.png",
            "res/textures/aluminium_foil-2K/2K-aluminium_foil_1_base_color.png",
            //"res/textures/Lava_03-2K/Lava_03_emissive-2K.png",
            //"res/textures/leaf_1-2K/2K-leaf_1-diffuse.jpg",
            "res/textures/amethyst_texture_1-2K/2K-amethyst_1-diffuse.jpg",
            //"res/textures/snow_field_aerial/snow_field_aerial_diff_2k.jpg",

            //"res/textures/metal_plate/metal_plate_spec_2k.jpg",
            //"res/textures/old_copper/2K_old_copper_specular_level.png",
            "res/textures/aluminium_foil-2K/2k-aluminum-spec.jpg",
            //"res/textures/Lava_03-2K/Lava_03_spec.jpg",
            //"res/textures/leaf_1-2K/2K-leaf_1-specular.jpg",
            "res/textures/amethyst_texture_1-2K/2K-amethyst_1-specular.jpg",
            //"res/textures/snow_field_aerial/snow_f_a_spec.jpg",

            //"res/textures/metal_plate/metal_plate_rough_2k.jpg",
            //"res/textures/old_copper/2K_old_copper_roughness.png",
            "res/textures/aluminium_foil-2K/2K-aluminium_foil_1_roughness.png",
            //"res/textures/Lava_03-2K/Lava_03_roughness-2K.png",
            //"res/textures/leaf_1-2K/leaf-rough.png",    // NEED TO GENERATE ROUGHNESS 
            "res/textures/amethyst_texture_1-2K/amethyst-rough.png", // NEED TO GENERATE ROUGHNESS 
            //"res/textures/snow_field_aerial/snow_field_aerial_rough_2k.jpg",

            //"res/textures/metal_plate/metal_plate_metal_2k.jpg",
            //"res/textures/old_copper/2K_old_copper_metallic.png",
            "res/textures/aluminium_foil-2K/2K-aluminium_foil_1_metallic.png",
            //"res/textures/Lava_03-2K/Lava_03_metallic-2K.png",
            //"res/textures/colors/black.jpg",
            "res/textures/colors/black.jpg",
            //"res/textures/colors/black.jpg",
        };

        m_Textures.reserve(textureList.size());

        // single threaded version
        
        for (int i = 0; i < textureList.size(); i++)
        {
            m_Textures.emplace_back(cache.RequestResource<Texture>(textureList[i]));
            m_Textures[i]->Bind();
        }

        String name = "aluminum";
        TextureProps aluminum;
        //aluminum.Ambient = nullptr;
        aluminum.Diffuse = m_Textures[0];
        aluminum.Specular = m_Textures[2];
        aluminum.Roughness = m_Textures[4];
        aluminum.Metallic = m_Textures[6];

        String name2 = "amethyst";
        TextureProps amethyst;
        //amethyst.Ambient = nullptr;
        amethyst.Diffuse = m_Textures[1];
        amethyst.Specular = m_Textures[3];
        amethyst.Roughness = m_Textures[5];
        amethyst.Metallic = m_Textures[7];

        m_Materials.emplace_back(cache.RequestResource<Material>(aluminum, name));
        m_Materials.emplace_back(cache.RequestResource<Material>(amethyst, name2));
        m_ActiveMaterial = m_Materials[0];

        int diffuseTextureLocations[2] = { static_cast<int>(m_Textures[0]->GetSlot()), static_cast<int>(m_Textures[1]->GetSlot()) };
        m_TextureCubeShader->SetUniform1iv("u_material.Diffuse", diffuseTextureLocations);

        int specularTextureLocations[2] = { static_cast<int>(m_Textures[2]->GetSlot()), static_cast<int>(m_Textures[3]->GetSlot()) };
        m_TextureCubeShader->SetUniform1iv("u_material.Specular", specularTextureLocations);

        int roughTextureLocs[2] = { static_cast<int>(m_Textures[4]->GetSlot()), static_cast<int>(m_Textures[5]->GetSlot()) };
        m_TextureCubeShader->SetUniform1iv("u_material.Roughness", roughTextureLocs);

        int metalTextureLocs[2] = { static_cast<int>(m_Textures[6]->GetSlot()), static_cast<int>(m_Textures[7]->GetSlot()) };
        m_TextureCubeShader->SetUniform1iv("u_material.Metallic", metalTextureLocs);

        for (int i = 0; i < m_Textures.size(); i++)
            m_Textures[i]->Unbind();

        SetTextureShaderUniforms();

        m_TextureCubeShader->Unbind();
        
        std::string shaderpath3 = "res/shaders/LightCube.shader";
        m_LightCubeShader = cache.RequestResource <Shader>(shaderpath3);

        //Set Uniforms

        // clear buffers
        m_ModelCubeVAO->Unbind();
        m_LightCubeVAO->Unbind();
        m_VBO->Unbind();
        
        m_Renderer.EnableDepthTest();
	}

	TestLighting::~TestLighting()
	{
        for (auto material : m_Materials)
        {
            material->Unbind();
        }

        m_Renderer.DisableDepthTest();

	}

	void TestLighting::OnUpdate(float deltaTime)
	{
        UpdateLights();
        UpdateMVP();
        m_Camera->ProcessInput(deltaTime);
	}

    void TestLighting::UpdateLights()
    {
        float moveRate = .000000001f;
        float moveScale = .5f;  // 1 means default sine wave of -1 to 1
        float moveAdjustment = 0.f;

        m_PointLights[0].Position.z = moveAdjustment + Maths::sin(std::chrono::high_resolution_clock::now().time_since_epoch().count() * moveRate) * moveScale;
        //m_PointLight.Diffuse.r = 0.5f + Maths::sin(std::chrono::high_resolution_clock::now().time_since_epoch().count() * moveRate) * 0.4f;
        //m_PointLight.Diffuse.g = 0.5f + Maths::sin(std::chrono::high_resolution_clock::now().time_since_epoch().count() * moveRate) * 0.5f;
        //m_PointLight.Diffuse.b = 0.5f + Maths::sin(std::chrono::high_resolution_clock::now().time_since_epoch().count() * .0000000001f) * 0.2f;
    }

    void TestLighting::UpdateMVP()
    {
        Globals& global = Globals::Get();

        m_Camera->m_Properties.Bottom = -m_Camera->m_Properties.Top;
        m_Camera->m_Properties.Right = m_Camera->m_Properties.Top * m_Camera->m_Properties.AspectRatio;
        m_Camera->m_Properties.Left = -m_Camera->m_Properties.Right;

        // Cube Matrix Update
        Maths::mat4 modelMatrix = Maths::translate(m_translation) * m_Quat->UpdateRotMatrix(m_angleRot, m_rotationAxis) * Maths::scale(m_scaling);
        m_Camera->CalculateView();
        Maths::mat4 projMatrix = m_Camera->m_Properties.IsOrtho ? Maths::orthographic(m_Camera->m_Properties.Left, m_Camera->m_Properties.Right, m_Camera->m_Properties.Bottom, m_Camera->m_Properties.Top, m_Camera->m_Properties.Near, m_Camera->m_Properties.Far)
            : Maths::perspective(Maths::toRadians(global.fovY) , m_Camera->m_Properties.AspectRatio, m_Camera->m_Properties.Near, m_Camera->m_Properties.Far);
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

        

        m_TextureCubeShader->Bind();
        m_TextureCubeShader->SetUniformMatrix4fv("u_ModelMatrix", modelMatrix);
        m_TextureCubeShader->SetUniformMatrix4fv("u_NormalMatrix", normalMatrix);
        m_TextureCubeShader->SetUniform3fv("u_CameraPos", m_Camera->GetPosition());
        m_TextureCubeShader->Unbind();
    }

	void TestLighting::OnRender()
	{

        m_Renderer.Clear();

        if (m_ActiveMaterial)
        {
            m_TextureCubeShader->Bind();
            m_ActiveMaterial->Bind();
            SetTextureShaderUniforms();
            m_Renderer.DrawCube(*m_ModelCubeVAO, *m_TextureCubeShader);

            m_TextureCubeShader->Unbind();
        }
            

        // Render light models
        // can comment out if don't need models representing their position
        m_LightCubeShader->Bind();
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
        m_LightCubeShader->Unbind();
        
	}
	
	void TestLighting::OnImGUIRender()
	{
        Globals& global = Globals::Get();

        if (ImGui::CollapsingHeader("Test Lighting", ImGuiTreeNodeFlags_DefaultOpen))
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

                    ImGui::SliderFloat("Reflectance(MicroFacet Param)", &m_Reflectance, 0.f, 10.f);

                    ImGui::Text("Shading Technique");
                    ImGui::SameLine();
                    if (ImGui::Button("Phong")) m_ShadingTechnique = 0;
                    ImGui::TreePop();
                }
                
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Material"))
            {
                for (size_t i = 0; i < m_Materials.size(); i++)
                {
                    if (ImGui::ColorButton(m_Materials[i]->Name.c_str(), {0.07568f, 0.61424f, 0.07568f, 1.f})) 
                        m_ActiveMaterial = m_Materials[i];
                    ImGui::SameLine();
                }

                if (m_ActiveMaterial)
                {
                    ImGui::Text("ID: %d", m_ActiveMaterial->ID);
                    ImGui::SliderFloat("Shininess", &m_ActiveMaterial->Ns, 0.001f, 300.f);

                    if(m_ReflectionModel == static_cast<int>(ReflectionModel::MICRO_FACET))
                        ImGui::SliderFloat("Index of Refraction", &m_ActiveMaterial->Ni, 0.f, 4.f);

                    ImGui::SliderFloat3("Ambient Strength", m_ActiveMaterial->Ka.e, 0.f, 1.f);
                    ImGui::SliderFloat3("Diffuse Strength", m_ActiveMaterial->Kd.e, 0.f, 1.f);
                    ImGui::SliderFloat3("Specular Strength", m_ActiveMaterial->Ks.e, 0.f, 1.f);
                }
                
                              
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
                    ImGui::SliderFloat("Point Light Radius ", &m_PointLightRadius, 0.f, 20.f);

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
                }
                ImGui::TreePop();
            }
        }
	}

    void TestLighting::SetTextureShaderUniforms()
    {
        m_TextureCubeShader->SetUniformMatrix4fv("u_MVP", m_MVP);

        m_TextureCubeShader->SetUniform1ui("u_material.TexID", m_ActiveMaterial->ID);
        
        m_TextureCubeShader->SetUniform1f("u_material.Ns", m_ActiveMaterial->Ns);
        m_TextureCubeShader->SetUniform3fv("u_material.Ka",       m_ActiveMaterial->Ka);
        m_TextureCubeShader->SetUniform3fv("u_material.Kd",       m_ActiveMaterial->Kd);
        m_TextureCubeShader->SetUniform3fv("u_material.Ks",       m_ActiveMaterial->Ks);

        m_TextureCubeShader->SetUniform1i("u_ShadingTechnique", m_ShadingTechnique);
        m_TextureCubeShader->SetUniform1i("u_ReflectionModel",  m_ReflectionModel);

        m_TextureCubeShader->SetUniform1f("u_PointLightRadius", m_PointLightRadius);
        m_TextureCubeShader->SetUniform1f("u_RadiantFlux", m_RadiantFlux);
        m_TextureCubeShader->SetUniform1f("u_Reflectance", m_Reflectance);

        for (int i = 0; i < m_PointLights.size(); i++)
        {
            std::string lightLabel = "u_PointLights[" + std::to_string(i) + "].";
            m_TextureCubeShader->SetUniform1i(lightLabel + "Enabled", m_PointLights[i].Enabled);
            m_TextureCubeShader->SetUniform3fv(lightLabel + "Position", m_PointLights[i].Position);
            m_TextureCubeShader->SetUniform3fv(lightLabel + "Color", m_PointLights[i].Color);
            m_TextureCubeShader->SetUniform1f(lightLabel + "Constant", m_PointLights[i].constant);
            m_TextureCubeShader->SetUniform1f(lightLabel + "Linear", m_PointLights[i].linear);
            m_TextureCubeShader->SetUniform1f(lightLabel + "Quadratic", m_PointLights[i].quadratic);
        }

        for (int i = 0; i < m_DirectionLights.size(); i++)
        {
            std::string lightLabel = "u_DirectionLights[" + std::to_string(i) + "].";
            m_TextureCubeShader->SetUniform1i(lightLabel + "Enabled", m_DirectionLights[i].Enabled);
            m_TextureCubeShader->SetUniform3fv(lightLabel + "Direction", m_DirectionLights[i].Direction);
            m_TextureCubeShader->SetUniform3fv(lightLabel + "Color", m_DirectionLights[i].Color);
        }

        for (int i = 0; i < m_SpotLights.size(); i++)
        {
            std::string lightLabel = "u_SpotLights[" + std::to_string(i) + "].";
            m_TextureCubeShader->SetUniform1i(lightLabel + "Enabled", m_SpotLights[i].Enabled);
            m_TextureCubeShader->SetUniform3fv(lightLabel + "Direction", m_SpotLights[i].Direction);
            m_TextureCubeShader->SetUniform3fv(lightLabel + "Position", m_SpotLights[i].Position);
            m_TextureCubeShader->SetUniform3fv(lightLabel + "Color", m_SpotLights[i].Color);
            m_TextureCubeShader->SetUniform1f(lightLabel + "Constant", m_SpotLights[i].constant);
            m_TextureCubeShader->SetUniform1f(lightLabel + "Linear", m_SpotLights[i].linear);
            m_TextureCubeShader->SetUniform1f(lightLabel + "Quadratic", m_SpotLights[i].quadratic);
            m_TextureCubeShader->SetUniform1f(lightLabel + "InnerCutoff", m_SpotLights[i].innerCutoff);
            m_TextureCubeShader->SetUniform1f(lightLabel + "OuterCutoff", m_SpotLights[i].outerCutoff);
        }
    }
    
}