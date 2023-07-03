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

		m_PushConstants.LightEnabled = 1;
		m_PushConstants.LightColor = Maths::vec3(1.0f);
		m_PushConstants.LightPos = { 235.f, 65.f, 20.f };
		m_PushConstants.CameraPos = m_Camera->GetData().Position;
		m_PushConstants.Tint = Maths::vec3(1.0f);
		m_PushConstants.Opacity = 1.f;
		m_PushConstants.RadiantFlux = 25.f;
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

		m_PushConstants.CameraPos = m_Camera->GetData().Position;
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
				ImGui::ColorEdit3("Tint", m_PushConstants.Tint.e);
				ImGui::SliderFloat("Opacity", &m_PushConstants.Opacity, 0.0f, 1.0f, "%.2f");
				ImGui::Separator();
				ImGui::SliderInt("Light Enabled", &m_PushConstants.LightEnabled, 0, 1);
				ImGui::SliderFloat3("Light Position", m_PushConstants.LightPos.e, -500.f, 500.f);
				ImGui::ColorEdit3("Light Color", m_PushConstants.LightColor.e);
				ImGui::SliderFloat("Radiant Flux", &m_PushConstants.RadiantFlux, 0.f, 100.f, "%.2f");
				ImGui::SliderFloat("Reflectance", &m_PushConstants.Reflectance, 0.f, 5.f, "%.2f");
				ImGui::Separator();
				ImGui::SliderFloat3("Ambient Constant", m_PushConstants.Ka.e, 0.f, 1.f, "%.2f");
				ImGui::SliderFloat3("Diffuse Constant", m_PushConstants.Kd.e, 0.f, 1.f, "%.2f");
				ImGui::SliderFloat3("Specular Constant", m_PushConstants.Ks.e, 0.f, 1.f, "%.2f");
				ImGui::Separator();
				ImGui::LabelText("Camera Position", "%.2f, %.2f, %.2f", m_PushConstants.CameraPos.x, m_PushConstants.CameraPos.y, m_PushConstants.CameraPos.z);


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