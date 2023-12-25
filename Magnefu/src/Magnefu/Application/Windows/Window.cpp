#include "mfpch.h"

#include "Window.h"

// -- Application Includes -------------------------------- //
#include "Magnefu/Application/Events/ApplicationEvent.h"
#include "Magnefu/Application/Events/MouseEvent.h"
#include "Magnefu/Application/Events/KeyEvent.h"
#include "Magnefu/Application/Events/KeyCodes.h"


// -- Core -------------------------- //
#include "Magnefu/Core/Numerics.hpp"

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

	void Window::Init(void* configuration)
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

		

			MF_CORE_DEBUG("A framebuffer resize event");
		});


		glfwSetWindowSizeCallback(s_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowUserPointer& data = *(WindowUserPointer*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event); // What does this line actually do?


			MF_CORE_DEBUG("A window resize event {} x {}", width, height);

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
				KeyPressedEvent event(key, 0);
				data.EventCallback(event);
				break;
			}
			case GLFW_REPEAT:
			{
				// GLFW doesn't provide a repeat count
				KeyPressedEvent event(key, 1);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(key);
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

	void Window::Shutdown()
	{
		glfwDestroyWindow(s_Window);
		glfwTerminate();
	}


	

	void Window::SetFullscreen(bool enableFullscreen)
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

	void Window::CenterMouse(bool dragging)
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

} // namespace Magnefu