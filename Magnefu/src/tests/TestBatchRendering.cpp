#include "mfpch.h"

#include "TestBatchRendering.h"

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

#include "Primitive2D.h"

#include <array>

namespace Magnefu
{

	TestBatchRendering::TestBatchRendering()
	{
        /* --Creating data and prepping buffer with data-- */

        m_QuadCount = 4;
        ASSERT(m_QuadCount > 0);

        m_VBO = std::make_unique<VertexBuffer>(sizeof(Vertex) * 4 * 100, nullptr);

        VertexBufferAttribsLayout layout;
        layout.Push<float>(3);
        layout.Push<float>(3);
        layout.Push<float>(4);
        layout.Push<float>(2);
        layout.Push<unsigned int>(1);

        m_VAO = std::make_unique<VertexArray>();
        m_VAO->AddBuffer(*m_VBO, layout);


        m_IBO = std::make_unique<IndexBuffer>(sizeof(unsigned int) * 6 * 100, nullptr);

        m_Shader = std::make_unique <Shader>("res/shaders/BatchRender.shader");
        m_Texture0 = std::make_unique<Texture>("res/textures/wall.jpg");
        m_Texture1 = std::make_unique<Texture>("res/textures/pluto.png");
        
        m_Shader->Bind();
        
        m_Texture0->Bind();
        m_Texture1->Bind(1);

        int textureLocations[2] = { 0, 1 };
        m_Shader->SetUniform1iv("u_Texture", textureLocations);
        
        
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
        

        // clear buffers
        m_VAO->Unbind();
        m_Shader->Unbind();
        m_VBO->Unbind();
        m_IBO->Unbind();
        m_Texture0->Unbind();
        m_Texture1->Unbind();
        

	}

	TestBatchRendering::~TestBatchRendering()
	{
	}

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

	void TestBatchRendering::OnUpdate(GLFWwindow* window, float deltaTime)
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
        m_bottom = -m_top;
        m_right = m_top * m_aspectRatio;
        m_left = -m_right;


        m_Camera->ProcessInput(window, deltaTime);


        // dynamic buffer updates

        std::vector<unsigned int> indices;
        indices.reserve(m_QuadCount * 6); 
        
        indices = SetIndices(m_QuadCount);

        std::vector<Primitive::Quad> Quads;
        Quads.reserve(m_QuadCount);

        for (int i = 0; i < m_QuadCount; i++)
        {
            Quads.emplace_back(Primitive::CreateQuad(-0.5f + i * 1.5f, -0.5f + i * 1.5f, i % 2));
        }

        
        m_VBO->Bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Primitive::Quad) * Quads.size(), Quads.data());

        m_IBO->Bind();
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int) * indices.size(), indices.data());
        m_IBO->Unbind();
        m_VBO->Unbind();

	}

	void TestBatchRendering::OnRender()
	{

        m_Renderer.Clear();

        m_Shader->Bind();
        m_Shader->SetUniformMatrix4fv("u_MVP", m_MVP);
        m_Texture0->Bind();
        m_Texture1->Bind(1);

        m_Renderer.Draw(*m_VAO, *m_IBO, *m_Shader);

	}
	
	void TestBatchRendering::OnImGUIRender()
	{
        Globals& global = Globals::Get();

        ImGui::SliderInt("Quad Count", &m_QuadCount, 0, 20);

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