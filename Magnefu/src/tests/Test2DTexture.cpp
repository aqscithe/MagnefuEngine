#include "mfpch.h"

#include "Test2DTexture.h"
#include "Magnefu/Application.h"

#include "imgui/imgui.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Texture.h"
#include "Shader.h"
#include "VertexBufferAttribsLayout.h"
#include "Renderer.h"

#include "ResourceCache.h"



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

        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };
        

        VertexBuffer vbo(7 * 4 * sizeof(float), vertices);

        VertexBufferAttribsLayout layout;
        layout.Push<float>(2);
        layout.Push<float>(3);
        layout.Push<float>(2);

        m_VAO = std::make_unique<VertexArray>();
        m_VAO->AddBuffer(vbo, layout);

        m_IBO = std::make_unique<IndexBuffer>(sizeof(indices) / sizeof(unsigned int), indices);

        std::string shaderPath = "res/shaders/Texture2D.shader";

        Application& app = Application::Get();
        ResourceCache& cache = app.GetResourceCache();
        Shader shader = cache.RequestResource<Shader>(shaderPath);

        m_Shader = std::make_unique <Shader>(shaderPath);
        m_Texture = std::make_unique<Texture>("res/textures/moon.png");
        
        m_Shader->Bind();
        m_Texture->Bind();
        m_Shader->SetUniform1i("u_Texture", 0);

        
        m_angleRot = 0.f;
        m_rotationAxis = { 0.f, 0.f, 0.f };
        m_translation = { 0.f, 0.f, 0.f };
        m_scaling = { 1.f, 1.f, 1.f };

        m_Quat = std::make_unique<Maths::Quaternion>(m_angleRot, m_rotationAxis);

        m_Camera = std::make_unique<Camera>();

        Globals& global = Globals::Get();

        m_aspectRatio = (float)global.WIDTH / (float)global.HEIGHT;
        m_near = 0.01f;
        m_far = 100.f;
        m_top = m_near * tan(global.fovY / 2.f);
        m_bottom = -m_top;
        m_right = m_top * m_aspectRatio;
        m_left = -m_right;
        m_IsOrtho = false;

        m_MVP = Maths::identity();
        

        // clear buffers
        m_VAO->Unbind();
        m_Shader->Unbind();
        vbo.Unbind();
        m_IBO->Unbind();
        m_Texture->Unbind();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
	}

	Test2DTexture::~Test2DTexture()
	{
        glDisable(GL_BLEND);
	}

	void Test2DTexture::OnUpdate(GLFWwindow* window, float deltaTime)
	{
        Globals& global = Globals::Get();
        // Updating MVP

        //Maths::mat4 modelMatrix = Maths::translate(m_translation) * Maths::rotate(m_angleRot, m_rotationAxis) * Maths::scale(m_scaling);
        Maths::mat4 modelMatrix = Maths::translate(m_translation) * m_Quat->UpdateRotMatrix(m_angleRot, m_rotationAxis) * Maths::scale(m_scaling);
        m_Camera->CalculateView();
        Maths::mat4 projMatrix = m_IsOrtho ? Maths::orthographic(m_left, m_right, m_bottom, m_top, m_near, m_far)
            : Maths::perspective(Maths::toRadians(global.fovY), m_aspectRatio, m_near, m_far);

        m_MVP = projMatrix * m_Camera->GetView() * modelMatrix;

        // Updating Projection Parameters
        m_top = m_near * tan(global.fovY / 2.f);
        m_bottom = -m_top;
        m_right = m_top * m_aspectRatio;
        m_left = -m_right;

        m_Camera->ProcessInput(window, deltaTime);
	}

	void Test2DTexture::OnRender()
	{
        //m_Renderer.Clear();

        m_Shader->Bind();

        m_Shader->SetUniformMatrix4fv("u_MVP", m_MVP);

        m_Texture->Bind();

        m_Renderer.Draw(*m_VAO, *m_IBO, *m_Shader);

	}
	
	void Test2DTexture::OnImGUIRender()
	{
        Globals& global = Globals::Get();

        ImGui::Text("Transform");
        ImGui::SliderFloat3("Model Translation", m_translation.e, -10.f, 10.f);
        ImGui::SliderFloat3("Model Rotation", m_rotationAxis.e, 0.f, 1.f);
        ImGui::SliderFloat("Model Rotation Angle", &m_angleRot, -360.f, 360.f);
        ImGui::SliderFloat3("Model Scale", m_scaling.e, 0.f, 10.f);

        ImGui::Text("Camera");
        ImGui::DragFloat("CameraSpeed", &m_Camera->m_Speed, 0.05f, 4.f);
        ImGui::SliderFloat3("Camera Translation", m_Camera->m_Position.e, -10.f, 10.f);

        ImGui::Text("Projection");
        ImGui::Checkbox("Toggle Projection Mode", &m_IsOrtho);
        ImGui::SliderFloat("Near", &m_near, 0.01f, 10.f);
        ImGui::SliderFloat("Far", &m_far, 10.f, 100.f);
        if (m_IsOrtho)
        {
            ImGui::SliderFloat("Left", &m_left, -(float)global.WIDTH, (float)global.WIDTH);
            ImGui::SliderFloat("Right", &m_right, -(float)global.WIDTH, (float)global.WIDTH);
            ImGui::SliderFloat("Bottom", &m_bottom, -(float)global.HEIGHT, (float)global.HEIGHT);
            ImGui::SliderFloat("Top", &m_top, -(float)global.HEIGHT, (float)global.HEIGHT);
        }
        else
        {
            ImGui::SliderFloat("FOV", &global.fovY, 1.f, 100.f);
        }
	}
}