#pragma once

// -- Application Includes ---------//
#include "Events/ApplicationEvent.h"
#include "Events/MouseEvent.h"
#include "Events/KeyEvent.h"
#include "Windows/Window.h"
#include "LayerStack.h"
#include "ImGui/ImGuiService.hpp"
#include "Scene/SceneManager.h"


// -- Graphics Includes --------------------- //


// -- Core Includes ---------------------------------- //




namespace Magnefu
{
	struct Draw
	{
		
	};

	class  Application
	{
	public:
		Application();
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		virtual ~Application();

		virtual void Run();

		void OnEvent(Event& event);
		void OnUpdate(float deltaTime);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline Window& GetWindow() { return *m_Window; }

		// -- Managers -- //


		inline SceneManager& GetSceneManager() { return *m_SceneManager; }

		// -- Services -- //
		inline MemoryService& GetMemoryService() { return *MemoryService::Instance(); }
		inline GraphicsContext& GetGraphicsContext() { return *GraphicsContext::Instance(); }


		// -- SCENES -- //
		


		inline static Application& Get() { return *s_Instance; }

		

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	

	private:

		// Try not to have any Array<T> members in this class as
		// the memory service is initialized in its constructor
		// but any arrays' default constructor will need its memory
		// allocator. class member constructors are called
		// BEFORE their owning object's constructor is called

		static Application* s_Instance;

		LayerStack m_LayerStack;
		Scope<Window> m_Window;

		Scope<SceneManager> m_SceneManager;

		bool m_Running;
		bool m_Minimized;


		
	};

	// to be defined in client
	Application* CreateApplication();
}