#include "SandboxApp.hpp"

// -- App ---------------- //
#include "AppLayers/SandboxLayer.hpp"
#include "AppLayers/Overlay.hpp"
#include "GameCamera.hpp"
#include "Magnefu/Application/Windows/Window.h"
#include "Magnefu/Application/Input/Input.h"
#include "Magnefu/Application/Input/KeyCodes.h"

#include "Magnefu/Application/ImGui/ImGuiService.hpp"

// -- Graphics --------------------------- //
#include "Magnefu/Graphics/GraphicsContext.h"
#include "Magnefu/Graphics/CommandBuffer.hpp" 
#include "Magnefu/Graphics/spirv_parser.hpp"
#include "Magnefu/Graphics/GPUProfiler.hpp"
#include "Magnefu/Graphics/Renderer.hpp"
#include "Magnefu/Graphics/RenderScene.hpp"
#include "Magnefu/Graphics/glTFScene.hpp"
#include "Magnefu/Graphics/ObjScene.hpp"
#include "Magnefu/Graphics/FrameGraph.hpp"
#include "Magnefu/Graphics/AsynchronousLoader.hpp"
#include "Magnefu/Graphics/SceneGraph.hpp"
#include "Magnefu/Graphics/RenderResourcesLoader.hpp"


// -- Core -------------------------- //
#include "Magnefu/Core/File.hpp"
#include "Magnefu/Core/glTF.hpp"


// -- Vendor ----------------------- //
#include "imgui/imgui.h"
#include "enkiTS/TaskScheduler.h"

#include "stb_image/stb_image.h"


#include "cglm/struct/mat3.h"
#include "cglm/struct/mat4.h"
#include "cglm/struct/quat.h"
#include "cglm/struct/cam.h"
#include "cglm/struct/affine.h"




// Static variables
#if defined(MF_PLATFORM_WINDOWS)

static Magnefu::WindowsWindow s_window;

#elif defined(MF_PLATFORM_LINUX)
// TODO: Setup an application window implementation for OS other than Microsoft Windows
static Magnefu::LinuxWindow s_window;

#elif defined(MF_PLATFORM_MAC)
static Magnefu::MacWindow s_window;

#endif



static Magnefu::ResourceManager     s_rm;
static Magnefu::GPUProfiler         s_gpu_profiler;

static Magnefu::BufferHandle        scene_cb;

static enki::TaskScheduler	        s_task_scheduler;




static const u16 INVALID_TEXTURE_INDEX = ~0u;
static Magnefu::RenderScene* scene = nullptr;
static GameCamera s_game_camera;


static Magnefu::AsynchronousLoader           s_async_loader;


static Magnefu::FrameGraphBuilder	s_frame_graph_builder;
static Magnefu::FrameGraph			s_frame_graph;

static Magnefu::RenderResourcesLoader s_render_resources_loader;

static Magnefu::SceneGraph			s_scene_graph;


// -- IO Tasks ------------------------------------------------------ //

struct RunPinnedTaskLoopTask : enki::IPinnedTask
{
	void Execute() override
	{
		while (task_scheduler->GetIsRunning() && execute)
		{
			// 'sleeps' until there are pinned tasks to run
			task_scheduler->WaitForNewPinnedTasks();

			task_scheduler->RunPinnedTasks();
		}
	}

	enki::TaskScheduler* task_scheduler;
	bool execute = true;

}; // RunPinnedTaskLoopTask


static RunPinnedTaskLoopTask        s_run_pinned_task;

struct AsynchronousLoadTask : enki::IPinnedTask
{
	void Execute() override
	{
		while (execute)
		{
			async_loader->update(nullptr);
		}
	}

	Magnefu::AsynchronousLoader*		async_loader;
	enki::TaskScheduler*	task_scheduler;
	bool					execute = true;

}; // AsynchronousLoadTask

static AsynchronousLoadTask         s_async_load_task;


// -- Render Data ----------------------------------- //

struct RenderData
{
	vec3s eye = vec3s{ 0.0f, 2.5f, 2.0f };
	vec3s look = vec3s{ 0.0f, 0.0, -1.0f };
	vec3s right = vec3s{ 1.0f, 0.0, 0.0f };

	f32 yaw = 0.0f;
	f32 pitch = 0.0f;

	vec3s light_position{ 0.f, 4.0f, 0.f };

	float model_scale = 1.0f;
	float light_range = 20.0f;
	float light_intensity = 80.0f;
};

