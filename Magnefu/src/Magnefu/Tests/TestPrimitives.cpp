#include "mfpch.h"
#include "TestPrimitives.h"
#include "Magnefu/Application.h"

#include "imgui.h"

namespace Magnefu
{
	TestPrimitives::TestPrimitives()
	{
		Application::Get().GetWindow().GetSceneCamera()->SetDefaultProps();

		m_Plane.Size = { 4.f, 4.f };
		m_Plane.Color = { 0.5f, 0.5f, 0.5f };
		m_Plane.Rotation = { -1.f, 0.f, 0.f };
		m_Plane.Angle = 90.f;
		
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
		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("Scene", tab_bar_flags))
		{
			if (ImGui::BeginTabItem("PLANE"))
			{
				ImGui::SliderFloat2("Size", m_Plane.Size.e, 1.f, 20.f);
				ImGui::SliderFloat("Angle", &m_Plane.Angle, -360.f, 360.f);
				ImGui::SliderFloat3("Rotation", m_Plane.Rotation.e, -1.f, 1.f);
				ImGui::ColorEdit3("Color", m_Plane.Color.e);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
}