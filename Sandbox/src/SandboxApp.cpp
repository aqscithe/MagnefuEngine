#include "SandboxApp.h"


Sandbox::Sandbox()
{
	//PushOverlay(new Magnefu::ImGuiLayer());
}

Sandbox::~Sandbox()
{

}

Magnefu::Application* Magnefu::CreateApplication()
{
	return new Sandbox();
}