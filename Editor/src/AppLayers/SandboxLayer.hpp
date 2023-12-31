#pragma once

#include "Magnefu/Application/Events/Event.h"
#include "Magnefu/Application/Layer.h"


class SandboxLayer : public Magnefu::Layer
{
public:
	SandboxLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnEvent(Magnefu::Event& e) override;

	void Update(float deltatime) override;
	void DrawGUI() override;




private:
	//Magnefu::Ref<Magnefu::SceneCamera>                  m_Camera;
	//Magnefu::GraphicsContext* m_GraphicsContext;
	//int                                                 m_AreaLightCount;
	//Maths::mat4                                         m_AreaLightPoints;
	//std::array<Magnefu::AreaLight, Magnefu::MAX_AREA_LIGHTS>  m_AreaLights;
};