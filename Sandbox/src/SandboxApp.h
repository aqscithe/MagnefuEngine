#pragma once

#include "Magnefu.h"

#include "imgui/imgui.h"


// USE STACK ALLOCATOR HERE

class GameLayer : public Magnefu::Layer
{
public:
	GameLayer() :
		Layer("3D Primitives")
	{
		m_Camera = std::static_pointer_cast<Magnefu::SceneCamera>(Magnefu::Application::Get().GetWindow().GetSceneCamera());
		m_GraphicsContext = Magnefu::Application::Get().GetWindow().GetGraphicsContext();
		m_PushConstants.Tint = Maths::vec3(1.0f);
	}

	void OnAttach() override
	{
		m_Camera->SetDefaultProps();
	}

	void OnDetach() override
	{

	}

	void OnUpdate(float deltaTime) override
	{
		m_Camera->ProcessInput(deltaTime);
		m_GraphicsContext->SetPushConstants(m_PushConstants);
	}

	void OnRender() override
	{
		
	}

	void OnGUIRender() override
	{
		ImGui::Begin("Scene");
		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("Scene", tab_bar_flags))
		{
			if (ImGui::BeginTabItem("Push Constants"))
			{
				ImGui::ColorEdit3("Color", m_PushConstants.Tint.e);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();
	}

	void OnEvent(Magnefu::Event& e) 
	{

	}

private:
	Magnefu::Ref<Magnefu::SceneCamera> m_Camera;
	Magnefu::GraphicsContext* m_GraphicsContext;
	Magnefu::PushConstants m_PushConstants;
};


class Sandbox : public Magnefu::Application
{
public:
	Sandbox();
	~Sandbox();

};