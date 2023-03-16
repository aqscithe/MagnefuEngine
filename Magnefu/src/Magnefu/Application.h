#pragma once

#include "Core.h"
#include "Window.h"
#include "Magnefu/LayerStack.h"
#include "Magnefu/ImGui/ImGuiLayer.h"
#include "Magnefu/Events/ApplicationEvent.h"
#include "Magnefu/Events/MouseEvent.h"
#include "Magnefu/Events/KeyEvent.h"


namespace Magnefu
{
	class MAGNEFU_API Application
	{
	public:
		Application();
		Application(const Application&) = delete;
		virtual ~Application();

		virtual void Run();

		void OnEvent(Event& event);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline Window& GetWindow() { return *m_Window; }

		inline static Application& Get() { return *s_Instance; }

	private:
		bool OnWindowClose(WindowCloseEvent& event);

		std::unique_ptr<Window> m_Window;
		bool m_Running;
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;

		static Application* s_Instance;
	};

	// to be defined in client
	Application* CreateApplication();
}