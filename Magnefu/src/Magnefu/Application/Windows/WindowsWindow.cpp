// -- PCH -- //
#include "mfpch.h"

// -- HEADER -- //
#include "WindowsWindow.h"

// -- Application Includes ------------------------ //
#include "Magnefu/Application/Events/ApplicationEvent.h"
#include "Magnefu/Application/Events/MouseEvent.h"
#include "Magnefu/Application/Events/KeyEvent.h"
#include "Magnefu/Application/Input/KeyCodes.h"
#include "Magnefu/Application/Camera/Camera.h"

// -- Graphics Includes ---------------------- //
#include "Magnefu/Graphics/GraphicsContext.h"

// -- Core -------------------------------- //
#include "Magnefu/Core/Numerics.hpp"


// -- Vendor Includes ------------------------- //
#include "imgui.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace Magnefu
{
	static GLFWwindow* s_Window = nullptr;

	static bool s_GLFWInitialized = false;

	static void GLFWErrorCallback(int code, const char* msg)
	{
		MF_CORE_ERROR("GLFW Error: {0} | {1}", code, msg);
	}


	static f32 GLFWGetMonitorRefresh()
	{
		/*SDL_DisplayMode current;
		int should_be_zero = SDL_GetCurrentDisplayMode(0, &current);
		RASSERT(!should_be_zero);
		return 1.0f / current.refresh_rate;*/

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		int refreshRate = mode->refreshRate;
		return 1.0f / refreshRate;

	}

	void WindowsWindow::Init(void* configuration)
	{

		// -- Create GLFW Window ------------------------------------------------------------ //
		WindowConfiguration& config = *(WindowConfiguration*)configuration;

		MF_CORE_INFO("Launching {0} Window: {1} X {2} ", config.name, config.width, config.height);

		if (!s_GLFWInitialized)
		{
			int success = glfwInit();
			MF_CORE_ASSERT(success, "Failed to initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
			s_GLFWInitialized = true;
		}


		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);


		s_Window = glfwCreateWindow(config.width, config.height, config.name, NULL, NULL);
		MF_CORE_ASSERT(s_Window, "Failed to create GLFW window");

		platform_handle = s_Window;

		int width_, height_;
		//glfwGetWindowSize(s_Window, &width_, &height_);
		glfwGetFramebufferSize(s_Window, &width_, &height_);
		height = height_;
		width = width_;

		// -- GLFW Window User Pointer for use in callback functions ------------------ //

		glfwSetWindowUserPointer(s_Window, &window_data);


		// Set GLFW Callbacks

		glfwSetFramebufferSizeCallback(s_Window, [](GLFWwindow* window, int width, int height)
			{
				WindowUserPointer& data = *(WindowUserPointer*)glfwGetWindowUserPointer(window);



		//MF_CORE_DEBUG("A framebuffer resize event: {} x {}", width, height);
			});


		glfwSetWindowSizeCallback(s_Window, [](GLFWwindow* window, int width, int height)
			{
				WindowUserPointer& data = *(WindowUserPointer*)glfwGetWindowUserPointer(window);
		data.Width = width;
		data.Height = height;

		WindowResizeEvent event(width, height);
		data.EventCallback(event); // What does this line actually do?


		//MF_CORE_DEBUG("A window resize event {} x {}", width, height);

			});

		glfwSetWindowCloseCallback(s_Window, [](GLFWwindow* window)
			{
				WindowUserPointer& data = *(WindowUserPointer*)glfwGetWindowUserPointer(window);

		WindowCloseEvent event;
		data.EventCallback(event);

		MF_CORE_DEBUG("A window close event");

			});

		glfwSetWindowFocusCallback(s_Window, [](GLFWwindow* window, int focused)
			{
				WindowUserPointer& data = *(WindowUserPointer*)glfwGetWindowUserPointer(window);

		if (focused)
		{
			WindowFocusEvent event;
			data.EventCallback(event);

		}
		else
		{
			WindowLostFocusEvent event;
			data.EventCallback(event);

		}

			});

		glfwSetWindowPosCallback(s_Window, [](GLFWwindow* window, int xpos, int ypos)
			{
				WindowUserPointer& data = *(WindowUserPointer*)glfwGetWindowUserPointer(window);
		WindowMovedEvent event(xpos, ypos);
		data.EventCallback(event);

			});

		glfwSetKeyCallback(s_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowUserPointer& data = *(WindowUserPointer*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, scancode, action, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					// GLFW doesn't provide a repeat count
					// 1 to indicate repeated event (key was held down)
					KeyPressedEvent event(key, scancode, action, 1);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key, scancode, action);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetMouseButtonCallback(s_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowUserPointer& data = *(WindowUserPointer*)glfwGetWindowUserPointer(window);

		switch (action)
		{
		case GLFW_PRESS:
		{
			MouseButtonPressedEvent event(button, 0);
			data.EventCallback(event);
			break;
		}
		case GLFW_REPEAT:
		{
			// GLFW doesn't provide a repeat count
			MouseButtonPressedEvent event(button, 1);
			data.EventCallback(event);
			break;
		}
		case GLFW_RELEASE:
		{
			MouseButtonReleasedEvent event(button);
			data.EventCallback(event);
			break;
		}
		}
			});

		glfwSetCursorPosCallback(s_Window, [](GLFWwindow* window, double xpos, double ypos)
			{
				WindowUserPointer& data = *(WindowUserPointer*)glfwGetWindowUserPointer(window);
		MouseMovedEvent event((float)xpos, (float)ypos);
		data.EventCallback(event);

			});

		glfwSetScrollCallback(s_Window, [](GLFWwindow* window, double xoffset, double yoffset)
			{
				WindowUserPointer& data = *(WindowUserPointer*)glfwGetWindowUserPointer(window);
		MouseScrolledEvent event((float)xoffset, (float)yoffset);
		data.EventCallback(event);

			});

		display_refresh = GLFWGetMonitorRefresh();


	}

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(s_Window);
		glfwTerminate();
	}




	void WindowsWindow::SetFullscreen(bool enableFullscreen)
	{
		static int windowed_xpos, windowed_ypos, windowed_width, windowed_height;

		if (enableFullscreen)
		{
			glfwGetWindowPos(s_Window, &windowed_xpos, &windowed_ypos);
			glfwGetWindowSize(s_Window, &windowed_width, &windowed_height);

			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);

			glfwSetWindowMonitor(s_Window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		else
		{
			glfwSetWindowMonitor(s_Window, NULL, windowed_xpos, windowed_ypos, windowed_width, windowed_height, GLFW_DONT_CARE);
		}
	}

	void WindowsWindow::CenterMouse(bool dragging)
	{
		if (dragging)
		{
			glfwSetCursorPos(s_Window, Magnefu::roundu32(width / 2.f), Magnefu::roundu32(height / 2.f));
			glfwSetInputMode(s_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else
		{
			glfwSetInputMode(s_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	void WindowsWindow::PollEvents()
	{
		glfwPollEvents();
		
	}


	/*


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