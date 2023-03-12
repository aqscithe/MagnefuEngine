#include "mfpch.h"

#include "TestClearColor.h"

#include <GL/glew.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

namespace Magnefu
{
	TestClearColor::TestClearColor()
		: m_ClearColor {0.f, 0.5f, 0.8f, 1.f}
	{
	}

	TestClearColor::~TestClearColor()
	{
		
	}

	void TestClearColor::OnUpdate(GLFWwindow* window, float deltaTime)
	{

	}

	void TestClearColor::OnRender()
	{
		glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void TestClearColor::OnImGUIRender()
	{
		ImGui::ColorEdit4("Clear Color", m_ClearColor.e);
	}
}