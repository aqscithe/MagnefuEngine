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
	struct WindowProps
	{
	public:
		std::string Title;
		uint16_t Width, Height;

		WindowProps(const std::string& title = "Magnefu Engine", uint16_t width = 1920, uint16_t height = 1080) :
			Title(title), Width(width), Height(height) {}
	};

	// interface system representing a desktop sytem based window

	class  Window
	{
	public:
		using EventCallbackFn = std::function<void(Event& event)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;
		virtual void DrawFrame() = 0;
		virtual void OnImGuiRender() = 0;

		// Main Loop Completed
		virtual void OnFinish() = 0;

		virtual uint16_t GetWidth() const = 0;
		virtual uint16_t GetHeight() const = 0;
		virtual void* GetNativeWindow() const = 0;


		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual bool IsVSync() const = 0;
		virtual void SetSceneCamera(const Ref<Camera>&) = 0;
		virtual Ref<Camera>& GetSceneCamera() = 0;
		virtual void SetFramebufferResized(bool framebufferResized) = 0;

		virtual void CloseWindow() = 0;

		static Window* Create(const WindowProps& props = WindowProps());
	};
}