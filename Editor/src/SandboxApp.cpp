#include "SandboxApp.hpp"

// -- App ---------------- //
#include "AppLayers/SandboxLayer.hpp"
#include "AppLayers/Overlay.hpp"
#include "Magnefu/Application/GameCamera.hpp"
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

#include "cglm/struct/vec2.h"
#include "cglm/struct/mat2.h"
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
static Magnefu::GpuVisualProfiler         s_gpu_profiler;

static Magnefu::BufferHandle        scene_cb;

static enki::TaskScheduler	        s_task_scheduler;




static const u16 INVALID_TEXTURE_INDEX = ~0u;
static Magnefu::RenderScene* scene = nullptr;
static Magnefu::GameCamera s_game_camera;


static Magnefu::AsynchronousLoader           s_async_loader;


static Magnefu::FrameGraphBuilder	s_frame_graph_builder;
static Magnefu::FrameGraph			s_frame_graph;


static Magnefu::RenderResourcesLoader s_render_resources_loader;

static Magnefu::SceneGraph			s_scene_graph;

static Magnefu::FrameRenderer			s_frame_renderer;

static Magnefu::TextureResource* dither_texture = nullptr;


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


vec4s normalize_plane(vec4s plane) {
	f32 len = glms_vec3_norm({ plane.x, plane.y, plane.z });
	return glms_vec4_scale(plane, 1.0f / len);
}

f32 linearize_depth(f32 depth, f32 z_far, f32 z_near) {
	return z_near * z_far / (z_far + depth * (z_near - z_far));
}


static void test_sphere_aabb(Magnefu::GameCamera& game_camera) {
	vec4s pos{ -14.5f, 1.28f, 0.f, 1.f };
	f32 radius = 0.5f;
	vec4s view_space_pos = glms_mat4_mulv(game_camera.camera.view, pos);
	bool camera_visible = view_space_pos.z < radius + game_camera.camera.near_plane;

	// X is positive, then it returns the same values as the longer method.
	vec2s cx{ view_space_pos.x, -view_space_pos.z };
	vec2s vx{ sqrtf(glms_vec2_dot(cx, cx) - (radius * radius)), radius };
	mat2s xtransf_min{ vx.x, vx.y, -vx.y, vx.x };
	vec2s minx = glms_mat2_mulv(xtransf_min, cx);
	mat2s xtransf_max{ vx.x, -vx.y, vx.y, vx.x };
	vec2s maxx = glms_mat2_mulv(xtransf_max, cx);

	vec2s cy{ -view_space_pos.y, -view_space_pos.z };
	vec2s vy{ sqrtf(glms_vec2_dot(cy, cy) - (radius * radius)), radius };
	mat2s ytransf_min{ vy.x, vy.y, -vy.y, vy.x };
	vec2s miny = glms_mat2_mulv(ytransf_min, cy);
	mat2s ytransf_max{ vy.x, -vy.y, vy.y, vy.x };
	vec2s maxy = glms_mat2_mulv(ytransf_max, cy);

	vec4s aabb{ minx.x / minx.y * game_camera.camera.projection.m00, miny.x / miny.y * game_camera.camera.projection.m11,
			   maxx.x / maxx.y * game_camera.camera.projection.m00, maxy.x / maxy.y * game_camera.camera.projection.m11 };
	vec4s aabb2{ aabb.x * 0.5f + 0.5f, aabb.w * -0.5f + 0.5f, aabb.z * 0.5f + 0.5f, aabb.y * -0.5f + 0.5f };

	vec3s left, right, top, bottom;
	Magnefu::get_bounds_for_axis(vec3s{ 1,0,0 }, { view_space_pos.x, view_space_pos.y, view_space_pos.z }, radius, game_camera.camera.near_plane, left, right);
	Magnefu::get_bounds_for_axis(vec3s{ 0,1,0 }, { view_space_pos.x, view_space_pos.y, view_space_pos.z }, radius, game_camera.camera.near_plane, top, bottom);

	left = Magnefu::project(game_camera.camera.projection, left);
	right = Magnefu::project(game_camera.camera.projection, right);
	top = Magnefu::project(game_camera.camera.projection, top);
	bottom = Magnefu::project(game_camera.camera.projection, bottom);

	vec4s clip_space_pos = glms_mat4_mulv(game_camera.camera.projection, view_space_pos);

	// left,right,bottom and top are in clip space (-1,1). Convert to 0..1 for UV, as used from the optimized version to read the depth pyramid.
	MF_APP_INFO("Camera visible {}, x {}, {}, widh {} --- {},{} width {}", camera_visible ? 1 : 0, aabb2.x, aabb2.z, aabb2.z - aabb2.x, left.x * 0.5 + 0.5, right.x * 0.5 + 0.5, (left.x - right.x) * 0.5);
	MF_APP_INFO("y {}, {}, height {} --- {},{} height {}", aabb2.y, aabb2.w, aabb2.w - aabb2.y, top.y * 0.5 + 0.5, bottom.y * 0.5 + 0.5, (top.y - bottom.y) * 0.5);
}

