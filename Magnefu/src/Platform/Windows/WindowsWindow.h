#pragma once

#include "Magnefu/Window.h"

#include "Globals.h"
#include <GLFW/glfw3.h>

namespace Magnefu
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		~WindowsWindow();

		void OnUpdate() override;

		inline uint16_t GetWidth() const override { return m_Data.Width; }
		inline uint16_t GetHeight() const override { return m_Data.Height; }
		inline void* GetNativeWindow() const override { return m_Window; }

		// Window attributes
		void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override { return m_Data.VSync; }

	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();


	private:
		GLFWwindow* m_Window;

		// Here we store all the data that a window api like glfw might be interested in.
		// this way we only have to pass the struct, not the entire class.
		struct WindowData
		{
			std::string Title;
			uint16_t Width;
			uint16_t Height;
			bool VSync;
			EventCallbackFn EventCallback;
		};

		WindowData m_Data;

		// ALL BELOW IS TEMPORARY

		struct MouseData
		{
			int displayWidth, displayHeight;
			float lastX;
			float lastY;
			float sensitivity;
			float DeltaX;
			float DeltaY;
			double	X;
			double	Y;
			bool flightMode;
		};

		MouseData m_Mouse;

		void processInput();
		void MouseUpdates();
	};
}


