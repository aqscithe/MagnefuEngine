#pragma once


// -- Application ------------------- //
#include "Magnefu/Application/Events/Event.h"
#include "Magnefu/Application/Camera/Camera.h"


// -- Graphics ----------------------- //
#include "Magnefu/Graphics/GraphicsContext.h"


// -- Core ------------------------- //
#include "Magnefu/Core/Assertions.h"
#include "Magnefu/Core/SmartPointers.h"





namespace Magnefu
{

	struct WindowConfiguration 
	{

		u32             width;
		u32             height;

		cstring         name;

		Allocator* allocator;

	}; // struct WindowConfiguration

	
	// interface system representing a desktop sytem based window

	class  Window : public Service
	{
	public:
		using EventCallbackFn = std::function<void(Event& event)>;

		struct WindowUserPointer
		{
			EventCallbackFn EventCallback;
			u32 Width;
			u32 Height;
		};

		virtual ~Window() = default;

		
		uint16_t		GetWidth() const { return width; }
		uint16_t		GetHeight() const { return height; }

		void*			GetWindowHandle() const { return platform_handle; }

		void			SetEventCallback(const EventCallbackFn& callback) { window_data.EventCallback = callback; }
		

		virtual void    SetFullscreen(bool value)  = 0;
		virtual void    CenterMouse(bool dragging) = 0;


		
		
		// -- DONT KNOW IF I WILL KEEP THESES
		virtual void OnFinish() {};
		//virtual bool IsVSync() {};
		virtual void CloseWindow() {};
		virtual void OnUpdate() {};
		virtual void DrawFrame() {};
		virtual void OnImGuiRender() {};

	protected:

		void*			platform_handle = nullptr;
		bool            requested_exit = false;
		bool            resized = false;
		bool            minimized = false;

		u32             width = 0;
		u32             height = 0;
		f32             display_refresh = 1.0f / 60.0f;

		WindowUserPointer window_data;

		static constexpr cstring    k_name = "Magnefu_Window_Service";
	};
}