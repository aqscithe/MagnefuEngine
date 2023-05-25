#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <string>

namespace Magnefu
{
	using String = std::string;

	struct RendererInfo
	{
		String Version;
		String Vendor;
		String Renderer;
	};

	class GraphicsContext
	{
	public:
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
		virtual void DrawFrame() = 0;
		virtual void OnImGuiRender() = 0;
		virtual void GetImGuiInitData() = 0;

		static GraphicsContext* Create(GLFWwindow* windowHandle);

	private:

	};
}