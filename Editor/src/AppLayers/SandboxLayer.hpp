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
};