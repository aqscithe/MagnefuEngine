#pragma once

#include "Magnefu.h"

#include "imgui/imgui.h"

#include <filesystem>


class EditorLayer : public Magnefu::Layer
{
public:
	EditorLayer() :
		Layer("Editor"),
		m_SceneObjects(Magnefu::Application::Get().GetSceneObjects()),
		m_Camera(std::static_pointer_cast<Magnefu::SceneCamera>(Magnefu::Application::Get().GetWindow().GetSceneCamera())),
		m_PushConstants(),
		m_AreaLightPoints(),
		m_AreaLights(),
		current_path(std::filesystem::current_path())
	{
		Magnefu::Application& app = Magnefu::Application::Get();

		m_GraphicsContext = app.GetWindow().GetGraphicsContext();

		ImGuiStyle& style = ImGui::GetStyle();

		// Set colors for window background, text, headers, etc.
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f); // Dark gray
		style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
		style.Colors[ImGuiCol_Header] = ImVec4(0.2f, 0.6f, 1.0f, 1.0f); // Light blue
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.7f, 1.0f, 1.0f); // Lighter blue

		// -- Set Area Light Info -- //

		app.SetAreaLightVertices(
			{
				-8.0f, 2.4f, -1.0f, 0.f ,
				-8.0f, 2.4f,  1.0f, 0.f ,
				-8.0f, 0.4f,  1.0f, 0.f ,
				-8.0f, 0.4f, -1.0f, 0.f
			});

		// Get number of area light instances in the scene
		// Area Lights are currently at index 1
		m_AreaLightCount = m_SceneObjects[1].GetInstanceCount();

		app.SetAreaLightCount(m_AreaLightCount);

		for (int areaLight = 0; areaLight < m_AreaLightCount; areaLight++)
		{
			auto& light = m_AreaLights[areaLight];
			light.Intensity = 4.f;
			light.Color = Maths::vec3(1.f);
			light.Translation = Maths::vec3(0.f);
			light.TwoSided = 1;
		}

		// ------------------------------ //

		m_PushConstants.Roughness = 0.7f;


		// -- Initialize Instanced Materials -- //
		for (int object = 0; object < m_SceneObjects.size(); object++)
		{

			if (m_SceneObjects[object].IsInstanced())
			{
				int instanceCount = m_SceneObjects[object].GetInstanceCount();
				auto& material = m_SceneObjects[object].GetMaterialDataInstanced();

				for (int instance = 0; instance < instanceCount; instance++)
				{
					material.Translation[instance] = Maths::vec3(0.0);
					material.Rotation[instance] = Maths::vec3(0.0);
					material.Scale[instance] = Maths::vec3(1.0);
					material.AngleOfRot[instance] = 0.f;

				}

			}

		}

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
		Magnefu::Application::Get().SetAreaLightData(m_AreaLights);


		// Getting material applied to area light geometry
		auto& material = m_SceneObjects[1].GetMaterialDataInstanced();
		int instanceCount = m_SceneObjects[1].GetInstanceCount();

		for (int instance = 0; instance < instanceCount; instance++)
		{
			material.Translation[instance] = m_AreaLights[instance].Translation;
		}

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
				for (int object = 0; object < m_SceneObjects.size(); object++)
				{

					// -- TODO: DETERMINE IF OBJECT IS INSTANCED -- //
					if (m_SceneObjects[object].IsInstanced())
					{
						int instanceCount = m_SceneObjects[object].GetInstanceCount();
						auto& material = m_SceneObjects[object].GetMaterialDataInstanced();

						for (int instance = 0; instance < instanceCount; instance++)
						{
							char label_i[32];
							snprintf(label_i, sizeof(label_i), "Object %d Instance %d Pos", object, instance);
							ImGui::SliderFloat3(label_i, material.Translation[instance].e, -500.f, 500.f);

							snprintf(label_i, sizeof(label_i), "Object %d Instance %d Rotation", object, instance);
							ImGui::SliderFloat3(label_i, material.Rotation[instance].e, 0.f, 1.f);

							snprintf(label_i, sizeof(label_i), "Object %d Instance %d Rot Angle", object, instance);
							ImGui::SliderFloat3(label_i, &material.AngleOfRot[instance], -360.f, 360.f);

							snprintf(label_i, sizeof(label_i), "Object %d Instance %d Scale", object, instance);
							ImGui::SliderFloat3(label_i, material.Scale[instance].e, 0.f, 1.f);
						}
					}
					else
					{
						auto& material = m_SceneObjects[object].GetMaterialData();

						char label[32];

						snprintf(label, sizeof(label), "Object %d Pos", object);
						ImGui::SliderFloat3(label, material.Translation.e, -500.f, 500.f);

						snprintf(label, sizeof(label), "Object %d Rot", object);
						ImGui::SliderFloat3(label, material.Rotation.e, 0.f, 1.f);

						snprintf(label, sizeof(label), "Object %d Angle", object);
						ImGui::SliderFloat(label, &material.AngleOfRot, -360.f, 360.f);

						snprintf(label, sizeof(label), "Object %d Scale", object);
						ImGui::SliderFloat3(label, material.Scale.e, 0.f, 1.f);

						snprintf(label, sizeof(label), "Object %d Tint", object);
						ImGui::ColorEdit3(label, material.Tint.e);

						snprintf(label, sizeof(label), "Object %d Opacity", object);
						ImGui::SliderFloat(label, &material.Opacity, 0.f, 1.f);

						snprintf(label, sizeof(label), "Object %d Reflectance", object);
						ImGui::SliderFloat(label, &material.Reflectance, 0.f, 5.f, "%.2f");
					}



					ImGui::Separator();
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Area Lights"))
			{
				for (int areaLight = 0; areaLight < m_AreaLightCount; areaLight++)
				{
					char label[32];

					auto& light = m_AreaLights[areaLight];

					snprintf(label, sizeof(label), "AreaLight %d Color", areaLight);
					ImGui::ColorEdit3(label, light.Color.e);

					snprintf(label, sizeof(label), "AreaLight %d Translation", areaLight);
					ImGui::SliderFloat3(label, light.Translation.e, -1000, 1000, "%.2f");

					snprintf(label, sizeof(label), "AreaLight %d Intensity", areaLight);
					ImGui::SliderFloat(label, &light.Intensity, 0.f, 20.f, "%.2f");

					snprintf(label, sizeof(label), "AreaLight %d Two-Sided", areaLight);
					ImGui::SliderInt(label, &light.TwoSided, 0, 1);
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


				ImGui::SliderFloat("Test Roughness", &m_PushConstants.Roughness, 0.f, 1.f);


				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();

		 

		if (ImGui::Begin("File Explorer")) 
		{

			static char buffer[256] = "";

			// Button or item to go up one level
			if (ImGui::Button("..")) 
			{
				current_path = current_path.parent_path();
			}

			// Input field for typing out the directory
			if (ImGui::InputText("Go to", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) 
			{
				std::filesystem::path new_path(buffer);
				if (std::filesystem::exists(new_path) && std::filesystem::is_directory(new_path)) 
				{
					current_path = new_path;
				}
				else 
				{
					// Handle invalid path
				}
			}

			for (const auto& entry : std::filesystem::directory_iterator(current_path)) 
			{
				if (ImGui::Selectable(entry.path().filename().string().c_str())) 
				{
					if (entry.is_directory()) 
					{
						// Change directory
						current_path = entry.path();
					}
					else 
					{
						// Handle file selection
					}
				}
			}
		}
		ImGui::End();
	}

	void OnEvent(Magnefu::Event& e)
	{

	}

private:
	Magnefu::Ref<Magnefu::SceneCamera>                  m_Camera;
	Magnefu::GraphicsContext* m_GraphicsContext;
	Magnefu::PushConstants                              m_PushConstants;
	std::vector<Magnefu::SceneObject>& m_SceneObjects;
	int                                                 m_AreaLightCount;
	Maths::mat4                                         m_AreaLightPoints;
	std::array<Magnefu::AreaLight, Magnefu::MAX_AREA_LIGHTS>  m_AreaLights;

	std::filesystem::path current_path; 
	//Magnefu::Light&     m_Light;
};
