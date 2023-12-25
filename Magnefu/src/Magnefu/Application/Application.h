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
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	struct ServiceManager;

	struct ApplicationConfiguration
	{

		u32                         width;
		u32                         height;

		cstring                     name = nullptr;

		bool                        init_base_services = false;

		ApplicationConfiguration& w(u32 value) { width = value; return *this; }
		ApplicationConfiguration& h(u32 value) { height = value; return *this; }
		ApplicationConfiguration& name_(cstring value) { name = value; return *this; }

	}; // struct ApplicationConfiguration

	struct Application
	{
		// 
		virtual void                create(const ApplicationConfiguration& configuration) {}
		virtual void                destroy() {}
		virtual bool                main_loop() { return false; }

		// Fixed update. Can be called more than once compared to rendering.
		virtual void                fixed_update(f32 delta) {}
		// Variable time update. Called only once per frame.
		virtual void                variable_update(f32 delta) {}
		// Rendering with optional interpolation factor.
		virtual void                render(f32 interpolation) {}
		// Per frame begin/end.
		virtual void                frame_begin() {}
		virtual void                frame_end() {}

		virtual void				PushLayer(Layer* layer);
		virtual void				PushOverlay(Layer* overlay) {}

		virtual void				OnEvent(Event& event);
		virtual bool				OnWindowClose(WindowCloseEvent& e);
		virtual bool				OnWindowResize(WindowResizeEvent& e);
		virtual bool				OnWindowMoved(WindowMovedEvent& e);
		virtual bool				OnWindowFocus(WindowFocusEvent& e);
		virtual bool				OnWindowLostFocus(WindowLostFocusEvent& e);


		void                        Run(const ApplicationConfiguration& configuration);

	protected:

		ServiceManager* service_manager = nullptr;
		SceneManager* scene_manager = nullptr;
		LayerStack* layer_stack = nullptr;

		bool minimized = false;
		bool running = true;



	}; // struct Application

	// to be defined in client
	Application* CreateApplication();

} // namespace





//namespace Magnefu
//{
//	struct Draw
//	{
//		
//	};
//
//	class  Application
//	{
//	public:
//		Application();
//		Application(const Application&) = delete;
//		Application& operator=(const Application&) = delete;
//
//		virtual ~Application();
//
//		virtual void Run();
//
//		void OnEvent(Event& event);
//		void OnUpdate(float deltaTime);
//
//		void PushLayer(Layer* layer);
//		void PushOverlay(Layer* overlay);
//
//		inline Window& GetWindow() { return *m_Window; }
//
//		// -- Managers -- //
//
//
//		inline SceneManager& GetSceneManager() { return *m_SceneManager; }
//
//		// -- Services -- //
//		inline MemoryService& GetMemoryService() { return *MemoryService::Instance(); }
//		inline GraphicsContext& GetGraphicsContext() { return *GraphicsContext::Instance(); }
//
//
//		// -- SCENES -- //
//		
//
//
//		inline static Application& Get() { return *s_Instance; }
//
//		
//
//	private:
//		bool OnWindowClose(WindowCloseEvent& e);
//		bool OnWindowResize(WindowResizeEvent& e);
//	
//
//	private:
//
//		// Try not to have any Array<T> members in this class as
//		// the memory service is initialized in its constructor
//		// but any arrays' default constructor will need its memory
//		// allocator. class member constructors are called
//		// BEFORE their owning object's constructor is called
//
//		static Application* s_Instance;
//
//		LayerStack m_LayerStack;
//		Scope<Window> m_Window;
//
//		Scope<SceneManager> m_SceneManager;
//
//		bool m_Running;
//		bool m_Minimized;
//
//
//		
//	};
//
//	// to be defined in client
//	Application* CreateApplication();
//}