// Light placement function ///////////////////////////////////////////////
void place_lights(Magnefu::Array<Magnefu::Light>& lights, bool grid) {

	using namespace Magnefu;

	if (grid) {
		for (u32 i = 0; i < k_num_lights; ++i) {
			Light& light = lights[i];

			const f32 x = (i % 4);
			const f32 y = 10.f;
			const f32 z = (i / 4);

			light.world_position = { x, y, z };
			light.intensity = 10.f;
			light.radius = 0.25f;
			light.color = { 1, 1, 1 };
		}
	}

	//// TODO(marco): we should take this into account when generating the lights positions
	//const float scale = 0.008f;

	//for ( u32 i = 0; i < k_num_lights; ++i ) {
	//    float x = get_random_value( mesh_aabb[ 0 ].x * scale, mesh_aabb[ 1 ].x * scale );
	//    float y = get_random_value( mesh_aabb[ 0 ].y * scale, mesh_aabb[ 1 ].y * scale );
	//    float z = get_random_value( mesh_aabb[ 0 ].z * scale, mesh_aabb[ 1 ].z * scale );

	//    float r = get_random_value( 0.0f, 1.0f );
	//    float g = get_random_value( 0.0f, 1.0f );
	//    float b = get_random_value( 0.0f, 1.0f );

	//    Light new_light{ };
	//    new_light.world_position = vec3s{ x, y, z };
	//    new_light.radius = 1.2f; // TODO(marco): random as well?

	//    new_light.color = vec3s{ r, g, b };
	//    new_light.intensity = 30.0f;

	//    lights.push( new_light );
	//}
}



// -- Render Data ----------------------------------- //

struct RenderData
{
	vec3s light_position{ 0.f, 10.0f, 0.f };

	float model_scale = 1.0f;
	float light_radius = 20.0f;
	float light_intensity = 80.0f;

	// Cloth physics
	f32 spring_stiffness = 10000.0f;
	f32 spring_damping = 5000.0f;
	f32 air_density = 2.0f;
	bool reset_simulation = false;
	vec3s wind_direction{ -2.0f, 0.0f, 0.0f };

	f32 animation_speed_multiplier = 0.05f;

	bool light_placement = true;

	bool enable_frustum_cull_meshes = true;
	bool enable_frustum_cull_meshlets = true;
	bool enable_occlusion_cull_meshes = true;
	bool enable_occlusion_cull_meshlets = true;
	bool freeze_occlusion_camera = false;

	bool enable_camera_inside = false;
	bool use_mcguire_method = false;
	bool skip_invisible_lights = true;
	bool use_view_aabb = true;
	bool force_fullscreen_light_aabb = false;

	mat4s projection_transpose{ };
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
	GraphicsContextCreation dc;
    dc.set_window(window->GetWidth(), window->GetHeight(), window->GetWindowHandle()).
        set_allocator(allocator).
        set_linear_allocator(scratch_allocator).
        set_num_threads(s_task_scheduler.GetNumTaskThreads());

	GraphicsContext* gpu = service_manager->get<GraphicsContext>();
	gpu->init(dc);

	//ResourceManager
	rm = &s_rm;
	rm->init(&MemoryService::Instance()->systemAllocator, nullptr);

	//GPUProfiler
	gpu_profiler = &s_gpu_profiler;
	gpu_profiler->init(&MemoryService::Instance()->systemAllocator, 100, dc.gpu_time_queries_per_frame);


	
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
	s_game_camera.camera.init_perpective(0.1f, 1000.f, 60.f, wconf.width * 1.f / wconf.height);
	s_game_camera.init(true, 20.f, 6.f, 0.1f);


	s_frame_graph_builder.init(gpu);
	s_frame_graph.init(&s_frame_graph_builder);


	

