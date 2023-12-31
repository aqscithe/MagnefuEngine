#pragma once

#include "Magnefu/Application/Events/Event.h"
#include "Magnefu/Application/Layer.h"

// -- Bascially everything menu related is in the overlay.
// The overlay layer is for everything that should be for things that should
// consume events first, should a mouse click occur


class Overlay : public Magnefu::Layer
{
public:
	Overlay();

	void OnAttach() override;
	void OnDetach() override;
	void OnEvent(Magnefu::Event& e) override;

	void Update(float deltatime) override;
	void DrawGUI() override;



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

	void BlockEvents(bool block) { m_block_events = block; }


private:
	//Magnefu::Ref<Magnefu::SceneCamera>                  m_Camera;
	//Magnefu::GraphicsContext* m_GraphicsContext;
	////Magnefu::PushConstants                              m_PushConstants;

	//std::filesystem::path current_path;

	//Magnefu::Scene* m_ActiveScene;
	//Magnefu::Entity* m_SelectedEntity;

	bool m_block_events = true;
};