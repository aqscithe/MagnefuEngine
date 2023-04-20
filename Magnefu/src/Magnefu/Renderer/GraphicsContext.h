#pragma once
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
		virtual void OnImGuiRender() = 0;

	private:

	};
}