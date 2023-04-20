#include "mfpch.h"
#include "TestPrimitives.h"
#include "Magnefu/Application.h"
#include "Magnefu/Core/Maths/Quaternion.h"

#include "imgui.h"

namespace Magnefu
{
	TestPrimitives::TestPrimitives()
	{
		m_Camera = std::static_pointer_cast<SceneCamera>(Application::Get().GetWindow().GetSceneCamera());
		m_Camera->SetDefaultProps();

		m_Plane.Size = { 10.f, 10.f, 0.f };
		m_Plane.Color = { 0.5f, 0.5f, 0.5f };
		m_Plane.Rotation = { -1.f, 0.f, 0.f };
		m_Plane.Translation = { 0.f, -1.f, 0.f };
		m_Plane.Angle = 90.f;

		m_Cube.Size = { 1.f, 0.f, 0.f };
		m_Cube.Color = { 1.f, 0.5f, 0.85f };
		m_Cube.Rotation = { 0.f, 0.f, 0.f };
		m_Cube.Translation = { 0.f, 0.f,0.f };
		m_Cube.Angle = 0.f;

		m_Sphere.Radius = 1.f;
		m_Sphere.SectorCount = 32;
		m_Sphere.StackCount = 13;
		m_Sphere.Color = { 0.1f, 0.73f, 0.4f };
		m_Sphere.Rotation = { 0.f, 0.f, 0.f };
		m_Sphere.Translation = { 0.f, 0.f,0.f };
		m_Sphere.Angle = 0.f;

	}

	TestPrimitives::~TestPrimitives()
	{
	}

	void TestPrimitives::OnUpdate(float deltaTime)
	{
		Application::Get().GetWindow().GetSceneCamera()->ProcessInput(deltaTime);
	}

	void TestPrimitives::OnRender(float renderInterpCoeff)
	{
		Renderer::BeginScene();
		Renderer::DrawPlane(m_Plane);
		Renderer::DrawCube(m_Cube);
		Renderer::DrawSphere(m_Sphere);
		Renderer::EndScene();
	}

	void TestPrimitives::OnEvent(Event&)
	{
		// TODO:
		// Recurrent neural network
		// Use loops for self reflection and rumination
	}

	void TestPrimitives::OnImGUIRender()
	{
		ImGui::Begin("Scene");
		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("Scene", tab_bar_flags))
		{
			if (ImGui::BeginTabItem("PLANE"))
			{
				ImGui::SliderFloat2("Size", m_Plane.Size.e, 1.f, 20.f);
				ImGui::SliderFloat("Angle", &m_Plane.Angle, -360.f, 360.f);
				ImGui::SliderFloat3("Rotation", m_Plane.Rotation.e, -1.f, 1.f);
				ImGui::SliderFloat3("Translation", m_Plane.Translation.e, -20.f, 20.f);
				ImGui::ColorEdit3("Color", m_Plane.Color.e);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("CUBE"))
			{
				ImGui::SliderFloat("Size", m_Cube.Size.e, 0.1f, 20.f);
				ImGui::SliderFloat("Angle", &m_Cube.Angle, -360.f, 360.f);
				ImGui::SliderFloat3("Rotation", m_Cube.Rotation.e, -1.f, 1.f);
				ImGui::SliderFloat3("Translation", m_Cube.Translation.e, -20.f, 20.f);
				ImGui::ColorEdit3("Color", m_Cube.Color.e);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("SPHERE"))
			{
				ImGui::SliderFloat("Radius", &m_Sphere.Radius, 0.1f, 20.f);
				ImGui::SliderInt("Stack Count", (int*) & m_Sphere.StackCount, 2, 80);
				ImGui::SliderInt("Sector Count", (int*) & m_Sphere.SectorCount, 3, 100);
				ImGui::SliderFloat("Angle", &m_Sphere.Angle, -360.f, 360.f);
				ImGui::SliderFloat3("Rotation", m_Sphere.Rotation.e, -1.f, 1.f);
				ImGui::SliderFloat3("Translation", m_Sphere.Translation.e, -20.f, 20.f);
				ImGui::ColorEdit3("Color", m_Sphere.Color.e);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();
		
	}
}