#pragma once

#include "mfpch.h"

#include "Core.h"
#include "Events/Event.h"


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

	class MAGNEFU_API Window
	{
	public:
		using EventCallbackFn = std::function<void(Event& event)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;

		virtual uint16_t GetWidth() const = 0;
		virtual uint16_t GetHeight() const = 0;


		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		static Window* Create(const WindowProps& props = WindowProps());
	};
}