#include "SandboxApp.hpp"


class SandboxLayer;

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
static void input_os_messages_callback(void* os_event, void* user_data)
{
	Magnefu::InputService* input = (Magnefu::InputService*)user_data;
	input->on_event(os_event);
}


// The app constructor is where I push the layers I want to use in that app.
Sandbox::Sandbox()
{
	// initialize layerstack
	layer_stack = new Magnefu::LayerStack();


	// Push layers - 1st layer is on bottom
	PushLayer(new SandboxLayer());
	// will eventually have an EditorLayer
	// may even have more specific layers like Physics Collisions and AI...
	// this is how updates can be controlled separately
}


Sandbox::~Sandbox()
{
	delete layer_stack;
}


void Sandbox::create(const Magnefu::ApplicationConfiguration& configuration)
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
	window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

	// input
	input = service_manager->get<InputService>();
	input->init(&MemoryService::Instance()->systemAllocator);

	// graphics
	DeviceCreation dc;
	dc.set_window(window->width, window->height, window->platform_handle).set_allocator(&MemoryService::Instance()->systemAllocator);

	GraphicsContext* gpu = service_manager->get<GraphicsContext>();
	gpu->init(dc);


	// App specific create ////////////////////////////////////////////////
	renderer = service_manager->get<Renderer>();

	RendererCreation rc{ gpu, &MemoryService::Instance()->systemAllocator };
	renderer->init(rc);

	// imgui backend
	imgui = service_manager->get<ImGuiService>();
	imgui->Init(renderer);

	MF_CORE_INFO("Sandbox Application created successfully!");
}

void Sandbox::destroy()
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

bool Sandbox::main_loop()
{
	return false;
}


Magnefu::Application* Magnefu::CreateApplication()
{
	return new Sandbox();
}