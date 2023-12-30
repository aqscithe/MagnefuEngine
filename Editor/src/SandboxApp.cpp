#include "SandboxApp.hpp"
#include "SandboxLayer.hpp"


#include "imgui/imgui.h"

// -- Sandbox App -------------------------------------------------------------------------- //

#if defined(MF_PLATFORM_WINDOWS)

static Magnefu::WindowsWindow s_window;

#elif defined(MF_PLATFORM_LINUX)
// TODO: Setup an application window implementation for OS other than Microsoft Windows
static Magnefu::LinuxWindow s_window;

#elif defined(MF_PLATFORM_MAC)
static Magnefu::MacWindow s_window;

#endif


// The app constructor is where I push the layers I want to use in that app.
Sandbox::Sandbox()
{
	
}


Sandbox::~Sandbox()
{
	
}


void Sandbox::Create(const Magnefu::ApplicationConfiguration& configuration)
{
	using namespace Magnefu;

	time_service_init();

 	LogService::Instance()->Init(nullptr);
	MemoryService::Instance()->Init(nullptr);
	service_manager = Magnefu::ServiceManager::instance;

	service_manager->init(&Magnefu::MemoryService::Instance()->systemAllocator);

	layer_stack = new Magnefu::LayerStack();
	layer_stack->PushLayer(new SandboxLayer());

	// window
	WindowConfiguration wconf{ configuration.width, configuration.height, configuration.name, &MemoryService::Instance()->systemAllocator };
	window = &s_window;
	window->Init(&wconf);
	window->SetEventCallback(BIND_EVENT_FN(this, Sandbox::OnEvent));

	// input
	input = service_manager->get<InputService>();
	input->Init(&MemoryService::Instance()->systemAllocator);
	input->SetEventCallback(BIND_EVENT_FN(this, Sandbox::OnEvent));

	// graphics
	DeviceCreation dc;
	dc.set_window(window->GetWidth(), window->GetHeight(), window->GetWindowHandle()).
		set_allocator(&MemoryService::Instance()->systemAllocator).
		set_stack_allocator(&MemoryService::Instance()->tempStackAllocator);

	GraphicsContext* gpu = service_manager->get<GraphicsContext>();
	gpu->init(dc);


	// App specific create ////////////////////////////////////////////////
	renderer = service_manager->get<Renderer>();

	RendererCreation rc{ gpu, &MemoryService::Instance()->systemAllocator };
	renderer->init(rc);

	// imgui backend
	ImGuiServiceConfiguration config{ gpu, window->GetWindowHandle() };
	imgui = service_manager->get<ImGuiService>();
	layer_stack->PushOverlay(imgui);
	//imgui->Init(renderer);
	imgui->Init(&config);


	
	
	

	// will eventually have an EditorLayer
	// may even have more specific layers like Physics Collisions and AI...
	// this is how updates can be controlled separately

	// TODO: How should i give the app access to its layer and overlay?


	MF_CORE_INFO("Sandbox Application created successfully!");
}

void Sandbox::Destroy()
{
	using namespace Magnefu;

	MF_CORE_INFO("Begin Sandbox Shutdown...");

	// Need to handle deletion of layers

	// Shutdown services
	imgui->Shutdown();
	input->Shutdown();
	renderer->shutdown();
	window->Shutdown();

	delete layer_stack;

	time_service_shutdown();

	service_manager->shutdown();

	MemoryService::Instance()->Shutdown();
	LogService::Instance()->Shutdown();
}

bool Sandbox::MainLoop()
{
	using namespace Magnefu;

	accumulator = 0.0;
	auto start_time = time_now();

	while (!window->requested_exit)
	{
		if (!window->minimized)
		{
			renderer->begin_frame();
		}

		input->NewFrame();

		window->PollEvents();

		if (window->resized)
		{
			renderer->resize_swapchain(window->GetWidth(), window->GetHeight());

			window->resized = false;
		}

		imgui->BeginFrame();
		
		auto end_time = time_now();
		f32 delta_time = (f32)time_delta_seconds(start_time, end_time);
		start_time = end_time;

		accumulator += delta_time;

		input->Update(delta_time);

		while (accumulator >= step)
		{
			FixedUpdate(delta_time);

			accumulator -= step;
		}

		VariableUpdate(delta_time);

		if (!window->minimized)
		{
			MemoryService::Instance()->imguiDraw();
			auto* gpu_commands = renderer->get_command_buffer(QueueType::Graphics, true);
			
			gpu_commands->push_marker("Frame");

			
			const f32 interpolation_factor = Maths::clamp(0.0f, 1.0f, (f32)(accumulator / step));
			Render(interpolation_factor);

			//imgui->Render(renderer, *gpu_commands);
			imgui->Render(*gpu_commands);

			gpu_commands->pop_marker();

			// Send commands to GPU
			renderer->queue_command_buffer(gpu_commands);

			renderer->end_frame();
		}
		else
		{
			ImGui::Render();
		}

		// Prepare for next frame if anything must be done.
		EndFrame();
	}

	return false;
}


Magnefu::Application* Magnefu::CreateApplication()
{
	return new Sandbox();
}

void Sandbox::OnEvent(Magnefu::Event& event)
{
	using namespace Magnefu;

	EventDispatcher dispatcher(event);
	dispatcher.Dispatch <WindowCloseEvent>(BIND_EVENT_FN(this, Sandbox::OnWindowClose));
	dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(this, Sandbox::OnWindowResize));
	dispatcher.Dispatch<WindowMovedEvent>(BIND_EVENT_FN(this,Sandbox::OnWindowMoved));
	dispatcher.Dispatch<WindowFocusEvent>(BIND_EVENT_FN(this, Sandbox::OnWindowFocus));
	dispatcher.Dispatch<WindowLostFocusEvent>(BIND_EVENT_FN(this,Sandbox::OnWindowLostFocus));

	dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(input, InputService::OnEvent));
	dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN(input, InputService::OnEvent));
	dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(input, InputService::OnEvent));
	dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN(input, InputService::OnEvent));
	dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(input, InputService::OnEvent));
	dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(input, InputService::OnEvent));
	dispatcher.Dispatch<GamepadConnectedEvent>(BIND_EVENT_FN(input, InputService::OnEvent));
	dispatcher.Dispatch<GamepadDisconnectedEvent>(BIND_EVENT_FN(input, InputService::OnEvent));

	

	for (auto it = layer_stack->end(); it != layer_stack->begin(); )
	{
		(*--it)->OnEvent(event);
		if (event.IsHandled())
			break;
	}
}

bool Sandbox::OnWindowClose(Magnefu::WindowCloseEvent& e)
{
	window->requested_exit = true;

	return true;
} 

bool Sandbox::OnWindowResize(Magnefu::WindowResizeEvent& e)
{
	window->SetHeight(e.GetHeight());
	window->SetWidth(e.GetWidth());

	if (e.GetWidth() == 0 || e.GetHeight() == 0)
	{
		window->minimized = true;
		return true;
	}

	window->minimized = false;
	window->resized = true;

	return true;
}

bool Sandbox::OnWindowMoved(Magnefu::WindowMovedEvent& e)
{

	return true;
}

bool Sandbox::OnWindowFocus(Magnefu::WindowFocusEvent& e)
{
	input->SetHasFocus(true);
	return true;
}

bool Sandbox::OnWindowLostFocus(Magnefu::WindowLostFocusEvent& e)
{
	input->SetHasFocus(false);
	return true;
}