	sizet scratch_marker = scratch_allocator->getMarker();

	StringBuffer temporary_name_buffer;
	temporary_name_buffer.init(1024, scratch_allocator);

	// Create binaries folders
	cstring shader_binaries_folder = temporary_name_buffer.append_use_f("%s", MAGNEFU_SHADER_FOLDER);
	if (!directory_exists(shader_binaries_folder)) {
		if (directory_create(shader_binaries_folder)) {
			MF_APP_INFO("Created folder {}", shader_binaries_folder);
		}
		else {
			MF_APP_ERROR("Cannot create folder {}");
		}
	}
	strcpy(renderer->resource_cache.binary_data_folder, shader_binaries_folder);
	temporary_name_buffer.clear();


	// will eventually have an EditorLayer
	// may even have more specific layers like Physics Collisions and AI...
	// this is how updates can be controlled separately

	// TODO: How should i give the app access to its layer and overlay?

	layer_stack = new Magnefu::LayerStack();
	layer_stack->PushLayer(new SandboxLayer());
	layer_stack->PushOverlay(new Overlay());


	// Load frame graph and parse gpu techniques

	{
		cstring frame_graph_path = temporary_name_buffer.append_use_f("%s%s", MAGNEFU_FRAME_GRAPH_FOLDER, "graph.json");

		s_frame_graph.parse(frame_graph_path, scratch_allocator);
		s_frame_graph.compile();

		s_render_resources_loader.init(renderer, scratch_allocator, &s_frame_graph);

		// TODO: add this to render graph itself.
		// Add utility textures (dithering, ...)
		temporary_name_buffer.clear();
		cstring dither_texture_path = temporary_name_buffer.append_use_f("%s/BayerDither4x4.png", MAGNEFU_TEXTURE_FOLDER);
		dither_texture = s_render_resources_loader.load_texture(dither_texture_path, false);

		

		// Parse techniques
		GpuTechniqueCreation gtc;
		const bool use_shader_cache = true;
		auto parse_technique = [&](cstring technique_name) {
			temporary_name_buffer.clear();
			cstring path = temporary_name_buffer.append_use_f("%s/%s", MAGNEFU_SHADER_FOLDER, technique_name);
			s_render_resources_loader.load_gpu_technique(path, use_shader_cache);
		};

		cstring techniques[] = { "meshlet.json", "fullscreen.json", "main.json",
								 "pbr_lighting.json", "dof.json", "cloth.json", "debug.json",
								 "culling.json" };

		const sizet num_techniques = ArraySize(techniques);
		for (sizet t = 0; t < num_techniques; ++t) {
			parse_technique(techniques[t]);
		}

	}
	
	s_scene_graph.init(allocator, 4);

	// -- Grab Scene Data ------------------------------------------------ //

	


    // [TAG: Multithreading]
    s_async_loader.init(renderer, &s_task_scheduler, allocator);

    Directory cwd{ };
    directory_current(&cwd);

	temporary_name_buffer.clear();
	cstring scene_path = nullptr;

	InjectDefault3DModel(scene_path);


    char file_base_path[512]{ };

    memcpy(file_base_path, scene_path, strlen(scene_path));
    file_directory_from_path(file_base_path);

    directory_change(file_base_path);

    char file_name[512]{ };
    memcpy(file_name, scene_path, strlen(scene_path));
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

	scene->use_meshlets = gpu->mesh_shaders_extension_present;
	scene->use_meshlets_emulation = !scene->use_meshlets;
    scene->init(file_name, file_base_path, allocator, scratch_allocator, &s_async_loader);

    // NOTE(marco): restore working directory
    directory_change(cwd.path);

	s_frame_renderer.init(allocator, renderer, &s_frame_graph, &s_scene_graph, scene);
	s_frame_renderer.prepare_draws(scratch_allocator);


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
	s_frame_renderer.shutdown();

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
			renderer->resize_swapchain(window->GetWidth(), window->GetHeight());

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
			// TODO: Move physics and physics-driven animations updates to FixedUpdate
			// Physics eventually to be done on gpu with physx
			FixedUpdate(delta_time);

