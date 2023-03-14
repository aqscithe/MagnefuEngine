#include "SandboxApp.h"

class ExampleLayer : public Magnefu::Layer
{
public:
	ExampleLayer() :
		Layer("Example")
	{}

	void OnUpdate() override
	{
		//MF_INFO("Example Layer Update");
	}

	void OnEvent(Magnefu::Event& e) override
	{
		MF_TRACE("{0}", e);
	}
};


Sandbox::Sandbox()
{
	PushLayer(new ExampleLayer());
}

Sandbox::~Sandbox()
{

}

std::unique_ptr<Magnefu::Application> Magnefu::CreateApplication()
{
	return std::make_unique<Sandbox>();
}