// ----------------------------------------------------------------------------------------------------------- //



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


	// -- Loading Services ---------------------------- //

	time_service_init();

 	LogService::Instance()->Init(nullptr);

    MemoryServiceConfiguration memory_configuration;
    memory_configuration.maxDynamicSize = mfgiga(2ull);

    MemoryService::Instance()->Init(&memory_configuration);
    Allocator* allocator = &MemoryService::Instance()->systemAllocator;

    StackAllocator* scratch_allocator = &MemoryService::Instance()->tempStackAllocator;
    scratch_allocator->init(mfmega(8));


	service_manager = Magnefu::ServiceManager::instance;
	service_manager->init(allocator);


	enki::TaskSchedulerConfig scheduler_config;
	// In this example we create more threads than the hardware can run,
	// because the IO thread will spend most of it's time idle or blocked
	// and therefore not scheduled for CPU time by the OS
	scheduler_config.numTaskThreadsToCreate += 1;
	s_task_scheduler.Initialize(scheduler_config);
	
	// window
	WindowConfiguration wconf{ configuration.width, configuration.height, configuration.name, allocator };
	window = &s_window;
	window->Init(&wconf);
	window->SetEventCallback(BIND_EVENT_FN(this, Sandbox::OnEvent));

	// input
	input = service_manager->get<InputService>();
	input->Init(allocator);
	input->SetEventCallback(BIND_EVENT_FN(this, Sandbox::OnEvent));

	// graphics
	DeviceCreation dc;
    dc.set_window(window->GetWidth(), window->GetHeight(), window->GetWindowHandle()).
        set_allocator(allocator).
        set_stack_allocator(scratch_allocator).
        set_num_threads(s_task_scheduler.GetNumTaskThreads());

	GraphicsContext* gpu = service_manager->get<GraphicsContext>();
	gpu->init(dc);

	//ResourceManager
	rm = &s_rm;
	rm->init(&MemoryService::Instance()->systemAllocator, nullptr);

	//GPUProfiler
	gpu_profiler = &s_gpu_profiler;
	gpu_profiler->init(&MemoryService::Instance()->systemAllocator, 100);


	
	renderer = service_manager->get<Renderer>();

	RendererCreation rc{ gpu, &MemoryService::Instance()->systemAllocator };
	renderer->init(rc);
	renderer->set_loaders(rm);

	// imgui backend
	ImGuiServiceConfiguration config{ gpu, window->GetWindowHandle() };
	imgui = service_manager->get<ImGuiService>();
	//imgui->Init(renderer);
	imgui->Init(&config);


	// Game Camera:
	s_game_camera.camera.init_perpective(0.1f, 4000.f, 60.f, wconf.width * 1.f / wconf.height);
	s_game_camera.init(true, 20.f, 6.f, 0.1f);


	s_frame_graph_builder.init(gpu);
	s_frame_graph.init(&s_frame_graph_builder);



	// will eventually have an EditorLayer
	// may even have more specific layers like Physics Collisions and AI...
	// this is how updates can be controlled separately

	// TODO: How should i give the app access to its layer and overlay?

	layer_stack = new Magnefu::LayerStack();
	layer_stack->PushLayer(new SandboxLayer());
	layer_stack->PushOverlay(new Overlay());


	// Load frame graph and parse gpu techniques

	{
		sizet scratch_marker = scratch_allocator->getMarker();

		StringBuffer temp_name_buffer;
		temp_name_buffer.init(1024, scratch_allocator);
		cstring frame_graph_path = temp_name_buffer.append_use_f("%s%s", MAGNEFU_FRAME_GRAPH_FOLDER, "graph.json");

		s_frame_graph.parse(frame_graph_path, scratch_allocator);
		s_frame_graph.compile();

		s_render_resources_loader.init(renderer, scratch_allocator, &s_frame_graph);

		// Parse techniques
		GpuTechniqueCreation gtc;
		temp_name_buffer.clear();
		cstring full_screen_pipeline_path = temp_name_buffer.append_use_f("%s/%s", MAGNEFU_SHADER_FOLDER, "fullscreen.json");
		s_render_resources_loader.load_gpu_technique(full_screen_pipeline_path);

		temp_name_buffer.clear();
		cstring main_pipeline_path = temp_name_buffer.append_use_f("%s/%s", MAGNEFU_SHADER_FOLDER, "main.json");
		s_render_resources_loader.load_gpu_technique(main_pipeline_path);

		temp_name_buffer.clear();
		cstring pbr_pipeline_path = temp_name_buffer.append_use_f("%s/%s", MAGNEFU_SHADER_FOLDER, "pbr_lighting.json");
		s_render_resources_loader.load_gpu_technique(pbr_pipeline_path);

		temp_name_buffer.clear();
		cstring dof_pipeline_path = temp_name_buffer.append_use_f("%s/%s", MAGNEFU_SHADER_FOLDER, "dof.json");
		s_render_resources_loader.load_gpu_technique(dof_pipeline_path);

		scratch_allocator->freeToMarker(scratch_marker);
	}

	s_scene_graph.init(allocator, 4);

	// -- Grab Scene Data ------------------------------------------------ //
	cstring file_path = "";
	InjectDefault3DModel(file_path);


    // [TAG: Multithreading]
    s_async_loader.init(renderer, &s_task_scheduler, allocator);

    Directory cwd{ };
    directory_current(&cwd);

    char file_base_path[512]{ };

    memcpy(file_base_path, file_path, strlen(file_path));
    file_directory_from_path(file_base_path);

    directory_change(file_base_path);

    char file_name[512]{ };
    memcpy(file_name, file_path, strlen(file_path));
    file_name_from_path(file_name);


    char* file_extension = file_extension_from_path(file_name);

    if (strcmp(file_extension, "gltf") == 0) 
    {
        scene = new glTFScene;
    }
    else if (strcmp(file_extension, "obj") == 0)
    {
        scene = new ObjScene;
    }

    scene->init(file_name, file_base_path, allocator, scratch_allocator, &s_async_loader);

    // NOTE(marco): restore working directory
    directory_change(cwd.path);

	scene->register_render_passes(&s_frame_graph);
    scene->prepare_draws(renderer, scratch_allocator, &s_scene_graph);


    // Start multithreading IO
    // Create IO threads at the end
    s_run_pinned_task.threadNum = s_task_scheduler.GetNumTaskThreads() - 1;
    s_run_pinned_task.task_scheduler = &s_task_scheduler;
    s_task_scheduler.AddPinnedTask(&s_run_pinned_task);

    // Send async load task to external thread FILE_IO
    s_async_load_task.threadNum = s_run_pinned_task.threadNum;
    s_async_load_task.task_scheduler = &s_task_scheduler;
    s_async_load_task.async_loader = &s_async_loader;
    s_task_scheduler.AddPinnedTask(&s_async_load_task);


	MF_CORE_INFO("Sandbox Application created successfully!");
}

