#include "mfpch.h"
#include "OpenGLContext.h"

#include "Magnefu/Core/Log.h"

#include "GLFW/glfw3.h"
#include "GLAD/glad.h"
#include "imgui.h"

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

		m_RendererInfo.Version = (const char*)glGetString(GL_VERSION);
		m_RendererInfo.Vendor = (const char*)glGetString(GL_VENDOR);
		m_RendererInfo.Renderer = (const char*)glGetString(GL_RENDERER);

		MF_CORE_DEBUG(m_RendererInfo.Version);
		MF_CORE_DEBUG(m_RendererInfo.Vendor);
		MF_CORE_DEBUG(m_RendererInfo.Renderer);
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}

	void OpenGLContext::OnImGuiRender()
	{
		ImGui::Text("Version: %s", m_RendererInfo.Version.c_str());
		ImGui::Text("Vendor: %s", m_RendererInfo.Vendor.c_str());
		ImGui::Text("Renderer: %s", m_RendererInfo.Renderer.c_str());
	}
}