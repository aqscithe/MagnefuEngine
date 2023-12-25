// -- PCH -- //
#include "mfpch.h"

// -- HEADER -- //
#include "WindowsWindow.h"

// -- Application Includes ------------------------ //
#include "Magnefu/Application/Events/ApplicationEvent.h"
#include "Magnefu/Application/Events/MouseEvent.h"
#include "Magnefu/Application/Events/KeyEvent.h"
#include "Magnefu/Application/Events/KeyCodes.h"
#include "Magnefu/Application/Camera/Camera.h"


// -- Graphics Includes ---------------------- //
#include "Magnefu/Graphics/GraphicsContext.h"


// -- Vendor Includes ------------------------- //
#include "imgui.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace Magnefu
{

	/*WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}


	void WindowsWindow::OnUpdate()
	{
		MF_PROFILE_FUNCTION();
		MouseUpdates();
		processInput();
		glfwPollEvents();
		
	}


	void WindowsWindow::processInput()
	{
		if (glfwGetKey((GLFWwindow*)m_Window, MF_KEY_ESCAPE) == GLFW_PRESS)
		{
			WindowCloseEvent event;
			m_Data.EventCallback(event);
		}
			
		m_Mouse.flightMode = (glfwGetMouseButton((GLFWwindow*)m_Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS || glfwGetMouseButton((GLFWwindow*)m_Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_REPEAT);
		if (m_Mouse.flightMode)
			glfwSetInputMode((GLFWwindow*)m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else
			glfwSetInputMode((GLFWwindow*)m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	void WindowsWindow::MouseUpdates()
	{

		double newMouseX, newMouseY;
		glfwGetCursorPos((GLFWwindow*)m_Window, &newMouseX, &newMouseY);
		m_Mouse.DeltaX = (float)(newMouseX - m_Mouse.X);
		m_Mouse.DeltaY = (float)(newMouseY - m_Mouse.Y);
		m_Mouse.X = newMouseX;
		m_Mouse.Y = newMouseY;

		m_Mouse.DeltaX *= m_Mouse.sensitivity;
		m_Mouse.DeltaY *= m_Mouse.sensitivity;


		if (!m_Mouse.flightMode || !m_SceneCamera) return;

		auto& camData = m_SceneCamera->GetData();

		camData.Yaw += m_Mouse.DeltaX;
		camData.Pitch += m_Mouse.DeltaY;

		if (camData.Pitch > Maths::toRadians(89.0f))
			camData.Pitch = Maths::toRadians(89.0f);
		if (camData.Pitch < Maths::toRadians(-89.0f))
			camData.Pitch = Maths::toRadians(-89.0f);
	}*/


}