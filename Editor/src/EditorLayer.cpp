#include "EditorLayer.h"

#include "imgui/imgui.h"

static const char* COMPONENT_TYPES[] = { "TransformComponent", "MeshComponent" };
static int currentComponent = -1; // -1: no selection, 0: MeshComponent, 1: TransformComponent
static bool showComponentCombo = false; // Flag to control visibility of the combo box

EditorLayer::EditorLayer() :
	Layer("Editor"),
	m_SceneObjects(Magnefu::Application::Get().GetSceneObjects()),
	m_Camera(std::static_pointer_cast<Magnefu::SceneCamera>(Magnefu::Application::Get().GetWindow().GetSceneCamera())),
	m_PushConstants(),
	m_AreaLightPoints(),
	m_AreaLights(),
	current_path(std::filesystem::current_path()),
	m_ActiveScene(nullptr),
	m_SelectedEntity(nullptr)
{
	Magnefu::Application& app = Magnefu::Application::Get();
	auto& scenes = app.GetScenes();
	


	//// All of this should be within an ImGui editor //
	//Magnefu::Scene& scene = app.CreateScene();
	//Magnefu::Entity& entity = scene->CreateEntity();

	//// I should only need to pass the name as an argument for meshcomponent. The other values should be fetched behind the scenes
	//Magnefu::MeshComponent& mesh = entity.AddComponent<Magnefu::MeshComponent>(Mesh_Name);
	//Magnefu::TransformComponent transform = entity.AddComponent<Magnefu::TransformComponent>(Maths::vec3(0.f), Maths::vec3(0.f), Maths::vec3(1.f));

	m_GraphicsContext = app.GetWindow().GetGraphicsContext();


	// Set colors for window background, text, headers, etc.


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

void EditorLayer::OnAttach()
{
	m_Camera->SetDefaultProps();
}

void EditorLayer::OnDetach()
{

}

void EditorLayer::OnUpdate(float deltaTime)
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

void EditorLayer::OnEvent(Magnefu::Event& e)
{

}

void EditorLayer::OnRender()
{

}

void EditorLayer::OnGUIRender()
{
	/*static bool show_demo_window = true;
	if (show_demo_window) 
	{
		ImGui::ShowDemoWindow(&show_demo_window);
	}*/
	ShowCreateSceneWindow();
	ShowScene();

	// TODO: Hmm...the editor should determine which scene is active...
	ShowEntityListWindow();

	ShowComponentWindow();

	if (showComponentCombo)
	{
		ShowAddComponentWidget();
	}

	ShowFileExplorer();
}

void EditorLayer::ShowScene()
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
}

