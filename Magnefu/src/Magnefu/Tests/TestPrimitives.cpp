#include "mfpch.h"
#include "TestPrimitives.h"
#include "Magnefu/Application.h"

#include "imgui.h"

namespace Magnefu
{
	TestPrimitives::TestPrimitives()
	{
		m_PlaneSize = { 10.f, 10.f };
		m_PlaneColor = { 0.5f, 0.5f, 0.5f };
		Application::Get().GetWindow().GetSceneCamera()->SetDefaultProps();
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
		Renderer::DrawPlane(m_PlaneSize, m_PlaneColor);
		Renderer::EndScene();
	}

	void TestPrimitives::OnEvent(Event&)
	{

	}

	void TestPrimitives::OnImGUIRender()
	{
		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("Scene", tab_bar_flags))
		{
			if (ImGui::BeginTabItem("PLANE"))
			{
				ImGui::SliderFloat2("Size", m_PlaneSize.e, 1.f, 20.f);
				ImGui::ColorEdit3("Color", m_PlaneColor.e);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
}