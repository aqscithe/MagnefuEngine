#include "Overlay.hpp"
#include "Magnefu/Application/Events/Event.h"
#include "imgui/imgui.h"


// should put imgui editor flags in a struct
//static bool show_demo_window = true;
static const char* COMPONENT_TYPES[] = { "TransformComponent", "MeshComponent" };
static int currentComponent = -1; // -1: no selection, 0: MeshComponent, 1: TransformComponent
static bool showComponentCombo = false; // Flag to control visibility of the combo box
static bool showMeshComponentWidget = false;
static bool openNewSceneDialog = false;
static bool openCameraSettingsWindow = false;
static bool openRendererSettingsWindow = false;
static bool openControlsWindow = false;
static bool showFramerate = false;
static bool showMemoryStatsWindow = false;
static int duplicateNameAppend = 0;
static char sceneNameBuffer[128] = "";
static const uint32_t MAX_SCENES = 1;

static double s_deltaTime = 1.0;

Overlay::Overlay() :
	Layer("Overlay")
{
	//auto& scenes = app.GetScenes();



	//// All of this should be within an ImGui editor //
	//Magnefu::Scene& scene = app.CreateScene();
	//Magnefu::Entity& entity = scene->CreateEntity();

	//// I should only need to pass the name as an argument for meshcomponent. The other values should be fetched behind the scenes
	//Magnefu::MeshComponent& mesh = entity.AddComponent<Magnefu::MeshComponent>(Mesh_Name);
	//Magnefu::TransformComponent transform = entity.AddComponent<Magnefu::TransformComponent>(Maths::vec3(0.f), Maths::vec3(0.f), Maths::vec3(1.f));


}

void Overlay::OnAttach()
{
	/*m_Camera->SetDefaultProps();*/
}

void Overlay::OnDetach()
{

}



void Overlay::OnEvent(Magnefu::Event& e)
{
	auto type = e.GetEventType();

	switch (type)
	{
	case Magnefu::EventType::None:
		break;
	case Magnefu::EventType::WindowClose:
		break;
	case Magnefu::EventType::WindowResize:
		break;
	case Magnefu::EventType::WindowFocus:
		break;
	case Magnefu::EventType::WindowLostFocus:
		break;
	case Magnefu::EventType::WindowMoved:
		break;
	case Magnefu::EventType::AppTick:
		break;
	case Magnefu::EventType::AppUpdate:
		break;
	case Magnefu::EventType::AppRender:
		break;
	case Magnefu::EventType::KeyPressed:
		break;
	case Magnefu::EventType::KeyReleased:
		break;
	case Magnefu::EventType::MouseButtonPressed:
		break;
	case Magnefu::EventType::MouseButtonReleased:
		break;
	case Magnefu::EventType::MouseMoved:
		break;
	case Magnefu::EventType::MouseScrolled:
		break;
	default:
		break;
	}


	// Setting certain events as handled by the overlay
	// For example, if the menu is showing and the user clicks on a button,
	// there is no need for the in-game character to perform an action

	if (m_block_events)
	{
		ImGuiIO& io = ImGui::GetIO();
		e.m_Handled |= e.IsInCategory(Magnefu::EventCategoryMouse) & io.WantCaptureMouse;
		e.m_Handled |= e.IsInCategory(Magnefu::EventCategoryKeyboard) & io.WantCaptureKeyboard;

		//MF_CORE_DEBUG("{}", e.ToString());
	}
}



void Overlay::Update(float deltatime)
{


}



void Overlay::DrawGUI()
{
	/*
	if (show_demo_window)
	{
		ImGui::ShowDemoWindow(&show_demo_window);
	}*/


	/*ShowApplicationMenuBar();
	ShowNewSceneDialog();
	ShowCameraSettingsWindow();
	ShowRendererSettingsWindow();
	ShowControlsWindow();
	ShowFramerateOverlay();
	ShowMemoryStats();

	if (m_ActiveScene)
	{
		ShowScene();
	}*/
}



