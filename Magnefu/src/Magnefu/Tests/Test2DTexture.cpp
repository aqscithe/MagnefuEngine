#include "mfpch.h"

#include "Test2DTexture.h"
#include "Magnefu/Application.h"

#include "imgui/imgui.h"

#include "Magnefu/Renderer/VertexArray.h"
#include "Magnefu/Renderer/Renderer.h"
#include "Magnefu/Scene/Camera.h"



namespace Magnefu
{
	Test2DTexture::Test2DTexture()
	{
        /* --Creating data and prepping buffer with data-- */

        float vertices[28] = {
            //  position               color                   texture coords
                -0.5f, -0.5f,    1.f,   0.5f,   0.3f,    0.f,  0.f,       // 0  BL
                 0.5f, -0.5f,    0.f,   0.5f,   0.7f,    1.f,  0.f,       // 1  BR
                 0.5f,  0.5f,    0.2f,  0.5f,   0.3f,    1.f,  1.f,       // 2  TR
                -0.5f,  0.5f,    0.8f,  1.f,    0.3f,    0.f,  1.f,       // 3  TL

        };

        uint32_t indices[] = {
            0, 1, 2,
            2, 3, 0
        };
        


        Ref<VertexBuffer> vbo = VertexBuffer::Create(sizeof(vertices), vertices);

        BufferLayout layout = {
            {ShaderDataType::Float2, "aPosition"},
            {ShaderDataType::Float3, "aColor"},
            {ShaderDataType::Float2, "aTexCoords"}
        };

        vbo->SetLayout(layout);

        Ref<IndexBuffer> ibo = IndexBuffer::Create(sizeof(indices) / sizeof(uint32_t), indices);

        m_VAO = VertexArray::Create();
        m_VAO->AddVertexBuffer(vbo);
        m_VAO->SetIndexBuffer(ibo);
        

        std::string shaderPath = "res/shaders/Texture2D.shader";
        std::string texturePath = "res/textures/moon.png";

        Application& app = Application::Get();

        /*TextureList textureList = {
            { TextureType::DIFFUSE, "res/textures/moon.png" }
        };*/

        m_Material = Material::Create(shaderPath);
        m_Material->Bind();

        m_angleRot = 0.f;
        m_rotationAxis = { 0.f, 0.f, 0.f };
        m_translation = { 0.f, 0.f, 0.f };
        m_scaling = { 1.f, 1.f, 1.f };

        m_Quat = CreateScope<Maths::Quaternion>(m_angleRot, m_rotationAxis);

        m_SceneCamera = app.GetWindow().GetSceneCamera();
        m_SceneCamera->SetDefaultProps();

        Maths::mat4 modelMatrix = Maths::translate(m_translation) * m_Quat->UpdateRotMatrix(m_angleRot, m_rotationAxis) * Maths::scale(m_scaling);
        m_SceneData = CreateScope<SceneData>();
        m_RenderData = CreateRef<SceneData>();
        m_SceneData->Mat4["u_MVP"] = m_SceneCamera->CalculateVP() * modelMatrix; // not sure this is necessary

        m_Material->InitRenderData(m_RenderData);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
	}

	Test2DTexture::~Test2DTexture()
	{
        glDisable(GL_BLEND);
	}

	void Test2DTexture::OnUpdate(float deltaTime)
	{
        // anything more efficient than just grabbing a copy of the
        // entire scenedata struct?
        m_PrevSceneData.Mat4["u_MVP"] = m_SceneData->Mat4["u_MVP"];

        Maths::mat4 modelMatrix = Maths::translate(m_translation) * m_Quat->UpdateRotMatrix(m_angleRot, m_rotationAxis) * Maths::scale(m_scaling);
        m_SceneData->Mat4["u_MVP"] = m_SceneCamera->CalculateVP() * modelMatrix; //current scene data
        m_SceneCamera->ProcessInput(deltaTime);
	}

	void Test2DTexture::OnRender(float renderInterpCoeff)
	{
        m_RenderData->Mat4["u_MVP"] = (m_SceneData->Mat4["u_MVP"] * renderInterpCoeff) + m_PrevSceneData.Mat4["u_MVP"] * (1.f - renderInterpCoeff);

        Renderer::BeginScene();
        Renderer::Submit(m_VAO, m_Material);
        Renderer::EndScene();
	}
	
	void Test2DTexture::OnImGUIRender()
	{
        ImGui::Text("Transform");
        ImGui::SliderFloat3("Model Translation", m_translation.e, -10.f, 10.f);
        ImGui::SliderFloat3("Model Rotation", m_rotationAxis.e, 0.f, 1.f);
        ImGui::SliderFloat("Model Rotation Angle", &m_angleRot, -360.f, 360.f);
        ImGui::SliderFloat3("Model Scale", m_scaling.e, 0.f, 10.f);

        m_SceneCamera->OnImGuiRender();

        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
        {
            if (ImGui::BeginTabItem("MATERIALS"))
            {
                m_Material->OnImGuiRender();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
             
	}
}