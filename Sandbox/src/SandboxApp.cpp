#include "SandboxApp.h"


Sandbox::Sandbox()
{
}

Sandbox::~Sandbox()
{
}

std::unique_ptr<Magnefu::Application> Magnefu::CreateApplication()
{
	return std::make_unique<Sandbox>();
}