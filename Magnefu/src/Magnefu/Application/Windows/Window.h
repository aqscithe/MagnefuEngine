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

	typedef void        (*OsMessagesCallback)(void* os_event, void* user_data);

	// interface system representing a desktop sytem based window

	class  Window : public Service
	{
	public:
		using EventCallbackFn = std::function<void(Event& event)>;

		virtual ~Window() {}

		void            Init(void* configuration) override;
		void            Shutdown() override;

		virtual void OnUpdate() = 0;
		virtual void DrawFrame() = 0;
		virtual void OnImGuiRender() = 0;

		// Main Loop Completed
		virtual void OnFinish() = 0;

		virtual uint16_t GetWidth() const = 0;
		virtual uint16_t GetHeight() const = 0;
		virtual void* GetNativeWindow() const = 0;

		virtual void            set_fullscreen(bool value) = 0;

		void            handle_os_messages();
		void            register_os_messages_callback(OsMessagesCallback callback, void* user_data);
		void            unregister_os_messages_callback(OsMessagesCallback callback);
		virtual void            center_mouse(bool dragging) = 0;


		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual bool IsVSync() const = 0;
		virtual void SetSceneCamera(const Ref<Camera>&) = 0;
		virtual Ref<Camera>& GetSceneCamera() = 0;
		virtual void SetFramebufferResized(bool framebufferResized) = 0;

		virtual void CloseWindow() = 0;

		static Window* Create(const WindowProps& props = WindowProps());


	public:

		Array<OsMessagesCallback> os_messages_callbacks;
		Array<void*>    os_messages_callbacks_data;

		void*			platform_handle = nullptr;
		bool            requested_exit = false;
		bool            resized = false;
		bool            minimized = false;

		u32             width = 0;
		u32             height = 0;
		f32             display_refresh = 1.0f / 60.0f;

		static constexpr cstring    k_name = "Magnefu_Window_Service";
	};
}