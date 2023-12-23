#pragma once

#include "Window.h"


namespace Magnefu
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		~WindowsWindow();

		void OnUpdate() override;
		void DrawFrame() override;
		void OnImGuiRender() override;

		// Main Loop Completed
		void OnFinish() override; 

		inline uint16_t GetWidth() const override { return m_Data.Width; }
		inline uint16_t GetHeight() const override { return m_Data.Height; }
		inline void* GetNativeWindow() const override { return m_Window; }
		inline Ref<Camera>& GetSceneCamera() override { return m_SceneCamera; };

		// Window attributes
		void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetSceneCamera(const Ref<Camera>& cam) override;
		bool IsVSync() const override { return m_Data.VSync; }
		void SetFramebufferResized(bool framebufferResized) override;

		void CloseWindow();

	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();


	private:
		struct WindowData
		{
			std::string Title;
			WindowsWindow* WindowPtr;
			CameraData* CamData;
			EventCallbackFn EventCallback;
			uint16_t Width;
			uint16_t Height;
			bool VSync;
		};

		WindowData m_Data;

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

		void* m_Window;

		// Here we store all the data that a window api like glfw might be interested in.
		// this way we only have to pass the struct, not the entire class.
		

		// Need reference to scene camera here in window
		Ref<Camera> m_SceneCamera;


		// ALL BELOW IS TEMPORARY

		

		void processInput();
		void MouseUpdates();
	};
}

