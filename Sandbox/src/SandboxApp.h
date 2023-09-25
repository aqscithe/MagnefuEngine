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
		m_PushConstants()
		//m_PointLights(Magnefu::Application::Get().GetPointLightData())
		//m_Light(Magnefu::Application::Get().GetLightData())
	{
		m_GraphicsContext = Magnefu::Application::Get().GetWindow().GetGraphicsContext();

		m_PushConstants.ALight.Intensity = 4.f;
		m_PushConstants.ALight.Color = Maths::vec3(1.f);
		m_PushConstants.ALight.Translation = Maths::vec3(0.f);
		m_PushConstants.ALight.Points0 = { -8.0f, 2.4f, -1.0f, 0.f };
		m_PushConstants.ALight.Points1 = { -8.0f, 2.4f,  1.0f, 0.f };
		m_PushConstants.ALight.Points2 = { -8.0f, 0.4f,  1.0f, 0.f };
		m_PushConstants.ALight.Points3 = { -8.0f, 0.4f, -1.0f, 0.f };
		m_PushConstants.ALight.TwoSided = 1;
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
					ImGui::ColorEdit3(label, material.Tint.e);

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

				ImGui::Text("Area Light");
				ImGui::Separator();

				ImGui::SliderFloat("Area Light Intensity", &m_PushConstants.ALight.Intensity, 0.0f, 10.f, "%.1f");
				ImGui::ColorEdit3("Area Light Color", m_PushConstants.ALight.Color.e);
				ImGui::SliderFloat3("Area Light Translation", m_PushConstants.ALight.Translation.e, -500.f, 500.f, "%.1f");
				ImGui::SliderInt("Area Light Two-sided", &m_PushConstants.ALight.TwoSided, 0, 1);

				// Now I need to ensure the shader knows it is receiving a push constant (either descriptor set stuff or
				// and I need to go to VulkanContext to ensure that is configured to properly send uniform data, push
				// constant data and is just configured correctly in general.

				/*for (int i = 0; i < m_PointLights.size(); i++)
				{
					char label[32];
					snprintf(label, sizeof(label), "Light %d Enabled", i);
					ImGui::SliderInt(label, &m_PointLights[i].LightEnabled, 0, 1);

					snprintf(label, sizeof(label), "Light %d Position", i);
					ImGui::SliderFloat3(label, m_PointLights[i].LightPos.e, -500.f, 500.f);

					snprintf(label, sizeof(label), "Light %d Color", i);
					ImGui::ColorEdit3(label, m_PointLights[i].LightColor.e);

					snprintf(label, sizeof(label), "Max Light Distance %d", i);
					ImGui::SliderFloat(label, &m_PointLights[i].MaxLightDist, 0.f, 1000.f, "%.2f");

					snprintf(label, sizeof(label), "Light %d Radiant Flux", i);
					ImGui::SliderFloat(label, &m_PointLights[i].RadiantFlux, 0.f, 100.f, "%.2f");

					ImGui::Separator();
				}*/
				

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
	std::vector<Magnefu::SceneObject>& m_SceneObjects;
	//std::array<Magnefu::PointLight, 3>&     m_PointLights;
	//Magnefu::Light&     m_Light;
};


class Sandbox : public Magnefu::Application
{
public:
	Sandbox();
	~Sandbox();

};