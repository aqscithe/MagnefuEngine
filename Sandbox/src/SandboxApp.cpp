#include "Magnefu.h"
#include "SandboxApp.h"



Sandbox::Sandbox()
{
	PushLayer(new GameLayer());
}

Sandbox::~Sandbox()
{
	
}


Magnefu::Application* Magnefu::CreateApplication()
{
	return new Sandbox();
}