void Overlay::ShowApplicationMenuBar()
{
	if (ImGui::BeginMainMenuBar()) {
		// File menu
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("Project"))
			{
				ImGui::MenuItem("New Project", NULL, false, true);
				ImGui::MenuItem("Save Project", NULL, false, true);
				ImGui::MenuItem("Open Project", NULL, false, true);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Scene"))
			{
				if (ImGui::MenuItem("New Scene", NULL, false, true))
				{
					openNewSceneDialog = true;
				}
				ImGui::MenuItem("Save Scene", NULL, false, true);
				ImGui::MenuItem("Open Scene", NULL, false, true);
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Exit", "Esc", false, true))
			{
				//Magnefu::Application::Get().GetWindow().CloseWindow();
			}
			ImGui::EndMenu();
		}

		// Display menu
		if (ImGui::BeginMenu("Display"))
		{
			if (ImGui::MenuItem("Camera", NULL, false, true))
			{
				openCameraSettingsWindow = true;
			}
			if (ImGui::MenuItem("Renderer", NULL, false, true))
			{
				openRendererSettingsWindow = true;
			}
			if (ImGui::MenuItem("Toggle Framerate", NULL, false, true))
			{
				showFramerate = !showFramerate;
			}
			ImGui::EndMenu();
		}

		// Tools menu
		if (ImGui::BeginMenu("Tools"))
		{
			// Memory
			if (ImGui::BeginMenu("Memory"))
			{
				if (ImGui::MenuItem("Memory Stats", NULL, false, true))
				{
					showMemoryStatsWindow = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		// Options menu
		if (ImGui::BeginMenu("Options"))
		{
			// Add options items here
			if (ImGui::MenuItem("Controls", NULL, false, true))
			{
				openControlsWindow = true;
			}
			ImGui::EndMenu();
		}

		// Help menu
		if (ImGui::BeginMenu("Help")) {
			// Add help items here
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void Overlay::ShowNewSceneDialog()
{

	if (openNewSceneDialog)
	{
		ImGui::OpenPopup("New Scene");
		openNewSceneDialog = false;
	}

	if (ImGui::BeginPopupModal("New Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::InputText("Scene Name", sceneNameBuffer, IM_ARRAYSIZE(sceneNameBuffer));

		if (ImGui::Button("OK"))
		{
			//CreateNewScene();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

//void Overlay::ShowCameraSettingsWindow()
//{
//	if (!openCameraSettingsWindow) { return; }
//
//	auto& camera = Magnefu::Application::Get().GetWindow().GetSceneCamera();
//
//	if (ImGui::Begin("Camera Details", &openCameraSettingsWindow, ImGuiWindowFlags_NoCollapse))
//	{
//		ImGui::SeparatorText("MATRICES");
//
//		ImGui::Text("View");
//		ImGui::Text("\t%.1f\t%.1f\t%.1f\t%.1f", camera->GetView().e[0], camera->GetView().e[1], camera->GetView().e[2], camera->GetView().e[3]);
//		ImGui::Text("\t%.1f\t%.1f\t%.1f\t%.1f", camera->GetView().e[4], camera->GetView().e[5], camera->GetView().e[6], camera->GetView().e[7]);
//		ImGui::Text("\t%.1f\t%.1f\t%.1f\t%.1f", camera->GetView().e[8], camera->GetView().e[9], camera->GetView().e[10], camera->GetView().e[11]);
//		ImGui::Text("\t%.1f\t%.1f\t%.1f\t%.1f", camera->GetView().e[12], camera->GetView().e[13], camera->GetView().e[14], camera->GetView().e[15]);
//
//		ImGui::SeparatorText("ORIENTATION");
//
//		ImGui::SliderFloat3("Position", camera->GetData().Position.e, -10.f, 10.f);
//		ImGui::Text("Yaw: %.2f", camera->GetData().Yaw);
//		ImGui::Text("Pitch: %.2f", camera->GetData().Pitch);
//
//		ImGui::SeparatorText("PROPERTIES");
//
//
//		ImGui::Text("Type: ");
//		if (ImGui::Selectable("\tPerspective", camera->IsPerspective()))
//		{
//			camera->GetData().Type = Magnefu::CameraType::Perspective;
//			camera->GetData().Near = 0.01f;
//			camera->SetOrtho(false);
//		}
//
//		if (ImGui::Selectable("\tOrthographic", camera->IsOrtho()))
//		{
//			camera->GetData().Type = Magnefu::CameraType::Orthographic;
//			camera->GetData().Near = 2.97f;
//			camera->SetPerspective(false);
//		}
//
//		ImGui::Text("Aspect Ration: %f", &camera->GetData().AspectRatio);
//		ImGui::SliderFloat("FOV", &camera->GetData().FOV, 45.f, 100.f);
//		ImGui::SliderFloat("Near", &camera->GetData().Near, 0.01f, 10.f);
//		ImGui::SliderFloat("Far", &camera->GetData().Far, 700.f, 1000.f);
//		ImGui::SliderFloat("Speed", &camera->GetData().Speed, 15.f, 100.f);
//		ImGui::SliderFloat3("Position", camera->GetData().Position.e, -500.f, 500.f);
//		ImGui::SliderFloat("Pitch", &camera->GetData().Pitch, -360.f, 360.f);
//		ImGui::SliderFloat("Yaw", &camera->GetData().Yaw, -360.f, 360.f);
//
//		ImGui::End();
//	}
//
//}

void Overlay::ShowRendererSettingsWindow()
{
	//if (!openRendererSettingsWindow) { return; }

	/*GraphicsContext* GraphicsContext = m_Window->GetGraphicsContext();
		ImGui::Begin("Renderer", &openRendererSettingsWindow, ImGuiWindowFlags_NoCollapse);
		ImGui::Text("Renderer: %s", GraphicsContext->GetRendererInfo().Renderer);
		ImGui::Text("Version: %s", GraphicsContext->GetRendererInfo().Version);
		ImGui::Text("Vendor: %s", GraphicsContext->GetRendererInfo().Vendor);
		ImGui::End();*/
}

void Overlay::ShowControlsWindow()
{
	if (!openControlsWindow) { return; }

	if (ImGui::Begin("CONTROLS", &openControlsWindow, ImGuiWindowFlags_NoCollapse))
	{
		ImGui::Text("APP CONTROLS");
		ImGui::Bullet(); ImGui::Text("Close App:      ESC  |");
		ImGui::Separator();
		ImGui::Text("CAMERA CONTROLS");
		ImGui::Bullet(); ImGui::Text("Camera Left:    A    | Left Arrow");
		ImGui::Bullet(); ImGui::Text("Camera Right:   D    | Right Arrow");
		ImGui::Bullet(); ImGui::Text("Camera Forward: W    | Up Arrow");
		ImGui::Bullet(); ImGui::Text("Camera Back:    S    | Down Arrow");
		ImGui::Bullet(); ImGui::Text("Camera Up:      E    |");
		ImGui::Bullet(); ImGui::Text("Camera Down:    Q    |");
		ImGui::Bullet(); ImGui::Text("Camera Rotate:  Right-Click + Move Mouse");
		ImGui::Bullet(); ImGui::Text("Adjust FOV:     Rotate Middle Mouse Button");

		ImGui::End();
	}


}

void Overlay::ShowFramerateOverlay()
{
	if (!showFramerate) { return; }

	const float PAD = 10.0f;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImVec2 work_pos = viewport->WorkPos; // Top-left position of the viewport
	ImVec2 work_size = viewport->WorkSize; // Size of the viewport
	ImVec2 window_pos, window_pos_pivot;
	window_pos.x = work_pos.x + work_size.x - PAD; // Position on the right
	window_pos.y = work_pos.y + PAD; // Position at the top
	window_pos_pivot.x = 1.0f; // Pivot on the right
	window_pos_pivot.y = 0.0f; // Pivot at the top

	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	if (ImGui::Begin("Framerate Overlay", &showFramerate, window_flags))
	{
		// Save the original color
		ImVec4 originalColor = ImGui::GetStyle().Colors[ImGuiCol_Text];

		// Set the new color for text
		ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(0.03f, 0.15f, 0.40f, 1.0f);


		float frameRate = (float)1.f / (s_deltaTime * (1.0 / 1000.0));
		//ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::Text("FPS: %.1f", frameRate);


		//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::GetStyle().Colors[ImGuiCol_Text] = originalColor;

	}
	ImGui::End();
}


void Overlay::ShowScene()
{
	/*ShowEntityListWindow();
	ShowResourceBrowser();

	ShowComponentWindow();

	if (showMeshComponentWidget)
	{
		ShowMeshComponentWidget();
	}*/
}



//void Overlay::ShowResourceBrowser()
//{
//	if (ImGui::Begin("Resource Browser"))
//	{
//
//		static char buffer[256] = "";
//
//		// Button or item to go up one level
//		if (ImGui::Button(".."))
//		{
//			current_path = current_path.parent_path();
//		}
//
//		// Input field for typing out the directory
//		if (ImGui::InputText("Go to", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
//		{
//			std::filesystem::path new_path(buffer);
//			if (std::filesystem::exists(new_path) && std::filesystem::is_directory(new_path))
//			{
//				current_path = new_path;
//			}
//			else
//			{
//				// Handle invalid path
//			}
//		}
//
//		for (const auto& entry : std::filesystem::directory_iterator(current_path))
//		{
//			if (ImGui::Selectable(entry.path().filename().string().c_str()))
//			{
//				if (entry.is_directory())
//				{
//					// Change directory
//					current_path = entry.path();
//				}
//				else
//				{
//					// Handle file selection
//				}
//			}
//		}
//	}
//	ImGui::End();
//}
//
//
//
//
//void Overlay::ShowEntityListWindow()
//{
//	if (ImGui::Begin("Entities"))
//	{
//		if (m_ActiveScene)
//		{
//			for (auto& entity : m_ActiveScene->GetEntities())
//			{
//				if (ImGui::Selectable(entity->GetName(), m_SelectedEntity == entity))
//				{
//					m_SelectedEntity = entity; // Update the selected entity
//				}
//
//
//				// Display Entity Node
//				/*if (ImGui::TreeNode(entity->GetName().c_str()))
//				{
//
//					ImGui::TreePop();
//				}*/
//			}
//
//
//		}
//
//		if (ImGui::Button("+"))
//		{
//			if (m_ActiveScene)
//			{
//				cstring name = "Entity" + m_ActiveScene->GetEntities().count();
//				m_ActiveScene->CreateEntity(name);
//			}
//
//		}
//	}
//	ImGui::End();
//}
//
//void Overlay::ShowAddComponentWidget()
//{
//	// Combo Boxes are also called "Dropdown" in other systems
//		// Expose flags as checkbox for the demo
//	//static ImGuiComboFlags flags = 0;
//	//ImGui::CheckboxFlags("ImGuiComboFlags_PopupAlignLeft", &flags, ImGuiComboFlags_PopupAlignLeft);
//	//ImGui::SameLine(); 
//	//if (ImGui::CheckboxFlags("ImGuiComboFlags_NoArrowButton", &flags, ImGuiComboFlags_NoArrowButton))
//	//	flags &= ~ImGuiComboFlags_NoPreview;     // Clear the other flag, as we cannot combine both
//	//if (ImGui::CheckboxFlags("ImGuiComboFlags_NoPreview", &flags, ImGuiComboFlags_NoPreview))
//	//	flags &= ~ImGuiComboFlags_NoArrowButton; // Clear the other flag, as we cannot combine both
//
//
//	if (ImGui::BeginCombo("Add Component", currentComponent >= 0 ? COMPONENT_TYPES[currentComponent] : "None"))
//	{
//		for (int i = 0; i < IM_ARRAYSIZE(COMPONENT_TYPES); i++)
//		{
//			bool isSelected = (currentComponent == i);
//			if (ImGui::Selectable(COMPONENT_TYPES[i], isSelected))
//			{
//				currentComponent = i;
//			}
//			if (isSelected)
//			{
//				ImGui::SetItemDefaultFocus();
//			}
//		}
//		ImGui::EndCombo();
//	}
//
//
//	if (currentComponent >= 0)
//	{
//		if (ImGui::Button("Add"))
//		{
//			// Add the selected component type to the entity
//			if (currentComponent == 0) // TransformComponent
//			{
//				if (!m_SelectedEntity->Contains<Magnefu::TransformComponent>())
//				{
//					m_SelectedEntity->AddComponent<Magnefu::TransformComponent>();
//				}
//			}
//			else if (currentComponent == 1)  // MeshComponent
//			{
//				showMeshComponentWidget = true;
//			}
//			currentComponent = -1; // Reset selection
//		}
//
//		if (ImGui::Button("Cancel"))
//		{
//			currentComponent = -1; // Reset selection on cancel
//			//showComponentCombo = false; 
//			showMeshComponentWidget = false;
//		}
//	}
//}
//
//void Overlay::ShowMeshComponentWidget()
//{
//	static int currentMesh = -1; // -1: no selection
//
//	// Assuming you have a function to get all mesh names
//	//std::vector<std::string> meshNames = GetAllMeshNames();
//	std::vector<std::string> meshNames = { "Corridor", "SantaHat", "Cat", "TV", "Tower", "VictorianPainting" };
//
//	if (ImGui::BeginCombo("Select Mesh", currentMesh >= 0 ? meshNames[currentMesh].c_str() : "None"))
//	{
//		for (int i = 0; i < meshNames.size(); i++)
//		{
//			bool isSelected = (currentMesh == i);
//			if (ImGui::Selectable(meshNames[i].c_str(), isSelected))
//			{
//				currentMesh = i;
//			}
//			if (isSelected)
//			{
//				ImGui::SetItemDefaultFocus();
//			}
//		}
//		ImGui::EndCombo();
//	}
//
//	if (currentMesh >= 0)
//	{
//		if (ImGui::Button("Add MeshComponent"))
//		{
//			if (!m_SelectedEntity->Contains<Magnefu::MeshComponent>())
//			{
//				// Add MeshComponent to the entity with the selected mesh
//				m_SelectedEntity->AddComponent<Magnefu::MeshComponent>(meshNames[currentMesh]);
//				currentMesh = -1; // Reset selection
//			}
//		}
//	}
//}
//
//
//
//void Overlay::ShowComponentWindow()
//{
//	if (ImGui::Begin("Components"))
//	{
//		if (m_SelectedEntity)
//		{
//			// TODO: Entity needs a GetComponents() function
//			// or does it?
//
//
//			// Display details for each component of the entity
//			if (Magnefu::TransformComponent* transform = static_cast<Magnefu::TransformComponent*>(m_SelectedEntity->GetTransformComponent()))
//			{
//				// Display transform component details
//				ImGui::Text("TRANSFORM");
//				ImGui::SliderFloat3("Position", transform->Position().e, -1000, 1000, "%.2f");
//				ImGui::SliderFloat3("Rotation", transform->Rotation().e, -360, 360, "%.2f");
//				ImGui::SliderFloat3("Scale", transform->Scale().e, 0.1, 100, "%.2f");
//				ImGui::Separator();
//			}
//			if (Magnefu::MeshComponent* transform = static_cast<Magnefu::MeshComponent*>(m_SelectedEntity->GetMeshComponent()))
//			{
//				// Display mesh component details
//				ImGui::Text("MESH");
//				ImGui::Separator();
//			}
//
//			ShowAddComponentWidget();
//
//			//if (ImGui::Button("Add Component"))
//			//{
//			//	showComponentCombo = true; // Set flag to open the combo box
//			//	
//			//}
//		}
//		else
//		{
//			ImGui::Text("No Entity Selected");
//		}
//
//
//	}
//	ImGui::End();
//}
//
//void Overlay::CreateNewScene()
//{
//	auto& sceneManager = Magnefu::Application::Get().GetSceneManager();
//
//	if (sceneManager.GetScenes().count() >= MAX_SCENES)
//	{
//		MF_WARN("Reached Max Scene Count: {}", MAX_SCENES);
//		return;
//	}
//
//	// Handle the creation of the new scene with the name in sceneNameBuffer
//	cstring name = sceneNameBuffer;
//
//	while (sceneManager.NameExists(name))
//	{
//		// Append number to name if it already exists
//		name = sceneNameBuffer;
//		name += duplicateNameAppend;
//		duplicateNameAppend++;
//
//	}
//	duplicateNameAppend = 0;
//
//	m_ActiveScene = sceneManager.CreateScene(name);
//}
//
//void Overlay::ShowMemoryStats()
//{
//	if (!showMemoryStatsWindow) { return; }
//
//	if (ImGui::Begin("Memory Stats", &showMemoryStatsWindow, ImGuiWindowFlags_NoCollapse))
//	{
//		if (ImGui::Button("Update Memory Stats"))
//		{
//			Magnefu::Application::Get().GetGraphicsContext().CalculateMemoryStats();
//		}
//
//		Magnefu::Application::Get().GetMemoryService().imguiDraw();
//
//		auto [blockCount, blockBytes, allocCount, allocBytes, usage, budget] =
//			Magnefu::Application::Get().GetGraphicsContext().GetMemoryStats();
//
//		ImGui::SeparatorText("VMA STATISTICS");
//
//		ImGui::Text("Block Count: %d", blockCount);
//		if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Number of `VkDeviceMemory` objects - Vulkan memory blocks allocated."); }
//
//		ImGui::Text("Block Bytes: %llu", blockBytes);
//		if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Number of bytes allocated in `VkDeviceMemory` blocks."); }
//
//		ImGui::Text("Allocation Count: %d", allocCount);
//		if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Number of #VmaAllocation objects allocated."); }
//
//		ImGui::Text("Allocation Bytes: %llu", allocBytes);
//		if (ImGui::IsItemHovered())
//		{
//			ImGui::SetTooltip("Total number of bytes occupied by all #VmaAllocation objects. \n\
//				Difference `(blockBytes - allocationBytes)` is the amount of memory allocated\n\
//				from Vulkan but unused by any #VmaAllocation.");
//		}
//
//		ImVec4 color = usage > (0.8f * budget) ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
//		ImGui::TextColored(color, "Usage: %llu", usage);
//		if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Estimated current memory usage of the program, in bytes."); }
//
//		ImGui::Text("Budget: %llu", budget);
//		if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Estimated amount of memory available to the program, in bytes."); }
//
//		ImVec4 originalColor = ImGui::GetStyle().Colors[ImGuiCol_PlotHistogram];
//		ImGui::GetStyle().Colors[ImGuiCol_PlotHistogram] = ImVec4(0.2f, 0.9f, 0.4f, 1.0f);
//
//		ImGui::ProgressBar((float)usage / budget, ImVec2(0.0f, 0.0f), "Byte Usage");
//		if (ImGui::IsItemHovered()) { ImGui::SetTooltip("%llu bytes remaining", budget - usage); }
//
//		ImGui::ProgressBar((float)allocBytes / blockBytes, ImVec2(0.0f, 0.0f), "VmaAllocation Byte Usage");
//		if (ImGui::IsItemHovered()) { ImGui::SetTooltip("%d bytes remaining", blockBytes - allocBytes); }
//		ImGui::GetStyle().Colors[ImGuiCol_PlotHistogram] = originalColor;
//
//
//		ImGui::End();
//	}
//}