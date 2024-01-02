#pragma once

// -- Application Includes ---------//
#include "Events/ApplicationEvent.h"
#include "Events/MouseEvent.h"
#include "Events/KeyEvent.h"
#include "Windows/WindowsWindow.h"
#include "LayerStack.h"
#include "ImGui/ImGuiService.hpp"
#include "Input/Input.h"
#include "Scene/SceneManager.h"


// -- Graphics Includes --------------------- //
#include "Magnefu/Graphics/Renderer.hpp"
#include "Magnefu/Graphics/GPUProfiler.hpp"


// -- Core Includes ---------------------------------- //



namespace Magnefu
{
//#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)
#define BIND_EVENT_FN(instance, func) std::bind(&func, instance, std::placeholders::_1)

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
		virtual void                Create(const ApplicationConfiguration& configuration) {}
		virtual void                Destroy() {}
		virtual bool                MainLoop() { return false; }

		// Fixed update. Can be called more than once compared to rendering.
		virtual void                FixedUpdate(f32 delta) {}
		// Variable time update. Called only once per frame.
		virtual void                VariableUpdate(f32 delta) {}
		// Rendering with optional interpolation factor.
		virtual void                Render(f32 interpolation) {}
		// Per frame begin/end.
		virtual void                BeginFrame() {}
		virtual void                EndFrame() {}

		virtual void				DrawGUI() {}

		virtual void				PushLayer(Layer* layer);
		virtual void				PushOverlay(Layer* overlay) {}

		virtual void				OnEvent(Event& event) { };
		virtual bool				OnWindowClose(WindowCloseEvent& e) { return false; };
		virtual bool				OnWindowResize(WindowResizeEvent& e) { return false; };
		virtual bool				OnWindowMoved(WindowMovedEvent& e) {return false;};
		virtual bool				OnWindowFocus(WindowFocusEvent& e) {return false;};
		virtual bool				OnWindowLostFocus(WindowLostFocusEvent& e) { return false; };



		// -- Getter Methods -------------------------------- //
		static Application* Get() { return s_Instance; }

		virtual void* GetWindow() { return nullptr; }
		virtual ServiceManager* GetServiceManager() { return service_manager; }



		void                        Run(const ApplicationConfiguration& configuration);

	protected:

		ServiceManager* service_manager = nullptr;
		SceneManager* scene_manager = nullptr;
		LayerStack* layer_stack = nullptr;

		InputService* input = nullptr;
		Renderer* renderer = nullptr;
		ImGuiService* imgui = nullptr;

		ResourceManager* rm = nullptr;
		GPUProfiler* gpu_profiler = nullptr;


#if defined(MF_PLATFORM_WINDOWS)

		WindowsWindow* window = nullptr;

#elif defined(MF_PLATFORM_LINUX)
		// TODO: Setup an application window implementation for OS other than Microsoft Windows
		LinuxWindow* window = nullptr;

#elif defined(MF_PLATFORM_MAC)
		MacWindow* window = nullptr;

#endif

		static Application* s_Instance;


	}; // struct Application

	// to be defined in client
	Application* CreateApplication();

} // namespace

