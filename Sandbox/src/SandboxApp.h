#pragma once

#include "Magnefu.h"

#include "imgui/imgui.h"

class TestLayer : public Magnefu::Layer
{
public:
	TestLayer() :
		Layer("Tests")
	{
		m_ActiveTest = nullptr;
		m_TestMenu = nullptr;
	}

	void OnAttach() override
	{
		m_ActiveTest = nullptr;
		m_TestMenu = new Magnefu::TestMenu(m_ActiveTest);

		m_ActiveTest = m_TestMenu;

		m_TestMenu->RegisterTest<Magnefu::Test2DTexture>("2D Texture");
		//m_TestMenu->RegisterTest<Magnefu::Test3DRender>("Cube Render");
		//m_TestMenu->RegisterTest<Magnefu::TestBatchRendering>("Batching");
		//m_TestMenu->RegisterTest<Magnefu::TestLighting>("Lighting");
		//m_TestMenu->RegisterTest <Magnefu::TestModelLoading>("3D Models");
	}

	void OnDetach() override
	{
		if (m_ActiveTest != m_TestMenu)
			delete m_TestMenu;
		delete m_ActiveTest;
	}

	void OnUpdate(float deltaTime) override
	{
		Magnefu::RenderCommand::ClearColor(0.08f, 0.08f, 0.08f, 1.f);
		Magnefu::RenderCommand::Clear();

		if (m_ActiveTest)
		{
			m_ActiveTest->OnUpdate(deltaTime);
			m_ActiveTest->OnRender();
			ImGui::Begin("Tests");
			if (m_ActiveTest != m_TestMenu && ImGui::Button("<-"))
			{
				delete m_ActiveTest;
				m_ActiveTest = m_TestMenu;
			}
			m_ActiveTest->OnImGUIRender();
			ImGui::End();
		}
	}

	void OnEvent(Magnefu::Event& event) {}

private:
	Magnefu::Test* m_ActiveTest;
	Magnefu::TestMenu* m_TestMenu;
};


class Sandbox : public Magnefu::Application
{
public:
	Sandbox();
	~Sandbox();

};