#include "mfpch.h"

#include "Window.h"

#include <GLFW/glfw3.h>

namespace Magnefu
{
	static GLFWwindow* s_Window = nullptr;

	static bool s_GLFWInitialized = false;

	static void GLFWErrorCallback(int code, const char* msg)
	{
		MF_CORE_ERROR("GLFW Error: {0} | {1}", code, msg);
	}

	static f32 glfw_get_monitor_refresh() 
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


		m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), NULL, NULL);
		MF_CORE_ASSERT(m_Window, "Failed to create GLFW window");
	}

	void Window::Shutdown()
	{
	}

} // namespace Magnefu