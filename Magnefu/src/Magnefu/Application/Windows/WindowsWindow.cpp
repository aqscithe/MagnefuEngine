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
	static bool s_GLFWInitialized = false;

	static void GLFWErrorCallback(int code, const char* msg)
	{
		MF_CORE_ERROR("GLFW Error: {0} | {1}", code, msg);
	}

	// Handled by Vulkan API
	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) 
	{
		//WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
	}

	Window* Window::Create(const WindowProps& props)
	{
		MF_PROFILE_FUNCTION();
		return new WindowsWindow(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		m_SceneCamera = Ref<Camera>(Camera::Create());

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		m_Data.CamData = &m_SceneCamera->GetData();
		m_Data.WindowPtr = this;

		MF_CORE_INFO("Launching Window: {0} - {1}x{2}", m_Data.Title, m_Data.Width, m_Data.Height);

		if (!s_GLFWInitialized)
		{
			int success = glfwInit();
			MF_CORE_ASSERT(success, "Failed to initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
			s_GLFWInitialized = true;
		}

		
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		

		m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), NULL, NULL);
		MF_CORE_ASSERT(m_Window, "Failed to create GLFW window");

		/*m_Context = GraphicsContext::Create(m_Window);

		if(m_Context)
			m_Context->Init();*/

		// maybe i pass WindowData as an argument to the context init function

		m_Window = GraphicsContext::get_window_handle();

		glfwSetWindowUserPointer((GLFWwindow*)m_Window, &m_Data);

		// vsync set by presentation mode in vulkan swap chain

		// Set GLFW Callbacks

		glfwSetFramebufferSizeCallback((GLFWwindow*)m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			WindowsWindow* win = dynamic_cast<WindowsWindow*>(data.WindowPtr);
			if (win)
				win->SetFramebufferResized(true);

			//MF_CORE_DEBUG("A framebuffer resize event");
		});

		glfwSetWindowSizeCallback((GLFWwindow*)m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);

			//MF_CORE_DEBUG("A window resize event");

		});

		glfwSetWindowCloseCallback((GLFWwindow*)m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			
			WindowCloseEvent event;
			data.EventCallback(event);

		});

		glfwSetKeyCallback((GLFWwindow*)m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);


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

		glfwSetMouseButtonCallback((GLFWwindow*)m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

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

		glfwSetCursorPosCallback((GLFWwindow*)m_Window, [](GLFWwindow* window, double xpos, double ypos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			MouseMovedEvent event((float)xpos, (float)ypos);
			data.EventCallback(event);

		});

		glfwSetScrollCallback((GLFWwindow*)m_Window, [](GLFWwindow* window, double xoffset, double yoffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			MouseScrolledEvent event((float)xoffset, (float)yoffset);
			data.EventCallback(event);

			if (!data.CamData) return;

			auto camData = data.CamData;

			camData->FOV -= (float)yoffset;
			if (camData->FOV < 1.0f)
				camData->FOV = 1.0f;
			if (camData->FOV > 100.f)
				camData->FOV = 100.f;
		});


		m_Mouse.lastX = m_Data.Width / 2.f;
		m_Mouse.lastY = m_Data.Height / 2.f;
		m_Mouse.sensitivity = 0.001f;
		m_Mouse.X = 0.0;
		m_Mouse.Y = 0.0;
		m_Mouse.flightMode = true;
	}

	void WindowsWindow::OnUpdate()
	{
		MF_PROFILE_FUNCTION();
		MouseUpdates();
		processInput();
		glfwPollEvents();
		
		//m_Context->SwapBuffers();
	}

	void WindowsWindow::DrawFrame()
	{
		/*m_Context->DrawFrame();*/
	}

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow((GLFWwindow*)m_Window);
		glfwTerminate();
	}


	void WindowsWindow::SetSceneCamera(const Ref<Camera>& cam)
	{
		m_SceneCamera = cam;
		m_Data.CamData = &m_SceneCamera->GetData();
	}

	void WindowsWindow::SetFramebufferResized(bool framebufferResized)
	{
		/*m_Context->SetFramebufferResized(framebufferResized);*/
	}

	void WindowsWindow::CloseWindow()
	{
		WindowCloseEvent event;
		m_Data.EventCallback(event);
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
	}

	void WindowsWindow::OnImGuiRender()
	{
		
	}

	void WindowsWindow::OnFinish()
	{
		/*m_Context->OnFinish();*/
	}

}