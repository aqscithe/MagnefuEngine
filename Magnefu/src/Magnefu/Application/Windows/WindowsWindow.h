#pragma once

#include "Window.h"


namespace Magnefu
{
	class WindowsWindow : public Window
	{
	public:
		~WindowsWindow() override {};

		void Init(void* config) override;
		void Shutdown() override;

		void        SetFullscreen(bool value) override;
		void        CenterMouse(bool dragging) override;
		void		PollEvents() override;

	private:	


	private:

		/*struct MouseData
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

		MouseData m_Mouse;*/


		// ALL BELOW IS TEMPORARY

		

		/*void processInput();
		void MouseUpdates();*/
	};
}


