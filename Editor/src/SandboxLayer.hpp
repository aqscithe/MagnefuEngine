#pragma once

#include "Magnefu/Application/Events/Event.h"
#include "Magnefu/Application/Layer.h"


class SandboxLayer : public Magnefu::Layer
{
public:
	SandboxLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(double deltaTime) override;
	void OnRender() override;
	void OnGUIRender() override;
	void OnEvent(Magnefu::Event& e);

private:
	void ShowApplicationMenuBar();
	void ShowNewSceneDialog();
	//void ShowCameraSettingsWindow();
	void ShowRendererSettingsWindow();
	void ShowControlsWindow();
	void ShowFramerateOverlay();
	void ShowScene();
	//void ShowResourceBrowser();
	//void ShowEntityListWindow();
	//void ShowComponentWindow();
	//void ShowAddComponentWidget();
	//void ShowMeshComponentWidget();
	//void ShowMemoryStats();

private:
	//void CreateNewScene();




private:
	//Magnefu::Ref<Magnefu::SceneCamera>                  m_Camera;
	//Magnefu::GraphicsContext* m_GraphicsContext;
	////Magnefu::PushConstants                              m_PushConstants;
	////Array<Magnefu::SceneObject>& m_SceneObjects;
	//int                                                 m_AreaLightCount;
	//Maths::mat4                                         m_AreaLightPoints;
	//std::array<Magnefu::AreaLight, Magnefu::MAX_AREA_LIGHTS>  m_AreaLights;

	//std::filesystem::path current_path;

	//Magnefu::Scene* m_ActiveScene;
	//Magnefu::Entity* m_SelectedEntity;
};