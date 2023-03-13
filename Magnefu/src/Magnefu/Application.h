#pragma once

#include "Core.h"
#include "Events/Event.h"
#include "Window.h"
#include <memory>

namespace Magnefu
{
	class MAGNEFU_API Application
	{
	public:
		Application();
		virtual ~Application();

		virtual void Run();

	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running;
		
	};

	// to be defined in client
	std::unique_ptr<Application> CreateApplication();
}