			accumulator -= step;
		}

		VariableUpdate(delta_time);


		static f32 animation_speed_multiplier = 0.05f;

		static bool enable_frustum_cull_meshes = true;
		static bool enable_frustum_cull_meshlets = true;
		static bool enable_occlusion_cull_meshes = true;
		static bool enable_occlusion_cull_meshlets = true;
		static bool freeze_occlusion_camera = false;

		static bool enable_camera_inside = false;
		static bool use_mcguire_method = false;
		static bool skip_invisible_lights = true;
		static bool use_view_aabb = true;
		static bool force_fullscreen_light_aabb = false;

		static mat4s projection_transpose{ };


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
				ImGui::InputFloat("Light radius", &render_data.light_radius);
				ImGui::InputFloat("Light intensity", &render_data.light_intensity);
                
                ImGui::SeparatorText("CAMERA");
				ImGui::InputFloat3("Camera position", s_game_camera.camera.position.raw);
				ImGui::InputFloat3("Camera target movement", s_game_camera.target_movement.raw);

				ImGui::SeparatorText("PHYSICS");
				ImGui::InputFloat3("Wind direction", render_data.wind_direction.raw);
				ImGui::InputFloat("Air density", &render_data.air_density);
				ImGui::InputFloat("Spring stiffness", &render_data.spring_stiffness);
				ImGui::InputFloat("Spring damping", &render_data.spring_damping);
				ImGui::Checkbox("Reset simulation", &render_data.reset_simulation);

				ImGui::SeparatorText("OPTIONS");

				static bool enable_meshlets = false;
				enable_meshlets = scene->use_meshlets && gpu->mesh_shaders_extension_present;
				ImGui::Checkbox("Use meshlets", &enable_meshlets);
				scene->use_meshlets = enable_meshlets;
				ImGui::Checkbox("Use meshlets emulation", &scene->use_meshlets_emulation);
				ImGui::Checkbox("Use frustum cull for meshes", &enable_frustum_cull_meshes);
				ImGui::Checkbox("Use frustum cull for meshlets", &enable_frustum_cull_meshlets);
				ImGui::Checkbox("Use occlusion cull for meshes", &enable_occlusion_cull_meshes);
				ImGui::Checkbox("Use occlusion cull for meshlets", &enable_occlusion_cull_meshlets);
				ImGui::Checkbox("Freeze occlusion camera", &freeze_occlusion_camera);
				ImGui::Checkbox("Show Debug GPU Draws", &scene->show_debug_gpu_draws);

				ImGui::Checkbox("Enable Camera Inside approximation", &enable_camera_inside);
				ImGui::Checkbox("Use McGuire method for AABB sphere", &use_mcguire_method);
				ImGui::Checkbox("Skip invisible lights", &skip_invisible_lights);
				ImGui::Checkbox("use view aabb", &use_view_aabb);
				ImGui::Checkbox("force fullscreen light aabb", &force_fullscreen_light_aabb);

                ImGui::Checkbox("Dynamically recreate descriptor sets", &recreate_per_thread_descriptors);
                ImGui::Checkbox("Use secondary command buffers", &use_secondary_command_buffers);

				ImGui::SliderFloat("Animation Speed Multiplier", &animation_speed_multiplier, 0.0f, 10.0f);

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

			if (ImGui::Begin("Scene")) {

				static u32 selected_node = u32_max;

				ImGui::Text("Selected node %u", selected_node);
				if (selected_node < s_scene_graph.nodes_hierarchy.size) {

					mat4s& local_transform = s_scene_graph.local_matrices[selected_node];
					f32 position[3]{ local_transform.m30, local_transform.m31, local_transform.m32 };

					if (ImGui::SliderFloat3("Node Position", position, -100.0f, 100.0f)) {
						local_transform.m30 = position[0];
						local_transform.m31 = position[1];
						local_transform.m32 = position[2];

						s_scene_graph.set_local_matrix(selected_node, local_transform);
					}
					ImGui::Separator();
				}

				for (u32 n = 0; n < s_scene_graph.nodes_hierarchy.size; ++n) {
					const SceneGraphNodeDebugData& node_debug_data = s_scene_graph.nodes_debug_data[n];
					if (ImGui::Selectable(node_debug_data.name ? node_debug_data.name : "-", n == selected_node)) {
						selected_node = n;
					}
				}
			}
			ImGui::End();

			if (ImGui::Begin("GPU"))
			{
                renderer->imgui_draw();
                ImGui::Separator();
				ImGui::Text("Cpu Time %fms", delta_time * 1000.f);
				gpu_profiler->imgui_draw();
			}
			ImGui::End();

			if (ImGui::Begin("Textures Debug")) 
			{
				const ImVec2 window_size = ImGui::GetWindowSize();

				FrameGraphResource* resource = s_frame_graph.get_resource("depth");

				ImGui::Image((ImTextureID)&resource->resource_info.texture.handle, window_size);

			}
			ImGui::End();

			DrawGUI();
		}


		{
			MF_PROFILE_SCOPE("Animation Updates");
			scene->update_animations(delta_time * animation_speed_multiplier);
		}

		{
			MF_PROFILE_SCOPE("Matrices Updates");
			s_scene_graph.update_matrices();
		}

		{
			MF_PROFILE_SCOPE("Scene Joint Updates");
			scene->update_joints();
		}

		// Copy static render info
		render_data.animation_speed_multiplier = animation_speed_multiplier;
		render_data.enable_frustum_cull_meshes = enable_frustum_cull_meshes;
		render_data.enable_frustum_cull_meshlets = enable_frustum_cull_meshlets;
		render_data.enable_occlusion_cull_meshes = enable_occlusion_cull_meshes;
		render_data.enable_occlusion_cull_meshlets = enable_occlusion_cull_meshlets;
		render_data.freeze_occlusion_camera = freeze_occlusion_camera;
		render_data.projection_transpose = projection_transpose;

		const f32 interpolation_factor = Maths::clamp(0.0f, 1.0f, (f32)(accumulator / step));
		Render(delta_time, interpolation_factor, &render_data);

		

		// Prepare for next frame if anything must be done.
		EndFrame();
	}

	return false;
}

