
// -- PCH -- //
#include "mfpch.h"

// -- .h -- //
#include "Application.h"

// -- Application Includes ---------//


// -- Graphics Includes --------------------- //
#include "Magnefu/Graphics/GraphicsContext.h"

// -- Core Includes ---------------------------------- //
#include "Magnefu/Core/Log.h"
#include "Magnefu/Core/Time.hpp"



//TEMP
#include "imgui/imgui.h"


namespace Magnefu
{
	Application* Application::s_Instance = nullptr;


	void Application::Run(const ApplicationConfiguration& configuration) 
	{
		MF_CORE_ASSERT(!s_Instance, "Application instance already exists.");
		s_Instance = this;

		create(configuration);
		main_loop();
		destroy();
	}

	void Application::PushLayer(Layer* layer)
	{
		layer_stack->PushLayer(layer);
		layer->OnAttach();
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch <WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));
		dispatcher.Dispatch<WindowMovedEvent>(BIND_EVENT_FN(Application::OnWindowMoved));
		dispatcher.Dispatch<WindowFocusEvent>(BIND_EVENT_FN(Application::OnWindowFocus));
		dispatcher.Dispatch<WindowLostFocusEvent>(BIND_EVENT_FN(Application::OnWindowLostFocus));

		for (auto it = layer_stack->end(); it != layer_stack->begin(); )
		{
			(*--it)->OnEvent(event);
			if (event.IsHandled())
				break;
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		is_running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
		    is_minimized = true;
		    return false;
		}

		is_minimized = false;
		return true;
	}

	bool Application::OnWindowMoved(WindowMovedEvent& e)
	{

		return true;
	}

	bool Application::OnWindowFocus(WindowFocusEvent& e)
	{
		has_focus = true;
		return true;
	}

	bool Application::OnWindowLostFocus(WindowLostFocusEvent& e)
	{
		has_focus = false;
		return true;
	}

	

 

      
}