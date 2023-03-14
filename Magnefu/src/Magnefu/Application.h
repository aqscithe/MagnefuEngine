#pragma once

#include "Core.h"
#include "Window.h"
#include "Magnefu/LayerStack.h"
#include "Magnefu/Events/ApplicationEvent.h"
#include "Magnefu/Events/MouseEvent.h"
#include "Magnefu/Events/KeyEvent.h"

#include <memory>

namespace Magnefu
{
	class MAGNEFU_API Application
	{
	public:
		Application();
		virtual ~Application();

		virtual void Run();

		void OnEvent(Event& event);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

	private:
		bool OnWindowClose(WindowCloseEvent& event);

		std::unique_ptr<Window> m_Window;
		bool m_Running;
		LayerStack m_LayerStack;
	};

	// to be defined in client
	std::unique_ptr<Application> CreateApplication();
}