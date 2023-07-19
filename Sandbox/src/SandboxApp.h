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

		m_LightData.LightEnabled = 1;
		m_LightData.LightColor = Maths::vec3(1.0f);
		m_LightData.LightPos = { 235.f, 65.f, 20.f };
		m_LightData.RadiantFlux = 10.f;
		m_LightData.MaxLightDist = 200.f;


		m_MaterialData.Tint = Maths::vec3(1.0f);
		m_MaterialData.Opacity = 1.f;
		m_MaterialData.Reflectance = 0.1f;
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
		//m_GraphicsContext->SetPushConstants(m_PushConstants);

		Magnefu::Application& app = Magnefu::Application::Get();
		app.SetLightData(m_LightData);
		app.SetMaterialData(m_MaterialData);
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
				ImGui::Text("MATERIAL");
				ImGui::Separator();
				ImGui::ColorEdit3("Tint", m_MaterialData.Tint.e);
				ImGui::SliderFloat("Opacity", &m_MaterialData.Opacity, 0.0f, 1.0f, "%.2f");
				ImGui::SliderFloat("Reflectance", &m_MaterialData.Reflectance, 0.f, 5.f, "%.2f");

				ImGui::Text("LIGHT DATA");
				ImGui::Separator();
				ImGui::SliderInt("Light Enabled", &m_LightData.LightEnabled, 0, 1);
				ImGui::SliderFloat3("Light Position", m_LightData.LightPos.e, -500.f, 500.f);
				ImGui::ColorEdit3("Light Color", m_LightData.LightColor.e);
				ImGui::SliderFloat("Max Light Distance", &m_LightData.MaxLightDist, 0.f, 1000.f, "%.2f");
				ImGui::SliderFloat("Radiant Flux", &m_LightData.RadiantFlux, 0.f, 100.f, "%.2f");

				ImGui::Text("CAMERA");
				ImGui::Separator();
				ImGui::LabelText("Camera Position", "%.2f, %.2f, %.2f", m_Camera->GetData().Position.x, m_Camera->GetData().Position.y, m_Camera->GetData().Position.z);


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
	//Magnefu::PushConstants m_PushConstants;
	Magnefu::Light         m_LightData;
	Magnefu::Material      m_MaterialData;
};


class Sandbox : public Magnefu::Application
{
public:
	Sandbox();
	~Sandbox();

};