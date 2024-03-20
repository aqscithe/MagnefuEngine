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
#include "Magnefu/Core/Numerics.hpp"


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
#include "cglm/struct/box.h"


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
void place_lights(Magnefu::Array<Magnefu::Light>& lights, u32 active_lights, bool grid) {

	using namespace Magnefu;

	if (grid) {
		const u32 lights_per_side = ceilu32(sqrtf(active_lights * 1.f));
		for (u32 i = 0; i < active_lights; ++i) {
			Light& light = lights[i];

			const f32 x = (i % lights_per_side) - lights_per_side * .5f;
			const f32 y = 0.05f;
			const f32 z = (i / lights_per_side) - lights_per_side * .5f;

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

u32 get_cube_face_mask(vec3s cube_map_pos, vec3s aabb[2]) {

	vec3s plane_normals[] = { {-1, 1, 0}, {1, 1, 0}, {1, 0, 1}, {1, 0, -1}, {0, 1, 1}, {0, -1, 1} };
	vec3s abs_plane_normals[] = { {1, 1, 0}, {1, 1, 0}, {1, 0, 1}, {1, 0, 1}, {0, 1, 1}, {0, 1, 1} };

	vec3s center = glms_vec3_sub(glms_aabb_center(aabb), cube_map_pos);
	vec3s extents = glms_vec3_divs(glms_vec3_sub(aabb[1], aabb[0]), 2.0f);

	bool rp[6];
	bool rn[6];

	for (u32 i = 0; i < 6; ++i) {
		f32 dist = glms_vec3_dot(center, plane_normals[i]);
		f32 radius = glms_vec3_dot(extents, abs_plane_normals[i]);

		rp[i] = dist > -radius;
		rn[i] = dist < radius;
	}

	u32 fpx = rn[0] && rp[1] && rp[2] && rp[3] && aabb[1].x > cube_map_pos.x;
	u32 fnx = rp[0] && rn[1] && rn[2] && rn[3] && aabb[0].x < cube_map_pos.x;
	u32 fpy = rp[0] && rp[1] && rp[4] && rn[5] && aabb[1].y > cube_map_pos.y;
	u32 fny = rn[0] && rn[1] && rn[4] && rp[5] && aabb[0].y < cube_map_pos.y;
	u32 fpz = rp[2] && rn[3] && rp[4] && rp[5] && aabb[1].z > cube_map_pos.z;
	u32 fnz = rn[2] && rp[3] && rn[4] && rn[5] && aabb[0].z < cube_map_pos.z;

	return fpx | (fnx << 1) | (fpy << 2) | (fny << 3) | (fpz << 4) | (fnz << 5);
}



// -- Render Data ----------------------------------- //

struct RenderData
{
	float model_scale = 1.0f;

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

	vec3s aabb_test_position{ 0,0,0 };
	bool enable_aabb_cubemap_test = false;
	bool enable_light_cluster_debug = false;
	bool enable_light_tile_debug = false;
	bool debug_show_light_tiles = false;
	bool debug_show_tiles = false;
	bool debug_show_bins = false;
	bool disable_shadows = false;
	bool shadow_meshlets_cone_cull = true;
	bool shadow_meshlets_sphere_cull = true;
	bool shadow_meshes_sphere_cull = true;
	bool shadow_meshlets_cubemap_face_cull = true;
	u32 lighting_debug_modes = 0;
	u32 light_to_debug = 0;
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

	// Allocate specific resource pool sizes
	dc.resource_pool_creation.buffers = 512;
	dc.resource_pool_creation.descriptor_set_layouts = 256;
	dc.resource_pool_creation.descriptor_sets = 900;
	dc.resource_pool_creation.pipelines = 256;
	dc.resource_pool_creation.render_passes = 256;
	dc.resource_pool_creation.shaders = 256;
	dc.resource_pool_creation.samplers = 128;
	dc.resource_pool_creation.textures = 128;
	dc.descriptor_pool_creation.combined_image_samplers = 700;
	dc.descriptor_pool_creation.storage_texel_buffers = 1;
	dc.descriptor_pool_creation.uniform_texel_buffers = 1;

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
	s_game_camera.camera.init_perpective(0.1f, 100.f, 60.f, wconf.width * 1.f / wconf.height);
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
		

		// TODO: improve
		// Manually add point shadows texture format.
		FrameGraphNode* point_shadows_pass_node = s_frame_graph.get_node("point_shadows_pass");
		if (point_shadows_pass_node) {
			RenderPass* render_pass = gpu->access_render_pass(point_shadows_pass_node->render_pass);  // the issue is that this returns nullptr, but it shouldn't
			if (render_pass) {
				render_pass->output.reset().depth(VK_FORMAT_D16_UNORM, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			}
		}


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

	for (u32 i = 0; i < 6; ++i) {
		scene->cubeface_flip[i] = false;
	}

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

			static bool one_time_check = true;
			if (s_async_loader.file_load_requests.size == 0 && one_time_check) {
				one_time_check = false;
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

		static vec3s aabb_test_position{ 0,0,0 };
		static bool enable_aabb_cubemap_test = false;
		static bool enable_light_cluster_debug = false;
		static bool enable_light_tile_debug = false;
		static bool debug_show_light_tiles = false;
		static bool debug_show_tiles = false;
		static bool debug_show_bins = false;
		static bool disable_shadows = false;
		static bool shadow_meshlets_cone_cull = true;
		static bool shadow_meshlets_sphere_cull = true;
		static bool shadow_meshes_sphere_cull = true;
		static bool shadow_meshlets_cubemap_face_cull = true;
		static u32 lighting_debug_modes = 0;
		static u32 light_to_debug = 0;


		// -- ImGui New Frame -------------- //
		imgui->BeginFrame();
		

		// Draw GUI
		{
            MF_PROFILE_SCOPE("Draw GUI");

			if (ImGui::Begin("Magnefu ImGui"))
			{
                ImGui::SeparatorText("MODELS");
				ImGui::InputFloat("Scene Global Scale scale", &scene->global_scale, 0.001f);

                ImGui::SeparatorText("CAMERA");
				ImGui::InputFloat3("Camera position", s_game_camera.camera.position.raw);
				ImGui::InputFloat3("Camera target movement", s_game_camera.target_movement.raw);

				ImGui::SeparatorText("PHYSICS");
				ImGui::InputFloat3("Wind direction", render_data.wind_direction.raw);
				ImGui::InputFloat("Air density", &render_data.air_density);
				ImGui::InputFloat("Spring stiffness", &render_data.spring_stiffness);
				ImGui::InputFloat("Spring damping", &render_data.spring_damping);

				ImGui::SeparatorText("LIGHTS");
				ImGui::Checkbox("Enable AABB cubemap test", &enable_aabb_cubemap_test);
				ImGui::Checkbox("Enable light cluster debug", &enable_light_cluster_debug);
				ImGui::Checkbox("Enable light tile debug", &enable_light_tile_debug);
				ImGui::SliderFloat3("AABB test position", aabb_test_position.raw, -1.5f, 1.5f, "%1.2f");
				ImGui::Checkbox("Reset simulation", &render_data.reset_simulation);
				ImGui::SliderUint("Active Lights", &scene->active_lights, 1, k_num_lights - 1);
				ImGui::SliderUint("Light Index", &light_to_debug, 0, scene->active_lights - 1);
				ImGui::SliderFloat3("Light position", scene->lights[light_to_debug].world_position.raw, -4.f, 4.f, "%1.3f");
				ImGui::SliderFloat("Light radius", &scene->lights[light_to_debug].radius, 0.01f, 10.f, "%2.3f");
				ImGui::SliderFloat("Light intensity", &scene->lights[light_to_debug].intensity, 0.01f, 10.f, "%2.3f");
				ImGui::Checkbox("Cubeface switch Pos X", &scene->cubeface_flip[0]);
				ImGui::Checkbox("Cubeface switch Neg X", &scene->cubeface_flip[1]);
				ImGui::Checkbox("Cubeface switch Pos Y", &scene->cubeface_flip[2]);
				ImGui::Checkbox("Cubeface switch Neg Y", &scene->cubeface_flip[3]);
				ImGui::Checkbox("Cubeface switch Pos Z", &scene->cubeface_flip[4]);
				ImGui::Checkbox("Cubeface switch Neg Z", &scene->cubeface_flip[5]);

				ImGui::SeparatorText("MESHES AND MESHLETS");

				static bool enable_meshlets = false;
				enable_meshlets = scene->use_meshlets && gpu->mesh_shaders_extension_present;
				ImGui::Checkbox("Use meshlets", &enable_meshlets);
				scene->use_meshlets = enable_meshlets;
				ImGui::Checkbox("Use meshlets emulation", &scene->use_meshlets_emulation);
				ImGui::Checkbox("Use frustum cull for meshes", &enable_frustum_cull_meshes);
				ImGui::Checkbox("Use frustum cull for meshlets", &enable_frustum_cull_meshlets);
				ImGui::Checkbox("Use occlusion cull for meshes", &enable_occlusion_cull_meshes);
				ImGui::Checkbox("Use occlusion cull for meshlets", &enable_occlusion_cull_meshlets);

				ImGui::Checkbox("Use meshes sphere cull for shadows", &shadow_meshes_sphere_cull);
				ImGui::Checkbox("Use meshlets cone cull for shadows", &shadow_meshlets_cone_cull);
				ImGui::Checkbox("Use meshlets sphere cull for shadows", &shadow_meshlets_sphere_cull);
				ImGui::Checkbox("Use meshlets cubemap face cull for shadows", &shadow_meshlets_cubemap_face_cull);

				ImGui::Checkbox("Freeze occlusion camera", &freeze_occlusion_camera);
				ImGui::Checkbox("Show Debug GPU Draws", &scene->show_debug_gpu_draws);

				ImGui::SeparatorText("OPTIONS");

				ImGui::Checkbox("Enable Camera Inside approximation", &enable_camera_inside);
				ImGui::Checkbox("Use McGuire method for AABB sphere", &use_mcguire_method);
				ImGui::Checkbox("Skip invisible lights", &skip_invisible_lights);
				ImGui::Checkbox("use view aabb", &use_view_aabb);
				ImGui::Checkbox("force fullscreen light aabb", &force_fullscreen_light_aabb);
				ImGui::Checkbox("debug show light tiles", &debug_show_light_tiles);
				ImGui::Checkbox("debug show tiles", &debug_show_tiles);
				ImGui::Checkbox("debug show bins", &debug_show_bins);
				ImGui::Separator();
				ImGui::Checkbox("Pointlight rendering", &scene->pointlight_rendering);
				ImGui::Checkbox("Pointlight rendering use meshlets", &scene->pointlight_use_meshlets);
				ImGui::Checkbox("Disable shadows", &disable_shadows);
				ImGui::Checkbox("Use tetrahedron shadows", &scene->use_tetrahedron_shadows);
				ImGui::Separator();
				ImGui::SliderUint("Lighting debug modes", &lighting_debug_modes, 0, 10);


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
			}
			ImGui::End();

			if (ImGui::Begin("GPU Profiler")) {
				ImGui::Text("Cpu Time %fms", delta_time * 1000.f);
				gpu_profiler->imgui_draw();

			}
			ImGui::End();

			if (ImGui::Begin("Textures Debug")) 
			{
				static u32 texture_to_debug = 40;
				ImVec2 window_size = ImGui::GetWindowSize();
				window_size.y += 50;
				ImGui::InputScalar("Texture ID", ImGuiDataType_U32, &texture_to_debug);
				static i32 face_to = 0;
				ImGui::SliderInt("Face", &face_to, 0, 5);
				scene->cubemap_debug_face_index = (u32)face_to;
				ImGui::Checkbox("Cubemap face enabled", &scene->cubemap_face_debug_enabled);

				ImGui::Image((ImTextureID)&texture_to_debug, window_size);
			}
			ImGui::End();

			if (ImGui::Begin("Lights Debug")) {
				const u32 lights_count = scene->lights.size;

				for (u32 l = 0; l < lights_count; ++l) {
					Light& light = scene->lights[l];

					ImGui::Text("%d: %d, %d R: %0.2f a: %0.6f", l, light.tile_x, light.tile_y, light.shadow_map_resolution, light.solid_angle);
				}

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
		render_data.aabb_test_position = aabb_test_position;
		render_data.enable_aabb_cubemap_test = enable_aabb_cubemap_test;
		render_data.enable_light_cluster_debug = enable_light_cluster_debug;
		render_data.enable_light_tile_debug = enable_light_tile_debug;
		render_data.debug_show_light_tiles = debug_show_light_tiles;
		render_data.debug_show_tiles = debug_show_tiles;
		render_data.debug_show_bins = debug_show_bins;
		render_data.disable_shadows = disable_shadows;
		render_data.shadow_meshlets_cone_cull = shadow_meshlets_cone_cull;
		render_data.shadow_meshlets_sphere_cull = shadow_meshlets_sphere_cull;
		render_data.shadow_meshes_sphere_cull = shadow_meshes_sphere_cull;
		render_data.shadow_meshlets_cubemap_face_cull = shadow_meshlets_cubemap_face_cull;
		render_data.lighting_debug_modes = lighting_debug_modes;
		render_data.light_to_debug = light_to_debug;

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
			scene_data.inverse_projection = glms_mat4_inv(s_game_camera.camera.projection);
			scene_data.inverse_view = glms_mat4_inv(s_game_camera.camera.view);
			scene_data.world_to_camera = s_game_camera.camera.view;
			scene_data.camera_position = vec4s{ s_game_camera.camera.position.x, s_game_camera.camera.position.y, s_game_camera.camera.position.z, 1.0f };
			scene_data.dither_texture_index = dither_texture ? dither_texture->handle.index : 0;

			scene_data.use_tetrahedron_shadows = scene->use_tetrahedron_shadows;
			scene_data.active_lights = scene->active_lights;

			scene_data.z_near = s_game_camera.camera.near_plane;
			scene_data.z_far = s_game_camera.camera.far_plane;
			scene_data.projection_00 = s_game_camera.camera.projection.m00;
			scene_data.projection_11 = s_game_camera.camera.projection.m11;
			scene_data.culling_options = 0;
			scene_data.set_frustum_cull_meshes(render_data->enable_frustum_cull_meshes);
			scene_data.set_frustum_cull_meshlets(render_data->enable_frustum_cull_meshlets);
			scene_data.set_occlusion_cull_meshes(render_data->enable_occlusion_cull_meshes);
			scene_data.set_occlusion_cull_meshlets(render_data->enable_occlusion_cull_meshlets);
			scene_data.set_freeze_occlusion_camera(render_data->freeze_occlusion_camera);
			scene_data.set_shadow_meshlets_cone_cull(render_data->shadow_meshlets_cone_cull);
			scene_data.set_shadow_meshlets_sphere_cull(render_data->shadow_meshlets_sphere_cull);
			scene_data.set_shadow_meshlets_cubemap_face_cull(render_data->shadow_meshlets_cubemap_face_cull);

			scene_data.resolution_x = gpu->swapchain_width * 1.f;
			scene_data.resolution_y = gpu->swapchain_height * 1.f;
			scene_data.aspect_ratio = gpu->swapchain_width * 1.f / gpu->swapchain_height;
			scene_data.num_mesh_instances = scene->mesh_instances.size;


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

            MapBufferParameters cb_map = { scene->scene_cb, 0, 0 };
			GpuSceneData* gpu_scene_data = (GpuSceneData*)gpu->map_buffer(cb_map);
            if (gpu_scene_data)
            {
				memcpy(gpu_scene_data, &scene->scene_data, sizeof(GpuSceneData));

                gpu->unmap_buffer(cb_map);
            }

			cb_map.buffer = scene->lighting_constants_cb[gpu->current_frame];
			GpuLightingData* gpu_lighting_data = (GpuLightingData*)gpu->map_buffer(cb_map);
			if (gpu_lighting_data) {

				gpu_lighting_data->cubemap_shadows_index = scene->cubemap_shadows_index;
				gpu_lighting_data->debug_show_light_tiles = render_data->debug_show_light_tiles ? 1 : 0;
				gpu_lighting_data->debug_show_tiles = render_data->debug_show_tiles ? 1 : 0;
				gpu_lighting_data->debug_show_bins = render_data->debug_show_bins ? 1 : 0;
				gpu_lighting_data->disable_shadows = render_data->disable_shadows ? 1 : 0;
				gpu_lighting_data->debug_modes = (u32)render_data->lighting_debug_modes;
				gpu_lighting_data->debug_texture_index = scene->lighting_debug_texture_index;

				gpu->unmap_buffer(cb_map);
			}

			// TODO: move light placement here.
			if (render_data->light_placement) 
			{
				render_data->light_placement = false;

				//place_lights( scene->lights, true );
			}

			const u32 tile_x_count = ceilu32(scene_data.resolution_x / k_tile_size);
			const u32 tile_y_count = ceilu32(scene_data.resolution_y / k_tile_size);
			const u32 tiles_entry_count = tile_x_count * tile_y_count * k_num_words;
			const u32 buffer_size = tiles_entry_count * sizeof(u32);

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



			// Place light AABB with a smaller aabb to indicate the center.
			const Light& light = scene->lights[render_data->light_to_debug];
			f32 half_radius = light.radius;
			scene->debug_renderer.aabb(glms_vec3_sub(light.world_position, { half_radius, half_radius ,half_radius }), glms_vec3_add(light.world_position, { half_radius, half_radius , half_radius }), { Color::white });
			scene->debug_renderer.aabb(glms_vec3_sub(light.world_position, { .1, .1, .1 }), glms_vec3_add(light.world_position, { .1, .1, .1 }), { Color::green });

			//f32 distance = glms_vec3_distance( { 0,0,0 }, light.world_position );
			//f32 distance_normalized = distance / (half_radius * 2.f);
			//f32 f = half_radius * 2;
			//f32 n = 0.01f;
			//float NormZComp = ( f + n ) / ( f - n ) - ( 2 * f * n ) / ( f - n ) / distance;
			//float NormZComp2 = ( f ) / ( n - f ) - ( f * n ) / ( n - f ) / distance;

			//// return z_near * z_far / (z_far + depth * (z_near - z_far));
			//f32 linear_d = n * f / ( f + 0.983 * ( n - f ) );
			//f32 linear_d2 = n * f / ( f + 1 * ( n - f ) );
			//f32 linear_d3 = n * f / ( f + 0.01 * ( n - f ) );

			//// ( f + z * ( n - f ) ) * lin_z = n * f;
			//// f * lin_z + (z * lin_z * (n - f ) = n * f
			//// ((n * f) - f * lin_z ) / (n - f) = z * lin_z

			//NormZComp = ( f + n ) / ( f - n ) - ( 2 * f * n ) / ( f - n ) / n;
			//NormZComp = ( f + n ) / ( f - n ) - ( 2 * f * n ) / ( f - n ) / f;
			//NormZComp2 = -( f ) / ( n - f ) - ( f * n ) / ( n - f ) / n;
			//NormZComp2 = -( f ) / ( n - f ) - ( f * n ) / ( n - f ) / f;

			//mat4s view = glms_look( light.world_position, { 0,0,-1 }, { 0,-1,0 } );
			//// TODO: this should be radius of the light.
			//mat4s projection = glms_perspective( glm_rad( 90.f ), 1.f, 0.01f, light.radius );
			//mat4s view_projection = glms_mat4_mul( projection, view );

			//vec3s pos_cs = project( view_projection, { 0,0,0 } );

			//rprint( "DDDD %f %f %f %f\n", NormZComp, -NormZComp2, linear_d, pos_cs.z );
			//{
			//    float fn = 1.0f / ( 0.01f - light.radius );
			//    float a = ( 0.01f + light.radius ) * fn;
			//    float b = 2.0f * 0.01f * light.radius * fn;
			//    float projectedDistance = light.world_position.z;
			//    float z = projectedDistance * a + b;
			//    float dbDistance = z / projectedDistance;

			//    float bc = dbDistance - NormZComp;
			//    float bd = dbDistance - NormZComp2;
			//}
			// Test AABB cubemap intersection method
			if (render_data->enable_aabb_cubemap_test) {
				// Draw enclosing cubemap aabb
				vec3s cubemap_position = { 0.f, 0.f, 0.f };
				vec3s cubemap_half_size = { 1, 1, 1 };
				scene->debug_renderer.aabb(glms_vec3_sub(cubemap_position, cubemap_half_size), glms_vec3_add(cubemap_position, cubemap_half_size), { Color::blue });

				vec3s aabb[] = { glms_vec3_subs(render_data->aabb_test_position, 0.2f), glms_vec3_adds(render_data->aabb_test_position, 0.2f) };
				u32 res = get_cube_face_mask(cubemap_position, aabb);
				// Positive X
				if ((res & 1)) {
					scene->debug_renderer.aabb(glms_vec3_add(cubemap_position, { 1,0,0 }), glms_vec3_add(cubemap_position, { 1.2, .2, .2 }), { Color::get_distinct_color(0) });
				}
				// Negative X
				if ((res & 2)) {
					scene->debug_renderer.aabb(glms_vec3_add(cubemap_position, { -1,0,0 }), glms_vec3_add(cubemap_position, { -1.2, -.2, -.2 }), { Color::get_distinct_color(1) });
				}
				// Positive Y
				if ((res & 4)) {
					scene->debug_renderer.aabb(glms_vec3_add(cubemap_position, { 0,1,0 }), glms_vec3_add(cubemap_position, { .2, 1.2, .2 }), { Color::get_distinct_color(2) });
				}
				// Negative Y
				if ((res & 8)) {
					scene->debug_renderer.aabb(glms_vec3_add(cubemap_position, { 0,-1,0 }), glms_vec3_add(cubemap_position, { .2, -1.2, .2 }), { Color::get_distinct_color(3) });
				}
				// Positive Z
				if ((res & 16)) {
					scene->debug_renderer.aabb(glms_vec3_add(cubemap_position, { 0,0,1 }), glms_vec3_add(cubemap_position, { .2, .2, 1.2 }), { Color::get_distinct_color(4) });
				}
				// Negative Z
				if ((res & 32)) {
					scene->debug_renderer.aabb(glms_vec3_add(cubemap_position, { 0,0,-1 }), glms_vec3_add(cubemap_position, { .2, .2, -1.2 }), { Color::get_distinct_color(5) });
				}
				// Draw aabb to test inside cubemap
				scene->debug_renderer.aabb(aabb[0], aabb[1], { Color::white });
				//scene->debug_renderer.line( { -1,-1,-1 }, { 1,1,1 }, { Color::white } );
				//scene->debug_renderer.line( { -1,-1,1 }, { 1,1,-1 }, { Color::white } );

				/*scene->debug_renderer.line({0.5,0,-0.5}, {-1 + .5,1,0 - .5}, {Color::blue});
				scene->debug_renderer.line( { -0.5,0,-0.5 }, { 1 - .5,1,0 - .5 }, { Color::green } );
				scene->debug_renderer.line( { 0,0,0 }, { 1,0,1 }, { Color::red } );
				scene->debug_renderer.line( { 0,0,0 }, { 1,0,-1 }, { Color::yellow } );
				scene->debug_renderer.line( { 0,0,0 }, { 0,1,1 }, { Color::white } );
				scene->debug_renderer.line( { 0,0,0 }, { 0,-1,1 }, { 0xffffff00 } ); */

				// AABB -> cubemap face rectangle test
				f32 s_min, s_max, t_min, t_max;
				project_aabb_cubemap_positive_x(aabb, s_min, s_max, t_min, t_max);
				//rprint( "POS X s %f,%f | t %f,%f\n", s_min, s_max, t_min, t_max );
				project_aabb_cubemap_negative_x(aabb, s_min, s_max, t_min, t_max);
				//rprint( "NEG X s %f,%f | t %f,%f\n", s_min, s_max, t_min, t_max );
				project_aabb_cubemap_positive_y(aabb, s_min, s_max, t_min, t_max);
				//rprint( "POS Y s %f,%f | t %f,%f\n", s_min, s_max, t_min, t_max );
				project_aabb_cubemap_negative_y(aabb, s_min, s_max, t_min, t_max);
				//rprint( "NEG Y s %f,%f | t %f,%f\n", s_min, s_max, t_min, t_max );
				project_aabb_cubemap_positive_z(aabb, s_min, s_max, t_min, t_max);
				//rprint( "POS Z s %f,%f | t %f,%f\n", s_min, s_max, t_min, t_max );
				project_aabb_cubemap_negative_z(aabb, s_min, s_max, t_min, t_max);
				//rprint( "NEG Z s %f,%f | t %f,%f\n", s_min, s_max, t_min, t_max );
			}

			if (false) {
				// NOTE(marco): adpated from http://www.aortiz.me/2018/12/21/CG.html#clustered-shading
				const u32 z_count = 32;
				const f32 tile_size = 64.0f;
				const f32 tile_pixels = tile_size * tile_size;
				const u32 tile_x_count = scene_data.resolution_x / f32(tile_size);
				const u32 tile_y_count = scene_data.resolution_y / f32(tile_size);

				const f32 tile_radius_sq = ((tile_size * 0.5f) * (tile_size * 0.5f)) * 2;

				const vec3s eye_pos = vec3s{ 0, 0, 0 };

				static Camera last_camera{ };

				if (!render_data->freeze_occlusion_camera) {
					last_camera = s_game_camera.camera;
				}

				mat4s inverse_projection = glms_mat4_inv(last_camera.projection);
				mat4s inverse_view = glms_mat4_inv(last_camera.view);

				auto screen_to_view = [&](const vec4s& screen_pos) -> vec3s {
					//Convert to NDC
					vec2s text_coord{ screen_pos.x / scene_data.resolution_x, screen_pos.y / scene_data.resolution_y };

					//Convert to clipSpace
					vec4s clip = vec4s{ text_coord.x * 2.0f - 1.0f,
										(1.0f - text_coord.y) * 2.0f - 1.0f,
										screen_pos.z,
										screen_pos.w };

					//View space transform
					vec4s view = glms_mat4_mulv(inverse_projection, clip);

					//Perspective projection
					// view = glms_vec4_scale( view, 1.0f / view.w );

					return vec3s{ view.x, view.y, view.z };
				};

				auto line_intersection_to_z_plane = [&](const vec3s& a, const vec3s& b, f32 z) -> vec3s {
					//all clusters planes are aligned in the same z direction
					vec3s normal = vec3s{ 0.0, 0.0, 1.0 };

					//getting the line from the eye to the tile
					vec3s ab = glms_vec3_sub(b, a);

					//Computing the intersection length for the line and the plane
					f32 t = (z - glms_dot(normal, a)) / glms_dot(normal, ab);

					//Computing the actual xyz position of the point along the line
					vec3s result = glms_vec3_add(a, glms_vec3_scale(ab, t));

					return result;
				};

				const f32 z_near = scene_data.z_near;
				const f32 z_far = scene_data.z_far;
				const f32 z_ratio = z_far / z_near;
				const f32 z_bin_range = 1.0f / f32(z_count);

				u32 light_count = scene->active_lights;

				Array<vec3s> lights_aabb_view;
				lights_aabb_view.init(&MemoryService::Instance()->systemAllocator, light_count * 2, light_count * 2);

				for (u32 l = 0; l < light_count; ++l) {
					Light& light = scene->lights[l];
					light.shadow_map_resolution = 0.0f;
					light.tile_x = 0;
					light.tile_y = 0;
					light.solid_angle = 0.0f;

					vec4s aabb_min_view = glms_mat4_mulv(last_camera.view, light.aabb_min);
					vec4s aabb_max_view = glms_mat4_mulv(last_camera.view, light.aabb_max);

					lights_aabb_view[l * 2] = vec3s{ aabb_min_view.x, aabb_min_view.y, aabb_min_view.z };
					lights_aabb_view[l * 2 + 1] = vec3s{ aabb_max_view.x, aabb_max_view.y, aabb_max_view.z };
				}

				for (u32 z = 0; z < z_count; ++z) {
					for (u32 y = 0; y < tile_y_count; ++y) {
						for (u32 x = 0; x < tile_x_count; ++x) {
							// Calculating the min and max point in screen space
							vec4s max_point_screen = vec4s{ f32((x + 1) * tile_size),
															f32((y + 1) * tile_size),
															0.0f, 1.0f }; // Top Right

							vec4s min_point_screen = vec4s{ f32(x * tile_size),
															f32(y * tile_size),
															0.0f, 1.0f }; // Top Right

							vec4s tile_center_screen = glms_vec4_scale(glms_vec4_add(min_point_screen, max_point_screen), 0.5f);
							vec2s tile_center{ tile_center_screen.x, tile_center_screen.y };

							// Pass min and max to view space
							vec3s max_point_view = screen_to_view(max_point_screen);
							vec3s min_point_view = screen_to_view(min_point_screen);

							// Near and far values of the cluster in view space
							// We use equation (2) directly to obtain the tile values
							f32 tile_near = z_near * pow(z_ratio, f32(z) * z_bin_range);
							f32 tile_far = z_near * pow(z_ratio, f32(z + 1) * z_bin_range);

							// Finding the 4 intersection points made from each point to the cluster near/far plane
							vec3s min_point_near = line_intersection_to_z_plane(eye_pos, min_point_view, tile_near);
							vec3s min_point_far = line_intersection_to_z_plane(eye_pos, min_point_view, tile_far);
							vec3s max_point_near = line_intersection_to_z_plane(eye_pos, max_point_view, tile_near);
							vec3s max_point_far = line_intersection_to_z_plane(eye_pos, max_point_view, tile_far);

							vec3s min_point_aabb_view = glms_vec3_minv(glms_vec3_minv(min_point_near, min_point_far), glms_vec3_minv(max_point_near, max_point_far));
							vec3s max_point_aabb_view = glms_vec3_maxv(glms_vec3_maxv(min_point_near, min_point_far), glms_vec3_maxv(max_point_near, max_point_far));

							vec4s min_point_aabb_world{ min_point_aabb_view.x, min_point_aabb_view.y, min_point_aabb_view.z, 1.0f };
							vec4s max_point_aabb_world{ max_point_aabb_view.x, max_point_aabb_view.y, max_point_aabb_view.z, 1.0f };

							min_point_aabb_world = glms_mat4_mulv(inverse_view, min_point_aabb_world);
							max_point_aabb_world = glms_mat4_mulv(inverse_view, max_point_aabb_world);

							bool intersects_light = false;
							for (u32 l = 0; l < scene->active_lights; ++l) {
								Light& light = scene->lights[l];

								vec3s& light_aabb_min = lights_aabb_view[l * 2];
								vec3s& light_aabb_max = lights_aabb_view[l * 2 + 1];

								f32 minx = min(min(light_aabb_min.x, light_aabb_max.x), min(min_point_aabb_view.x, max_point_aabb_view.x));
								f32 miny = min(min(light_aabb_min.y, light_aabb_max.y), min(min_point_aabb_view.y, max_point_aabb_view.y));
								f32 minz = min(min(light_aabb_min.z, light_aabb_max.z), min(min_point_aabb_view.z, max_point_aabb_view.z));

								f32 maxx = max(max(light_aabb_min.x, light_aabb_max.x), max(min_point_aabb_view.x, max_point_aabb_view.x));
								f32 maxy = max(max(light_aabb_min.y, light_aabb_max.y), max(min_point_aabb_view.y, max_point_aabb_view.y));
								f32 maxz = max(max(light_aabb_min.z, light_aabb_max.z), max(min_point_aabb_view.z, max_point_aabb_view.z));

								f32 dx = abs(maxx - minx);
								f32 dy = abs(maxy - miny);
								f32 dz = abs(maxz - minz);

								f32 allx = abs(light_aabb_max.x - light_aabb_min.x) + abs(max_point_aabb_view.x - min_point_aabb_view.x);
								f32 ally = abs(light_aabb_max.y - light_aabb_min.y) + abs(max_point_aabb_view.y - min_point_aabb_view.y);
								f32 allz = abs(light_aabb_max.z - light_aabb_min.z) + abs(max_point_aabb_view.z - min_point_aabb_view.z);

								bool intersects = (dx <= allx) && (dy < ally) && (dz <= allz);

								if (intersects) {
									intersects_light = true;

									vec4s sphere_world{ light.world_position.x, light.world_position.y, light.world_position.z, 1.0f };
									vec4s sphere_ndc = glms_mat4_mulv(last_camera.view_projection, sphere_world);

									sphere_ndc.x /= sphere_ndc.w;
									sphere_ndc.y /= sphere_ndc.w;

									vec2s sphere_screen{ ((sphere_ndc.x + 1.0f) * 0.5f) * scene_data.resolution_x, ((sphere_ndc.y + 1.0f) * 0.5f) * scene_data.resolution_y, };

									f32 d = glms_vec2_distance(sphere_screen, tile_center);

									f32 diff = d * d - tile_radius_sq;

									if (diff < 1.0e-4) {
										continue;
									}

									// NOTE(marco): as defined in https://math.stackexchange.com/questions/73238/calculating-solid-angle-for-a-sphere-in-space
									f32 solid_angle = (2.0f * rpi) * (1.0f - (sqrtf(diff) / d));

									// NOTE(marco): following https://efficientshading.com/wp-content/uploads/s2015_shadows.pdf
									f32 resolution = sqrtf((4.0f * rpi * tile_pixels) / (6 * solid_angle));

									if (resolution > light.shadow_map_resolution) {
										light.shadow_map_resolution = resolution;
										light.tile_x = x;
										light.tile_y = y;
										light.solid_angle = solid_angle;
									}
								}
							}

							if (render_data->enable_light_cluster_debug && intersects_light) {
								scene->debug_renderer.aabb(vec3s{ min_point_aabb_world.x, min_point_aabb_world.y, min_point_aabb_world.z },
									vec3s{ max_point_aabb_world.x, max_point_aabb_world.y, max_point_aabb_world.z },
									{ Color::get_distinct_color(z) });
							}
						}
					}
				}

				lights_aabb_view.shutdown();

				if (render_data->enable_light_tile_debug) {
					f32 light_pos_len = 0.01;
					for (u32 l = 0; l < light_count; ++l) {
						Light& light = scene->lights[l];

						//rprint( "Light resolution %f\n", light.shadow_map_resolution );

						if (light.shadow_map_resolution != 0.0f) {
							{
								vec4s sphere_world{ light.world_position.x, light.world_position.y, light.world_position.z, 1.0f };
								vec4s sphere_ndc = glms_mat4_mulv(last_camera.view_projection, sphere_world);

								sphere_ndc.x /= sphere_ndc.w;
								sphere_ndc.y /= sphere_ndc.w;

								vec2s top_left{ sphere_ndc.x - light_pos_len, sphere_ndc.y - light_pos_len };
								vec2s bottom_right{ sphere_ndc.x + light_pos_len, sphere_ndc.y + light_pos_len };
								vec2s top_right{ sphere_ndc.x + light_pos_len, sphere_ndc.y - light_pos_len };
								vec2s bottom_left{ sphere_ndc.x - light_pos_len, sphere_ndc.y + light_pos_len };

								scene->debug_renderer.line_2d(top_left, bottom_right, { Color::get_distinct_color(l + 1) });
								scene->debug_renderer.line_2d(top_right, bottom_left, { Color::get_distinct_color(l + 1) });
							}

							{
								vec2s screen_scale{ 1.0f / f32(scene_data.resolution_x), 1.0f / (scene_data.resolution_y) };

								vec2s bottom_right{ f32((light.tile_x + 1) * tile_size), f32(scene_data.resolution_y - (light.tile_y + 1) * tile_size) };
								bottom_right = glms_vec2_subs(glms_vec2_scale(glms_vec2_mul(bottom_right, screen_scale), 2.0f), 1.0f);

								vec2s top_left{ f32((light.tile_x) * tile_size), f32(scene_data.resolution_y - (light.tile_y) * tile_size) };
								top_left = glms_vec2_subs(glms_vec2_scale(glms_vec2_mul(top_left, screen_scale), 2.0f), 1.0f);

								vec2s top_right{ bottom_right.x, top_left.y };
								vec2s bottom_left{ top_left.x, bottom_right.y };

								scene->debug_renderer.line_2d(top_left, top_right, { Color::get_distinct_color(l + 1) });
								scene->debug_renderer.line_2d(top_right, bottom_right, { Color::get_distinct_color(l + 1) });
								scene->debug_renderer.line_2d(bottom_left, bottom_right, { Color::get_distinct_color(l + 1) });
								scene->debug_renderer.line_2d(bottom_left, top_left, { Color::get_distinct_color(l + 1) });
							}
						}
					}
				}
			}
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