void Sandbox::Destroy()
{
	using namespace Magnefu;

	MF_CORE_INFO("Begin Sandbox Shutdown...");

	GraphicsContext* gpu = service_manager->get<GraphicsContext>();

    s_run_pinned_task.execute = false;
    s_async_load_task.execute = false;
    s_task_scheduler.WaitforAllAndShutdown();

    vkDeviceWaitIdle(gpu->vulkan_device);

    s_async_loader.shutdown();

	gpu->destroy_buffer(scene_cb);

	imgui->Shutdown();
	
	gpu_profiler->shutdown();

	s_scene_graph.shutdown();

	s_frame_graph.shutdown();
	s_frame_graph_builder.shutdown();

    scene->shutdown(renderer);


	rm->shutdown();
	renderer->shutdown();

    delete scene;

	input->Shutdown();
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

	Magnefu::GraphicsContext* gpu = service_manager->get<Magnefu::GraphicsContext>();


	RenderData render_data;


	accumulator = 0.0;
	i64 start_time = Magnefu::time_now();
    i64 absolute_time = start_time;

	while (!window->requested_exit)
	{
		MF_PROFILE_SCOPE("Frame");


		// -- New Frame ----------- //
		if (!window->minimized)
		{
			gpu->new_frame();

            static bool checksz = true;
            if (s_async_loader.file_load_requests.size == 0 && checksz)
            {
                checksz = false;
                MF_APP_DEBUG("Uploaded textures in {} seconds", time_from_seconds(absolute_time));
            }
		}


		// -- Process Input ----------- //
		window->PollEvents();
		input->NewFrame();



		if (window->resized)
		{
            // Resize Framebuffer
			gpu->resize(window->GetWidth(), window->GetHeight());

			window->resized = false;
			s_frame_graph.on_resize(*gpu, window->GetWidth(), window->GetHeight());

            // Change aspect ratio
			s_game_camera.camera.set_aspect_ratio(window->GetWidth() * 1.f / window->GetHeight());
		}


		// -- Game State Updates ---------------------------- //
		i64 end_time = Magnefu::time_now();
		f32 delta_time = (f32)Magnefu::time_delta_seconds(start_time, end_time);
		start_time = end_time;

		accumulator += delta_time;

		while (accumulator >= step)
		{
			FixedUpdate(delta_time);

			accumulator -= step;
		}

		VariableUpdate(delta_time);


		// -- ImGui New Frame -------------- //
		imgui->BeginFrame();
		

		// Draw GUI
		{
            MF_PROFILE_SCOPE("Draw GUI");

			if (ImGui::Begin("Magnefu ImGui"))
			{
                ImGui::SeparatorText("MODELS");
				ImGui::InputFloat("Scene Global Scale scale", &scene->global_scale, 0.001f);

                ImGui::SeparatorText("LIGHTS");
				ImGui::SliderFloat3("Light position", render_data.light_position.raw, -30.f, 30.f);
				ImGui::InputFloat("Light range", &render_data.light_range);
				ImGui::InputFloat("Light intensity", &render_data.light_intensity);
                
                ImGui::SeparatorText("CAMERA");
				ImGui::InputFloat3("Camera position", s_game_camera.camera.position.raw);
				ImGui::InputFloat3("Camera target movement", s_game_camera.target_movement.raw);

                ImGui::SeparatorText("OPTIONS");
                ImGui::Checkbox("Dynamically recreate descriptor sets", &recreate_per_thread_descriptors);
                ImGui::Checkbox("Use secondary command buffers", &use_secondary_command_buffers);

				static bool fullscreen = false;
				if (ImGui::Checkbox("Fullscreen", &fullscreen))
				{
					window->SetFullscreen(fullscreen);
				}

				static i32 present_mode = renderer->gpu->present_mode;
				if (ImGui::Combo("Present Mode", &present_mode, Magnefu::PresentMode::s_value_names, Magnefu::PresentMode::Count))
				{
					renderer->set_presentation_mode((Magnefu::PresentMode::Enum)present_mode);
				}

				s_frame_graph.add_ui();

			}
			ImGui::End();

			if (ImGui::Begin("GPU"))
			{
                renderer->imgui_draw();
                ImGui::Separator();
				gpu_profiler->imgui_draw();
			}
			ImGui::End();

			DrawGUI();
		}

		s_scene_graph.update_matrices();

		const f32 interpolation_factor = Maths::clamp(0.0f, 1.0f, (f32)(accumulator / step));
		Render(interpolation_factor, &render_data);

		

		// Prepare for next frame if anything must be done.
		EndFrame();
	}

	return false;
}

