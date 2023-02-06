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

        m_VAO = std::make_unique<VertexArray>();
        m_VAO->AddBuffer(*m_VBO, layout);



        m_Shader = std::make_unique <Shader>("res/shaders/TestLighting.shader");
        
        m_Shader->Bind();

        m_ObjectColor = { 1.f, 1.f, 1.f };

        // Lights

        m_light.K_a = 1.f; // should be in material file
        m_light.Position = { 0.f, 0.f, 0.f };
        m_light.Ambient = { 1.f, 1.f, 1.f, 1.f };
        
        
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
        SetShaderUniforms();

        // clear buffers
        m_VAO->Unbind();
        m_Shader->Unbind();
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

        Maths::mat4 modelMatrix = Maths::translate(m_translation) * m_Quat->UpdateRotMatrix(m_angleRot, m_rotationAxis) * Maths::scale(m_scaling);
        m_Camera->CalculateView();
        Maths::mat4 projMatrix = m_IsOrtho ? Maths::orthographic(m_left, m_right, m_bottom, m_top, m_near, m_far)
            : Maths::perspective(Maths::toRadians(global.fovY), m_aspectRatio, m_near, m_far);

        m_MVP = projMatrix * m_Camera->GetView() * modelMatrix;

        m_bottom = -m_top;
        m_right = m_top * m_aspectRatio;
        m_left = -m_right;


        m_Camera->ProcessInput(window, deltaTime);
	}

	void TestLighting::OnRender()
	{

        m_Renderer.Clear();

        m_Shader->Bind();
        SetShaderUniforms();

        m_Renderer.DrawCube(*m_VAO, *m_Shader);

	}
	
	void TestLighting::OnImGUIRender()
	{
        Globals& global = Globals::Get();

        ImGui::SliderInt("Cubes Count", &m_cubeCount, 0, 3);
        ImGui::ColorEdit3("Cube Color", m_ObjectColor.e);

        ImGui::Text("Lights");
        ImGui::SliderFloat("Ambient Strength", &m_light.K_a, 0.f, 1.f);
        ImGui::SliderFloat3("Light Position", m_light.Position.e, -10.f, 10.f);
        ImGui::ColorEdit4("Ambient Color", m_light.Ambient.e);


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

    void TestLighting::SetShaderUniforms()
    {
        m_Shader->SetUniformMatrix4fv("u_MVP", m_MVP);
        m_Shader->SetUniform3fv("u_ObjectColor", m_ObjectColor);
        m_Shader->SetUniform1f("u_light.K_a", m_light.K_a);
        m_Shader->SetUniform3fv("u_light.Position", m_light.Position);
        m_Shader->SetUniform4fv("u_light.Ambient", m_light.Ambient);
    }
}