void Sandbox::Render(f32 delta_time, f32 interpolation_factor, void* data)
{
    using namespace Magnefu;

    // //

    if (!window->minimized)
    {

        GraphicsContext* gpu = service_manager->get<GraphicsContext>();

        RenderData* render_data = (RenderData*)data;

        {
			GpuSceneData& scene_data = scene->scene_data;
			scene_data.previous_view_projection = scene_data.view_projection;   // Cache previous view projection
			scene_data.view_projection = s_game_camera.camera.view_projection;
			scene_data.inverse_view_projection = glms_mat4_inv(s_game_camera.camera.view_projection);
			scene_data.world_to_camera = s_game_camera.camera.view;
			scene_data.camera_position = vec4s{ s_game_camera.camera.position.x, s_game_camera.camera.position.y, s_game_camera.camera.position.z, 1.0f };
			scene_data.dither_texture_index = dither_texture ? dither_texture->handle.index : 0;

			scene_data.z_near = s_game_camera.camera.near_plane;
			scene_data.z_far = s_game_camera.camera.far_plane;
			scene_data.projection_00 = s_game_camera.camera.projection.m00;
			scene_data.projection_11 = s_game_camera.camera.projection.m11;
			scene_data.frustum_cull_meshes = render_data->enable_frustum_cull_meshes ? 1 : 0;
			scene_data.frustum_cull_meshlets = render_data->enable_frustum_cull_meshlets ? 1 : 0;
			scene_data.occlusion_cull_meshes = render_data->enable_occlusion_cull_meshes ? 1 : 0;
			scene_data.occlusion_cull_meshlets = render_data->enable_occlusion_cull_meshlets ? 1 : 0;
			scene_data.freeze_occlusion_camera = render_data->freeze_occlusion_camera ? 1 : 0;

			scene_data.resolution_x = gpu->swapchain_width * 1.f;
			scene_data.resolution_y = gpu->swapchain_height * 1.f;
			scene_data.aspect_ratio = gpu->swapchain_width * 1.f / gpu->swapchain_height;

			// TEST packed light data
			scene_data.light0_data0 = vec4s{ render_data->light_position.x, render_data->light_position.y, render_data->light_position.z, render_data->light_radius };
			scene_data.light0_data1 = vec4s{ 1.f, 1.f, 1.f, render_data->light_intensity };

			// Frustum computations
			if (!render_data->freeze_occlusion_camera) {
				scene_data.world_to_camera_debug = scene_data.world_to_camera;
				scene_data.view_projection_debug = scene_data.view_projection;
				render_data->projection_transpose = glms_mat4_transpose(s_game_camera.camera.projection);
			}

			scene_data.frustum_planes[0] = normalize_plane(glms_vec4_add(render_data->projection_transpose.col[3], render_data->projection_transpose.col[0])); // x + w  < 0;
			scene_data.frustum_planes[1] = normalize_plane(glms_vec4_sub(render_data->projection_transpose.col[3], render_data->projection_transpose.col[0])); // x - w  < 0;
			scene_data.frustum_planes[2] = normalize_plane(glms_vec4_add(render_data->projection_transpose.col[3], render_data->projection_transpose.col[1])); // y + w  < 0;
			scene_data.frustum_planes[3] = normalize_plane(glms_vec4_sub(render_data->projection_transpose.col[3], render_data->projection_transpose.col[1])); // y - w  < 0;
			scene_data.frustum_planes[4] = normalize_plane(glms_vec4_add(render_data->projection_transpose.col[3], render_data->projection_transpose.col[2])); // z + w  < 0;
			scene_data.frustum_planes[5] = normalize_plane(glms_vec4_sub(render_data->projection_transpose.col[3], render_data->projection_transpose.col[2])); // z - w  < 0;


            // Update common constant buffer
            MF_PROFILE_SCOPE("Uniform Buffer Update");

            MapBufferParameters scene_cb_map = { scene->scene_cb, 0, 0 };
			GpuSceneData* gpu_scene_data = (GpuSceneData*)gpu->map_buffer(scene_cb_map);
            if (gpu_scene_data)
            {
				memcpy(gpu_scene_data, &scene->scene_data, sizeof(GpuSceneData));

                gpu->unmap_buffer(scene_cb_map);
            }

			// TODO: move light placement here.
			if (render_data->light_placement) 
			{
				render_data->light_placement = false;

				//place_lights( scene->lights, true );
			}

			u32 tile_x_count = scene_data.resolution_x / k_tile_size;
			u32 tile_y_count = scene_data.resolution_y / k_tile_size;
			u32 tiles_entry_count = tile_x_count * tile_y_count * k_num_words;
			u32 buffer_size = tiles_entry_count * sizeof(u32);

			Buffer* lights_tiles_buffer = nullptr;
			// Check just the first tile, as we destroy/create them together
			if (scene->lights_tiles_sb[0].index != k_invalid_buffer.index) {
				lights_tiles_buffer = renderer->gpu->access_buffer(scene->lights_tiles_sb[gpu->current_frame]);
			}

			if (lights_tiles_buffer == nullptr || lights_tiles_buffer->size != buffer_size) {
				for (u32 i = 0; i < k_max_frames; ++i) {
					renderer->gpu->destroy_buffer(scene->lights_tiles_sb[i]);
				}

				BufferCreation buffer_creation{ };
				buffer_creation.reset().set(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, ResourceUsageType::Dynamic, buffer_size).set_name("light_tiles");

				for (u32 i = 0; i < k_max_frames; ++i) {
					scene->lights_tiles_sb[i] = renderer->gpu->create_buffer(buffer_creation);
				}
			}


			UploadGpuDataContext upload_context{ s_game_camera, &MemoryService::Instance()->tempStackAllocator };
			upload_context.enable_camera_inside = render_data->enable_camera_inside;
			upload_context.force_fullscreen_light_aabb = render_data->force_fullscreen_light_aabb;
			upload_context.skip_invisible_lights = render_data->skip_invisible_lights;
			upload_context.use_mcguire_method = render_data->use_mcguire_method;
			upload_context.use_view_aabb = render_data->use_view_aabb;
			s_frame_renderer.upload_gpu_data(upload_context);
        }

		DrawTask draw_task;
		draw_task.init(renderer->gpu, &s_frame_graph, renderer, imgui, &s_gpu_profiler, scene, &s_frame_renderer);
		s_task_scheduler.AddTaskSetToPipe(&draw_task);


		// TODO: May need to be moved to fixed update above
		CommandBuffer* async_compute_command_buffer = nullptr;
		{
			MF_PROFILE_SCOPE("PhysicsUpdate");
			async_compute_command_buffer = scene->update_physics(delta_time, render_data->air_density, render_data->spring_stiffness, render_data->spring_damping, render_data->wind_direction, render_data->reset_simulation);
			render_data->reset_simulation = false;
		}

		s_task_scheduler.WaitforTaskSet(&draw_task);

		// Avoid using the same command buffer
		renderer->add_texture_update_commands((draw_task.thread_id + 1) % s_task_scheduler.GetNumTaskThreads());
		gpu->present(async_compute_command_buffer);

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