void Sandbox::Render(f32 interpolation_factor, void* data)
{
    using namespace Magnefu;

    // //

    if (!window->minimized)
    {

        GraphicsContext* gpu = service_manager->get<GraphicsContext>();

        RenderData* render_data = (RenderData*)data;

        {
            // Update common constant buffer
            MF_PROFILE_SCOPE("Uniform Buffer Update");

            MapBufferParameters cb_map = { scene->scene_cb, 0, 0 };
			GpuSceneData* uniform_data = (GpuSceneData*)gpu->map_buffer(cb_map);
            if (uniform_data)
            {
                uniform_data->view_projection = s_game_camera.camera.view_projection;
                uniform_data->eye = vec4s{ s_game_camera.camera.position.x, s_game_camera.camera.position.y, s_game_camera.camera.position.z, 1.0f };
                uniform_data->light_position = vec4s{ render_data->light_position.x, render_data->light_position.y, render_data->light_position.z, 1.0f };
                uniform_data->light_range = render_data->light_range;
                uniform_data->light_intensity = render_data->light_intensity;

                gpu->unmap_buffer(cb_map);
            }

            scene->upload_materials(/* model scale */);
        }

        scene->submit_draw_task(imgui, gpu_profiler, &s_task_scheduler);

        gpu->present();

    }
    else
    {
        ImGui::Render();
    }
}

void Sandbox::DrawGUI()
{
	using namespace Magnefu;

	for (auto it = layer_stack->end(); it != layer_stack->begin(); )
	{
		(*--it)->DrawGUI();
	}

}

void Sandbox::FixedUpdate(f32 delta)
{
}

void Sandbox::VariableUpdate(f32 delta_time)
{
	input->Update(delta_time);
	s_game_camera.update(input, window->GetWidth(), window->GetHeight(), delta_time);
	window->CenterMouse(s_game_camera.mouse_dragging);
}



void Sandbox::BeginFrame()
{
}

void Sandbox::EndFrame()
{
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