void EditorLayer::ShowFileExplorer()
{
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


void EditorLayer::ShowCreateSceneWindow()
{
	if (ImGui::Begin("New Scene"))
	{
		if (ImGui::Button("New Scene"))
		{
				m_ActiveScene = Magnefu::Application::Get().CreateScene();
		}
	}
	ImGui::End();
}

void EditorLayer::ShowEntityListWindow()
{
	if (ImGui::Begin("Entities")) 
	{
		if (m_ActiveScene)
		{
			for (auto& entity : m_ActiveScene->GetEntities())
			{
				if (ImGui::Selectable(entity->GetName().c_str(), m_SelectedEntity == entity.get()))
				{
					m_SelectedEntity = entity.get(); // Update the selected entity
				}


				// Display Entity Node
				if (ImGui::TreeNode(entity->GetName().c_str()))
				{
					
					ImGui::TreePop();
				}
			}

			
		}
		
		if (ImGui::Button("+")) 
		{
			if (m_ActiveScene)
			{
				std::string name = "Entity" + std::to_string(m_ActiveScene->GetEntities().size());
				m_ActiveScene->CreateEntity(name);
			}
			
		}
	}
	ImGui::End();
}

void EditorLayer::ShowAddComponentWidget() 
{
	// Combo Boxes are also called "Dropdown" in other systems
		// Expose flags as checkbox for the demo
	//static ImGuiComboFlags flags = 0;
	//ImGui::CheckboxFlags("ImGuiComboFlags_PopupAlignLeft", &flags, ImGuiComboFlags_PopupAlignLeft);
	//ImGui::SameLine(); 
	//if (ImGui::CheckboxFlags("ImGuiComboFlags_NoArrowButton", &flags, ImGuiComboFlags_NoArrowButton))
	//	flags &= ~ImGuiComboFlags_NoPreview;     // Clear the other flag, as we cannot combine both
	//if (ImGui::CheckboxFlags("ImGuiComboFlags_NoPreview", &flags, ImGuiComboFlags_NoPreview))
	//	flags &= ~ImGuiComboFlags_NoArrowButton; // Clear the other flag, as we cannot combine both

	


	if (ImGui::BeginCombo("Add Component", currentComponent >= 0 ? COMPONENT_TYPES[currentComponent] : "None"))
	{
		for (int i = 0; i < IM_ARRAYSIZE(COMPONENT_TYPES); i++)
		{
			bool isSelected = (currentComponent == i);
			if (ImGui::Selectable(COMPONENT_TYPES[i], isSelected))
			{
				currentComponent = i;
				showComponentCombo = false;
				ImGui::CloseCurrentPopup();
			}
			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
		

	if (currentComponent >= 0) 
	{
		if (ImGui::Button("Add")) 
		{
			// Add the selected component type to the entity
			if (currentComponent == 0) // TransformComponent
			{
				if (!m_SelectedEntity->Contains<Magnefu::TransformComponent>())
				{
					m_SelectedEntity->AddComponent<Magnefu::TransformComponent>();
				}
			}
			else if (currentComponent == 1)  // MeshComponent
			{ 
				if (!m_SelectedEntity->Contains<Magnefu::MeshComponent>())
				{
					ShowMeshComponentWidget();
				}
			}
			currentComponent = -1; // Reset selection
		}

		if (ImGui::Button("Cancel")) 
		{
			currentComponent = -1; // Reset selection on cancel
			showComponentCombo = false; 
		}
	}
}

void EditorLayer::ShowMeshComponentWidget() 
{
	static int currentMesh = -1; // -1: no selection

	// Assuming you have a function to get all mesh names
	//std::vector<std::string> meshNames = GetAllMeshNames();
	std::vector<std::string> meshNames = { "Corridor", "SantaHat", "Cat", "TV", "Tower", "VictorianPainting" };

	if (ImGui::BeginCombo("Select Mesh", currentMesh >= 0 ? meshNames[currentMesh].c_str() : "None")) 
	{
		for (int i = 0; i < meshNames.size(); i++) 
		{
			bool isSelected = (currentMesh == i);
			if (ImGui::Selectable(meshNames[i].c_str(), isSelected)) 
			{
				currentMesh = i;
			}
			if (isSelected) 
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (currentMesh >= 0) 
	{
		if (ImGui::Button("Add MeshComponent")) 
		{
			// Add MeshComponent to the entity with the selected mesh
			m_SelectedEntity->AddComponent<Magnefu::MeshComponent>(meshNames[currentMesh]);
			currentMesh = -1; // Reset selection
		}
	}
}



void EditorLayer::ShowComponentWindow()
{
	if (ImGui::Begin("Components")) 
	{
		if (m_SelectedEntity)
		{
			// TODO: Entity needs a GetComponents() function
			// or does it?
			
			
			// Display details for each component of the entity
			if (Magnefu::TransformComponent* transform = static_cast<Magnefu::TransformComponent*>(m_SelectedEntity->GetTransformComponent()))
			{
				// Display transform component details
			}
			if (Magnefu::MeshComponent* transform = static_cast<Magnefu::MeshComponent*>(m_SelectedEntity->GetMeshComponent()))
			{
				// Display mesh component details
				
			}

			if (ImGui::Button("Add Component"))
			{
				showComponentCombo = true; // Set flag to open the combo box
				
			}
		}
		else
		{
			ImGui::Text("No Entity Selected");
		}
		
		
	}
	ImGui::End();
}
