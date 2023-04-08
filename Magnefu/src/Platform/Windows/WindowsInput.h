#pragma once

#include "Magnefu/Core/Events/Input.h"


namespace Magnefu
{
	class WindowsInput : public Input
	{
	protected:
		bool IsKeyPressedImpl(int keycode) override;
		bool IsKeyReleasedImpl(int keycode) override;

		bool IsMouseButtonPressedImpl(int buttoncode);
		bool IsMouseButtonReleasedImpl(int buttoncode);

		std::pair<float, float> GetMousePositionImpl() override;
		float GetMouseXImpl() override;
		float GetMouseYImpl() override;
	};
}