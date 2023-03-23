#include "SandboxApp.h"


Sandbox::Sandbox()
{
}

Sandbox::~Sandbox()
{

}

Magnefu::Application* Magnefu::CreateApplication()
{
	return new Sandbox();
}