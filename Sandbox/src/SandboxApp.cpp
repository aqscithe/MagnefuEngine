#include "Magnefu.h"
#include "SandboxApp.h"


Sandbox::Sandbox()
{
	MF_INFO("HELLO FROM SANDBOX APP");
	
}

Sandbox::~Sandbox()
{

}

Magnefu::Application* Magnefu::CreateApplication()
{
	return new Sandbox();
}