#include "SandboxApp.hpp"
#include "SandboxLayer.hpp"


// -- Sandbox App -------------------------------------------------------------------------- //

#if defined(MF_PLATFORM_WINDOWS)

static Magnefu::WindowsWindow s_window;

#elif defined(MF_PLATFORM_LINUX)
// TODO: Setup an application window implementation for OS other than Microsoft Windows
static Magnefu::LinuxWindow s_window;

#elif defined(MF_PLATFORM_MAC)
static Magnefu::MacWindow s_window;

#endif

// Callbacks

// I should use my own event system...
//static void input_os_messages_callback(Magnefu::Event& e, void* user_data)
//{
//	Magnefu::InputService* input = (Magnefu::InputService*)user_data;
//	input->OnEvent(e);
//}


// The app constructor is where I push the layers I want to use in that app.
Sandbox::Sandbox()
{
	
}


Sandbox::~Sandbox()
{
	delete layer_stack;
}


void Sandbox::Create(const Magnefu::ApplicationConfiguration& configuration)
{
	using namespace Magnefu;

	time_service_init();

 	LogService::Instance()->Init(nullptr);
	MemoryService::Instance()->Init(nullptr);
	service_manager = Magnefu::ServiceManager::instance;

	service_manager->init(&Magnefu::MemoryService::Instance()->systemAllocator);
	//service_manager->get<LogService>();

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
	dc.set_window(window->GetWidth(), window->GetHeight(), window->GetWindowHandle()).set_allocator(&MemoryService::Instance()->systemAllocator);

	GraphicsContext* gpu = service_manager->get<GraphicsContext>();
	gpu->init(dc);


	// App specific create ////////////////////////////////////////////////
	renderer = service_manager->get<Renderer>();

	RendererCreation rc{ gpu, &MemoryService::Instance()->systemAllocator };
	renderer->init(rc);

	// imgui backend
	imgui = service_manager->get<ImGuiService>();
	imgui->Init(renderer);


	// -- Initialize layer stack and push layers -------------- //
	layer_stack = new Magnefu::LayerStack();
	layer_stack->PushLayer(new SandboxLayer());
	layer_stack->PushOverlay(imgui);

	// will eventually have an EditorLayer
	// may even have more specific layers like Physics Collisions and AI...
	// this is how updates can be controlled separately

	// TODO: How should i give the app access to its layer and overlay?


	MF_CORE_INFO("Sandbox Application created successfully!");
}

void Sandbox::Destroy()
{
	using namespace Magnefu;

	MF_CORE_INFO("Sandbox Shutdown");

	// Shutdown services
	imgui->Shutdown();
	input->Shutdown();
	renderer->shutdown();
	window->Shutdown();

	time_service_shutdown();

	service_manager->shutdown();

	MemoryService::Instance()->Shutdown();
	LogService::Instance()->Shutdown();
}

bool Sandbox::MainLoop()
{
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
	is_running = false;

	return true;
}

bool Sandbox::OnWindowResize(Magnefu::WindowResizeEvent& e)
{
	if (e.GetWidth() == 0 || e.GetHeight() == 0)
	{
		is_minimized = true;
		return true;
	}

	is_minimized = false;
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