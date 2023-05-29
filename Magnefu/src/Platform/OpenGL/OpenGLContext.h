#pragma once

#include "Magnefu/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Magnefu
{
	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);

		void Init() override;
		void SwapBuffers() override;
		void DrawFrame() override;
		void OnImGuiRender() override;
		void OnFinish() override;
		void GetImGuiInitData() override {int x = 1;}

	private:
		GLFWwindow* m_WindowHandle;
		RendererInfo m_RendererInfo;
	};
}