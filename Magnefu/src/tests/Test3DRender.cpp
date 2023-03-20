#include "mfpch.h"

#include "Test3DRender.h"

#include "imgui/imgui.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Texture.h"
#include "Shader.h"
#include "VertexBufferAttribsLayout.h"
#include "Renderer.h"


namespace Magnefu
{
	Test3DRender::Test3DRender()
	{
        /* --Creating data and prepping buffer with data-- */

        float vertices[64] = {
            //  position               color                   texture coords
                -0.5f, -0.5f,  0.5f,    1.f,   0.5f,   0.3f,    0.f,  0.f,       // 0  BL
                 0.5f, -0.5f,  0.5f,    0.f,   0.5f,   0.7f,    1.f,  0.f,       // 1  BR
                 0.5f,  0.5f,  0.5f,    0.2f,  0.5f,   0.3f,    1.f,  1.f,       // 2  TR
                -0.5f,  0.5f,  0.5f,    0.8f,  1.f,    0.3f,    0.f,  1.f,       // 3  TL
                -0.5,  -0.5f, -0.5f,    1.f,   0.5f,   0.3f,    0.f,  0.f,       // 4  
                -0.5f,  0.5f, -0.5f,    0.f,   0.5f,   0.7f,    1.f,  0.f,       // 5
                 0.5f,  0.5f, -0.5f,    0.2f,  0.5f,   0.3f,    1.f,  1.f,       // 6
                 0.5f, -0.5f, -0.5f,    0.8f,  1.f,    0.3f,    0.f,  1.f        // 7
        };

        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0,
            2, 1, 7,
            7, 6, 2,
            2, 6, 5,
            5, 3, 2,
            3, 0, 4,
            4, 5, 3,
            0, 4, 7,
            7, 1, 0,
            5, 4, 7,
            7, 6, 5
        };
        

        VertexBuffer vbo(8 * 8 * sizeof(float), vertices);

        VertexBufferAttribsLayout layout;
        layout.Push<float>(3);
        layout.Push<float>(3);
        layout.Push<float>(2);

        m_VAO = std::make_unique<VertexArray>();
        m_VAO->AddBuffer(vbo, layout);

        m_IBO = std::make_unique<IndexBuffer>(sizeof(indices) / sizeof(unsigned int), indices);

        m_Shader = std::make_unique <Shader>("res/shaders/Test.shader");
        m_Texture0 = std::make_unique<Texture>("res/textures/grass.jpg");
        m_Texture1 = std::make_unique<Texture>("res/textures/moon.png");
        
        m_Shader->Bind();
        m_Texture0->Bind();
        m_Texture1->Bind(1);
        m_Shader->SetUniform1i("u_Texture0", 0);
        m_Shader->SetUniform1i("u_Texture1", 1);

        
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
        m_Texture0->Unbind();
        m_Texture1->Unbind();

        glEnable(GL_DEPTH_TEST);
	}

	Test3DRender::~Test3DRender()
	{
        glDisable(GL_DEPTH_TEST);
	}

	void Test3DRender::OnUpdate(GLFWwindow* window, float deltaTime)
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

	void Test3DRender::OnRender()
	{
        m_Renderer.Clear();

        m_Shader->Bind();

        m_Shader->SetUniformMatrix4fv("u_MVP", m_MVP);

        m_Texture0->Bind();
        m_Texture1->Bind(1);

        m_Renderer.Draw(*m_VAO, *m_IBO, *m_Shader);

	}
	
	void Test3DRender::OnImGUIRender()
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