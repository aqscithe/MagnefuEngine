#include "mfpch.h"
#include "WindowsInput.h"
#include "WindowsWindow.h"
#include "Magnefu/Application/Application.h"

#include "GLFW/glfw3.h"


namespace Magnefu
{
	InputService* InputService::s_Instance = nullptr;

	void WindowsInput::Init(void* config)
	{
		MF_CORE_ASSERT(!s_Instance, "Input instance already exists.");
		s_Instance = this;
	}

	bool WindowsInput::IsKeyPressedImpl(int keycode)
	{
		auto window = (WindowsWindow*)Application::Get()->GetWindow();
		auto state = glfwGetKey((GLFWwindow*)window->GetWindowHandle(), keycode);

		return state == GLFW_PRESS || state == GLFW_REPEAT && has_focus;
	}

	bool WindowsInput::IsKeyReleasedImpl(int keycode)
	{
		auto window = (WindowsWindow*)Application::Get()->GetWindow();
		auto state = glfwGetKey((GLFWwindow*)window->GetWindowHandle(), keycode);

		return state == GLFW_RELEASE;
	}

	bool WindowsInput::IsMouseButtonPressedImpl(int buttoncode)
	{
		auto window = (WindowsWindow*)Application::Get()->GetWindow();
		auto state = glfwGetMouseButton((GLFWwindow*)window->GetWindowHandle(), buttoncode);

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WindowsInput::IsMouseButtonReleasedImpl(int buttoncode)
	{
		auto window = (WindowsWindow*)Application::Get()->GetWindow();
		auto state = glfwGetMouseButton((GLFWwindow*)window->GetWindowHandle(), buttoncode);

		return state == GLFW_RELEASE;
	}

	std::pair<float, float> WindowsInput::GetMousePositionImpl()
	{
		auto window = (WindowsWindow*)Application::Get()->GetWindow();
		double x, y;
		glfwGetCursorPos((GLFWwindow*)window->GetWindowHandle(), &x, &y);

		return { (float)x, (float)y };
	}

	float WindowsInput::GetMouseXImpl()
	{
		auto [x, y] = GetMousePositionImpl();
		return x;
	}

	float WindowsInput::GetMouseYImpl()
	{
		auto [x, y] = GetMousePositionImpl();
		return y;
	}
}