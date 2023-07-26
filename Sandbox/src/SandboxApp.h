#pragma once

#include "Magnefu.h"

#include "imgui/imgui.h"


// USE STACK ALLOCATOR HERE

class GameLayer : public Magnefu::Layer
{
public:
	GameLayer() :
		Layer("3D Primitives"), 
		m_SceneObjects(Magnefu::Application::Get().GetSceneObjects()), 
		m_Camera(std::static_pointer_cast<Magnefu::SceneCamera>(Magnefu::Application::Get().GetWindow().GetSceneCamera())),
		//m_Lights(Magnefu::Application::Get().GetLightData())
		m_Light(Magnefu::Application::Get().GetLightData())
	{
		m_GraphicsContext = Magnefu::Application::Get().GetWindow().GetGraphicsContext();

		/*m_LightData.LightEnabled = 1;
		m_LightData.LightColor = Maths::vec3(1.0f);
		m_LightData.LightPos = { 235.f, 65.f, 20.f };
		m_LightData.RadiantFlux = 10.f;
		m_LightData.MaxLightDist = 200.f;*/


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
		//app.SetLightData(m_LightData);
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
			if (ImGui::BeginTabItem("Objects"))
			{
				for (int i = 0;i < m_SceneObjects.size(); i++)
				{
					auto& material = m_SceneObjects[i].GetMaterialData();

					char label[32];
					snprintf(label, sizeof(label), "Object %d Pos", i);
					ImGui::SliderFloat3(label, material.Translation.e, -500.f, 500.f);

					snprintf(label, sizeof(label), "Object %d Rot", i);
					ImGui::SliderFloat3(label, material.Rotation.e, 0.f, 1.f);

					snprintf(label, sizeof(label), "Object %d Angle", i);
					ImGui::SliderFloat(label, &material.AngleOfRot, -360.f, 360.f);

					snprintf(label, sizeof(label), "Object %d Scale", i);
					ImGui::SliderFloat3(label, material.Scale.e, 0.f, 1.f);

					snprintf(label, sizeof(label), "Object %d Tint", i);
					ImGui::SliderFloat3(label, material.Tint.e, 0.f, 1.f);

					snprintf(label, sizeof(label), "Object %d Opacity", i);
					ImGui::SliderFloat(label, &material.Opacity, 0.f, 1.f);

					snprintf(label, sizeof(label), "Object %d Reflectance", i);
					ImGui::SliderFloat(label, &material.Reflectance, 0.f, 5.f, "%.2f");

					ImGui::Separator();
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Push Constants"))
			{
				ImGui::Text("CAMERA");
				ImGui::Separator();
				ImGui::LabelText("Camera Position", "%.2f, %.2f, %.2f", m_Camera->GetData().Position.x, m_Camera->GetData().Position.y, m_Camera->GetData().Position.z);

				ImGui::Text("LIGHT DATA");
				ImGui::Separator();

				//for (int i = 0; i < m_Lights.size(); i++)
				//{
				//	char label[32];
				//	snprintf(label, sizeof(label), "Light %d Enabled", i);
				//	ImGui::SliderInt(label, &m_Lights[i].LightEnabled, 0, 1);

				//	snprintf(label, sizeof(label), "Light %d Position", i);
				//	ImGui::SliderFloat3(label, m_Lights[i].LightPos.e, -500.f, 500.f);

				//	snprintf(label, sizeof(label), "Light %d Color", i);
				//	ImGui::ColorEdit3(label, m_Lights[i].LightColor.e);

				//	snprintf(label, sizeof(label), "Max Light Distance %d", i);
				//	ImGui::SliderFloat(label, &m_Lights[i].MaxLightDist, 0.f, 1000.f, "%.2f");

				//	snprintf(label, sizeof(label), "Light %d Radiant Flux", i);
				//	ImGui::SliderFloat(label, &m_Lights[i].RadiantFlux, 0.f, 100.f, "%.2f");

				//	ImGui::Separator();
				//}

				char label[32];
				snprintf(label, sizeof(label), "Light Enabled");
				ImGui::SliderInt(label, &m_Light.LightEnabled, 0, 1);

				snprintf(label, sizeof(label), "Light Position");
				ImGui::SliderFloat3(label, m_Light.LightPos.e, -500.f, 500.f);

				snprintf(label, sizeof(label), "Light Color");
				ImGui::ColorEdit3(label, m_Light.LightColor.e);

				snprintf(label, sizeof(label), "Max Light Distance");
				ImGui::SliderFloat(label, &m_Light.MaxLightDist, 0.f, 1000.f, "%.2f");

				snprintf(label, sizeof(label), "Light Radiant Flux");
				ImGui::SliderFloat(label, &m_Light.RadiantFlux, 0.f, 100.f, "%.2f");
				

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
	std::vector<Magnefu::SceneObject>& m_SceneObjects;
	//std::array<Magnefu::Light, 3>&     m_Lights;
	Magnefu::Light&     m_Light;
};


class Sandbox : public Magnefu::Application
{
public:
	Sandbox();
	~Sandbox();

};