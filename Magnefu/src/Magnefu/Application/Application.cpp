
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

		Create(configuration);
		MainLoop();
		Destroy();
	}

	void Application::PushLayer(Layer* layer)
	{
		layer_stack->PushLayer(layer);
		layer->OnAttach();
	}

	

	

 

      
}