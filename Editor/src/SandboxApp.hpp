#pragma once

#include "Magnefu/Magnefu.h"
#include <filesystem>





class Sandbox : public Magnefu::Application
{
	public:
		Sandbox();
		~Sandbox();

		 void               Create(const Magnefu::ApplicationConfiguration& configuration) override;
		 void               Destroy() override;
		 bool               MainLoop() override;

		 void				DrawGUI() override;


		 // Remember i need to call glfwpollevents at some point during run loop
		 // 
		 
		// Fixed update. Can be called more than once compared to rendering.
		 //void                FixedUpdate(f32 delta) override;
		// Variable time update. Called only once per frame.
		 //void                VariableUpdate(f32 delta) override;
		// Rendering with optional interpolation factor.
		// void                Render(f32 interpolation) override;
		// Per frame begin/end.
		 //void                BeginFrame() override;
		 //void                EndFrame() override;


		 void				OnEvent(Magnefu::Event& event) ;
		 bool				OnWindowClose(Magnefu::WindowCloseEvent& e);
		 bool				OnWindowResize(Magnefu::WindowResizeEvent& e);
		 bool				OnWindowMoved(Magnefu::WindowMovedEvent& e);
		 bool				OnWindowFocus(Magnefu::WindowFocusEvent& e);
		 bool				OnWindowLostFocus(Magnefu::WindowLostFocusEvent& e);


		 //void                        on_resize(u32 new_width, u32 new_height);

		 void*						GetWindow() override { return (void*)window; }


public:

		 f64                         accumulator = 0.0;
		 f64                         current_time = 0.0;
		 f32                         step = 1.0f / 60.0f; // Should be set to refresh rate of monitor by default? - Should have several options that fit neatly within monitor refresh container



		 Magnefu::InputService* input = nullptr;
		 Magnefu::Renderer* renderer = nullptr;
		 Magnefu::ImGuiService* imgui = nullptr;

		 Magnefu::ResourceManager rm;
		 Magnefu::GPUProfiler gpu_profiler;


#if defined(MF_PLATFORM_WINDOWS)

		 Magnefu::WindowsWindow* window = nullptr;

#elif defined(MF_PLATFORM_LINUX)
		 // TODO: Setup an application window implementation for OS other than Microsoft Windows
		 Magnefu::LinuxWindow* window = nullptr;

#elif defined(MF_PLATFORM_MAC)
		 Magnefu::MacWindow* window = nullptr;

#endif
		

		

};