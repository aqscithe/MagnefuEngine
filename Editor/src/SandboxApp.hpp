#pragma once

#include "Magnefu/Magnefu.h"

#include <filesystem>





class Sandbox : public Magnefu::Application
{
	public:
		Sandbox();
		~Sandbox();

		 void                create(const Magnefu::ApplicationConfiguration& configuration) override;
		 void                destroy() override;
		 bool                main_loop() override;

		// Fixed update. Can be called more than once compared to rendering.
		 void                fixed_update(f32 delta) override;
		// Variable time update. Called only once per frame.
		 void                variable_update(f32 delta) override;
		// Rendering with optional interpolation factor.
		 void                render(f32 interpolation) override;
		// Per frame begin/end.
		 void                frame_begin() override;
		 void                frame_end() override;

		 void				PushLayer(Magnefu::Layer* layer) override;
		 void				PushOverlay(Magnefu::Layer* overlay) override;

		 bool				OnWindowClose(Magnefu::WindowCloseEvent& e) override;
		 bool				OnWindowResize(Magnefu::WindowResizeEvent& e) override;

		 void                        on_resize(u32 new_width, u32 new_height);

		 void*						GetWindow() override { return (void*)window; }


		 f64                         accumulator = 0.0;
		 f64                         current_time = 0.0;
		 f32                         step = 1.0f / 60.0f;



#if defined(MF_PLATFORM_WINDOWS)

		 Magnefu::WindowsWindow* window = nullptr;

#elif defined(MF_PLATFORM_LINUX)
		 // TODO: Setup an application window implementation for OS other than Microsoft Windows
		 Magnefu::LinuxWindow* window = nullptr;

#elif defined(MF_PLATFORM_MAC)
		 Magnefu::MacWindow* window = nullptr;

#endif
		

		Magnefu::InputService* input = nullptr;
		Magnefu::Renderer* renderer = nullptr;
		Magnefu::ImGuiService* imgui = nullptr;

};