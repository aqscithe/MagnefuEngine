#include "mfpch.h"
#include "OpenGLContext.h"

#include "Magnefu/Core/Log.h"

#include "GLFW/glfw3.h"
#include "GLAD/glad.h"

namespace Magnefu
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) :
		m_WindowHandle(windowHandle)
	{
		MF_CORE_ASSERT(m_WindowHandle, "Window Handle is null!");
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		int success = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		MF_CORE_ASSERT(success, "Failed to initialize GLAD");

		MF_CORE_DEBUG((const char*)glGetString(GL_VERSION));
		MF_CORE_DEBUG((const char*)glGetString(GL_VENDOR));
		MF_CORE_DEBUG((const char*)glGetString(GL_RENDERER));
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
}