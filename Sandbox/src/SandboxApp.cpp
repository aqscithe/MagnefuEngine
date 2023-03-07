#include "SandboxApp.h"

#include <memory>

Sandbox::Sandbox()
{
}

Sandbox::~Sandbox()
{
}

//void Sandbox::Run()
//{
//}

std::unique_ptr<Magnefu::Application> Magnefu::CreateApplication()
{
	return std::make_unique<Sandbox>();
}