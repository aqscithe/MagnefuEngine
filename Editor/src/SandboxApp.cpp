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
static Magnefu::TextureResource* blue_noise_128_rg_texture = nullptr;
static Magnefu::SamplerHandle repeat_sampler, repeat_nearest_sampler;



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

static void DrawNode(u32 node_index, u32& selected_node, u32& meshlet_count, const Magnefu::SceneGraph& scene_graph, Magnefu::RenderScene* scene) {
	const Magnefu::SceneGraphNodeDebugData& node_debug_data = scene_graph.nodes_debug_data[node_index];
	bool is_selected = (node_index == selected_node);

	// Option for a tree node
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
	if (is_selected)
		node_flags |= ImGuiTreeNodeFlags_Selected;

	bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)node_index, node_flags, node_debug_data.name ? node_debug_data.name : "-");

	// If the node is selected
	if (ImGui::IsItemClicked()) {
		selected_node = node_index;
		if (scene->mesh_instances.count() > node_debug_data.mesh_index)
		{
			meshlet_count = scene->mesh_instances[node_debug_data.mesh_index].mesh->meshlet_count;
		}
	}

	// If this is a leaf node, there's no need to attempt to draw children
	if (node_open) {
		u32 child_index = scene_graph.nodes_hierarchy[node_index].first_child_index;
		for (u32 i = 0; i < scene_graph.nodes_hierarchy[node_index].children_count; ++i) {
			DrawNode(child_index, selected_node, meshlet_count, scene_graph, scene);
			child_index++;
		}
		ImGui::TreePop();
	}
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


static void perform_geometric_tests(bool enable_aabb_cubemap_test, Magnefu::RenderScene* scene, const vec3s& aabb_test_position,
	Magnefu::GpuSceneData& scene_data, bool freeze_occlusion_camera, Magnefu::GameCamera& game_camera,
	bool enable_light_tile_debug, Magnefu::Allocator* allocator, bool enable_light_cluster_debug) {

	using namespace Magnefu;

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
	if (enable_aabb_cubemap_test) {
		// Draw enclosing cubemap aabb
		vec3s cubemap_position = { 0.f, 0.f, 0.f };
		vec3s cubemap_half_size = { 1, 1, 1 };
		scene->debug_renderer.aabb(glms_vec3_sub(cubemap_position, cubemap_half_size), glms_vec3_add(cubemap_position, cubemap_half_size), { Color::blue });

		vec3s aabb[] = { glms_vec3_subs(aabb_test_position, 0.2f), glms_vec3_adds(aabb_test_position, 0.2f) };
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

		if (!freeze_occlusion_camera) {
			last_camera = game_camera.camera;
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
		lights_aabb_view.init(allocator, light_count * 2, light_count * 2);

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

					if (enable_light_cluster_debug && intersects_light) {
						scene->debug_renderer.aabb(vec3s{ min_point_aabb_world.x, min_point_aabb_world.y, min_point_aabb_world.z },
							vec3s{ max_point_aabb_world.x, max_point_aabb_world.y, max_point_aabb_world.z },
							{ Color::get_distinct_color(z) });
					}
				}
			}
		}

		lights_aabb_view.shutdown();

		if (enable_light_tile_debug) {
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

// Enums

namespace JitterType {
	enum Enum {
		Halton = 0,
		R2,
		Hammersley,
		InterleavedGradients
	};

	cstring names[] = { "Halton", "Martin Robert R2", "Hammersley", "Interleaved Gradients" };

} // namespace JitterType



// -- Render Data ----------------------------------- //

struct RenderData
{
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

	vec2s jitter_offsets{};
	vec2s last_clicked_position{};

	vec3s raytraced_shadow_light_direction = vec3s{ 0, 1, -0.2f };
	vec3s raytraced_shadow_light_position = vec3s{ 0, 1, 0 };
	float raytraced_shadow_light_intensity = 5.0f;
	i32 raytraced_shadow_light_type = 0;
	f32 raytraced_shadow_light_radius = 10.f;
	vec3s raytraced_shadow_light_color = vec3s{ 1, 1, 1 };


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
	dc.resource_pool_creation.textures = 256;
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

	s_scene_graph.init(allocator, 4);

	// [TAG: Multithreading]
	s_async_loader.init(renderer, &s_task_scheduler, allocator);


	Directory cwd{ };
	directory_current(&cwd);

	temporary_name_buffer.clear();
	cstring scene_paths[SCENE_COUNT];

	// TODO: [leon] should read in scene list from separate file instead of 
	// from glTF.hpp
	InjectDefault3DModel(scene_paths);


	//for (i32 scene_i = 1; scene_i < SCENE_COUNT; ++scene_i) { 
	for (i32 scene_i = 0; scene_i < SCENE_COUNT; ++scene_i) { // why does book start from 1 instead of 0 ?
		cstring scene_path = scene_paths[scene_i];
		sizet scene_path_len = strlen(scene_paths[scene_i]);

		char file_base_path[512]{ };
		memcpy(file_base_path, scene_path, scene_path_len);
		file_directory_from_path(file_base_path);

		directory_change(file_base_path);

		char file_name[512]{ };
		memcpy(file_name, scene_path, scene_path_len);
		file_name_from_path(file_name);

		char* file_extension = file_extension_from_path(file_name);

		if (scene == nullptr) {
			// TODO(marco): further refactor to allow different formats
			if (strcmp(file_extension, "gltf") == 0) {
				scene = new glTFScene;
			}
			else if (strcmp(file_extension, "obj") == 0) {
				scene = new ObjScene;
			}
			scene->init(&s_scene_graph, allocator, renderer);
			scene->use_meshlets = gpu->mesh_shaders_extension_present;
			scene->use_meshlets_emulation = !scene->use_meshlets;
		}

		scene->add_mesh(file_name, file_base_path, scratch_allocator, &s_async_loader);
	}

	// NOTE(marco): restore working directory
	directory_change(cwd.path);

	s_frame_graph_builder.init(gpu);
	s_frame_graph.init(&s_frame_graph_builder);

	if (gpu->fragment_shading_rate_present)
	{
		TextureCreation texture_creation{ };
		u32 adjusted_width = (window->GetWidth() + gpu->min_fragment_shading_rate_texel_size.width - 1) / gpu->min_fragment_shading_rate_texel_size.width;
		u32 adjusted_height = (window->GetHeight() + gpu->min_fragment_shading_rate_texel_size.height - 1) / gpu->min_fragment_shading_rate_texel_size.height;
		texture_creation.set_size(adjusted_width, adjusted_height, 1).set_format_type(VK_FORMAT_R8_UINT, TextureType::Texture2D).set_mips(1).set_layers(1).set_flags(TextureFlags::Compute_mask | TextureFlags::ShadingRate_mask).set_name("fragment_shading_rate");

		scene->fragment_shading_rate_image = gpu->create_texture(texture_creation);

		FrameGraphResourceInfo resource_info{ };

		resource_info.set_external_texture_2d(adjusted_width, adjusted_height, VK_FORMAT_R8_UINT, 0, scene->fragment_shading_rate_image);

		s_frame_graph.add_resource("shading_rate_image", FrameGraphResourceType_ShadingRate, resource_info);
	}



	// will eventually have an EditorLayer
	// may even have more specific layers like Physics Collisions and AI...
	// this is how updates can be controlled separately

	// TODO: How should i give the app access to its layer and overlay?

	layer_stack = new Magnefu::LayerStack();
	layer_stack->PushLayer(new SandboxLayer());
	layer_stack->PushOverlay(new Overlay());


	// Load frame graph and parse gpu techniques

	{
		//cstring frame_graph_path = temporary_name_buffer.append_use_f("%s%s", MAGNEFU_FRAME_GRAPH_FOLDER, "graph.json");
		cstring frame_graph_path = temporary_name_buffer.append_use_f("%s%s", MAGNEFU_FRAME_GRAPH_FOLDER, "graph_ray_tracing.json");

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

		// Cache frame graph resources in scene
		FrameGraphResource* resource = s_frame_graph.get_resource("motion_vectors");
		if (resource) {
			scene->motion_vector_texture = resource->resource_info.texture.handle;
		}

		resource = s_frame_graph.get_resource("visibility_motion_vectors");
		if (resource) {
			scene->visibility_motion_vector_texture = resource->resource_info.texture.handle;
		}

		s_render_resources_loader.init(renderer, scratch_allocator, &s_frame_graph);

		SamplerCreation sampler_creation;
		sampler_creation.set_address_mode_uv(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT)
			.set_min_mag_mip(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR).set_name("repeat_sampler");

		repeat_sampler = gpu->create_sampler(sampler_creation);
		sampler_creation.set_min_mag_mip(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST).set_name("repeat_nearest_sampler");
		repeat_nearest_sampler = gpu->create_sampler(sampler_creation);

		// TODO: add this to render graph itself.
		// Add utility textures (dithering, blue noise ...)
		temporary_name_buffer.clear();
		cstring dither_texture_path = temporary_name_buffer.append_use_f("%s/BayerDither4x4.png", MAGNEFU_TEXTURE_FOLDER);
		dither_texture = s_render_resources_loader.load_texture(dither_texture_path, false);

		gpu->link_texture_sampler(dither_texture->handle, repeat_nearest_sampler);

		temporary_name_buffer.clear();

		cstring blue_noise_texture_path = temporary_name_buffer.append_use_f("%s/LDR_RG01_0.png", MAGNEFU_TEXTURE_FOLDER);
		blue_noise_128_rg_texture = s_render_resources_loader.load_texture(blue_noise_texture_path, false);

		gpu->link_texture_sampler(blue_noise_128_rg_texture->handle, repeat_sampler);

		scene->blue_noise_128_rg_texture_index = blue_noise_128_rg_texture->handle.index;

		// Parse techniques
		GpuTechniqueCreation gtc;
		const bool use_shader_cache = true;
		auto parse_technique = [&](cstring technique_name) {
			temporary_name_buffer.clear();
			cstring path = temporary_name_buffer.append_use_f("%s/%s", MAGNEFU_SHADER_FOLDER, technique_name);
			s_render_resources_loader.load_gpu_technique(path, use_shader_cache);
		};

		cstring techniques[] = { "reflections.json", "ddgi.json", "ray_tracing.json", "meshlet.json", "fullscreen.json", "main.json",
								 "pbr_lighting.json", "dof.json", "cloth.json", "debug.json",
								 "culling.json", "volumetric_fog.json" };



		const sizet num_techniques = ArraySize(techniques);
		for (sizet t = 0; t < num_techniques; ++t) {
			parse_technique(techniques[t]);
		}

	}


	// NOTE(marco): build AS before preparing draws
	{
		CommandBuffer* gpu_commands = gpu->get_command_buffer(0, 0, true);

		// NOTE(marco): build BLAS
		VkAccelerationStructureBuildGeometryInfoKHR as_info{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		as_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		as_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
		as_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		as_info.geometryCount = scene->geometries.size;
		as_info.pGeometries = scene->geometries.data;

		Array<u32> max_primitives_count;
		max_primitives_count.init(gpu->allocator, scene->geometries.size, scene->geometries.size);

		for (u32 range_index = 0; range_index < scene->geometries.size; range_index++) {
			max_primitives_count[range_index] = scene->build_range_infos[range_index].primitiveCount;
		}

		VkAccelerationStructureBuildSizesInfoKHR as_size_info{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
		gpu->vkGetAccelerationStructureBuildSizesKHR(gpu->vulkan_device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &as_info, max_primitives_count.data, &as_size_info);

		BufferCreation as_buffer_creation{ };
		as_buffer_creation.set(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, ResourceUsageType::Immutable, as_size_info.accelerationStructureSize).set_device_only(true).set_name("blas_buffer");
		scene->blas_buffer = gpu->create_buffer(as_buffer_creation);

		Buffer* blas_buffer = gpu->access_buffer(scene->blas_buffer);

		as_buffer_creation.reset().set(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR, ResourceUsageType::Immutable, as_size_info.buildScratchSize).set_device_only(true).set_name("blas_scratch_buffer");

		BufferHandle blas_scratch_buffer_handle = gpu->create_buffer(as_buffer_creation);

		VkAccelerationStructureCreateInfoKHR as_create_info{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
		as_create_info.buffer = blas_buffer->vk_buffer;
		as_create_info.offset = 0;
		as_create_info.size = as_size_info.accelerationStructureSize;
		as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

		gpu->vkCreateAccelerationStructureKHR(gpu->vulkan_device, &as_create_info, gpu->vulkan_allocation_callbacks, &scene->blas);

		as_info.dstAccelerationStructure = scene->blas;

		as_info.scratchData.deviceAddress = gpu->get_buffer_device_address(blas_scratch_buffer_handle);

		VkAccelerationStructureBuildRangeInfoKHR* blas_ranges[] = {
			scene->build_range_infos.data
		};

		gpu->vkCmdBuildAccelerationStructuresKHR(gpu_commands->vk_command_buffer, 1, &as_info, blas_ranges);

		gpu->submit_immediate(gpu_commands);

		// NOTE(marco): build TLAS
		VkAccelerationStructureDeviceAddressInfoKHR blas_address_info{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
		blas_address_info.accelerationStructure = scene->blas;

		VkDeviceAddress blas_address = gpu->vkGetAccelerationStructureDeviceAddressKHR(gpu->vulkan_device, &blas_address_info);

		VkAccelerationStructureInstanceKHR tlas_structure{ };
		// NOTE(marco): identity matrix
		tlas_structure.transform.matrix[0][0] = 1.0f;
		tlas_structure.transform.matrix[1][1] = 1.0f;
		tlas_structure.transform.matrix[2][2] = -1.0f;
		tlas_structure.mask = 0xff;
		tlas_structure.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		tlas_structure.accelerationStructureReference = blas_address;

		as_buffer_creation.reset().set(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, ResourceUsageType::Immutable, sizeof(VkAccelerationStructureInstanceKHR)).set_data(&tlas_structure).set_name("tlas_instance_buffer");
		BufferHandle tlas_instance_buffer_handle = gpu->create_buffer(as_buffer_creation);

		VkAccelerationStructureGeometryKHR tlas_geometry{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
		tlas_geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		tlas_geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		tlas_geometry.geometry.instances.arrayOfPointers = false;
		tlas_geometry.geometry.instances.data.deviceAddress = gpu->get_buffer_device_address(tlas_instance_buffer_handle);

		as_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		as_info.geometryCount = 1;
		as_info.pGeometries = &tlas_geometry;

		u32 max_instance_count = 1;

		gpu->vkGetAccelerationStructureBuildSizesKHR(gpu->vulkan_device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &as_info, &max_instance_count, &as_size_info);

		as_buffer_creation.reset().set(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, ResourceUsageType::Immutable, as_size_info.accelerationStructureSize).set_device_only(true).set_name("tlas_buffer");
		scene->tlas_buffer = gpu->create_buffer(as_buffer_creation);

		Buffer* tlas_buffer = gpu->access_buffer(scene->tlas_buffer);

		as_buffer_creation.reset().set(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR, ResourceUsageType::Immutable, as_size_info.buildScratchSize).set_device_only(true).set_name("tlas_scratch_buffer");

		BufferHandle tlas_scratch_buffer_handle = gpu->create_buffer(as_buffer_creation);

		as_create_info.buffer = tlas_buffer->vk_buffer;
		as_create_info.offset = 0;
		as_create_info.size = as_size_info.accelerationStructureSize;
		as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

		gpu->vkCreateAccelerationStructureKHR(gpu->vulkan_device, &as_create_info, gpu->vulkan_allocation_callbacks, &scene->tlas);

		as_info.dstAccelerationStructure = scene->tlas;

		as_info.scratchData.deviceAddress = gpu->get_buffer_device_address(tlas_scratch_buffer_handle);

		VkAccelerationStructureBuildRangeInfoKHR tlas_range_info{ };
		tlas_range_info.primitiveCount = 1;

		VkAccelerationStructureBuildRangeInfoKHR* tlas_ranges[] = {
			&tlas_range_info
		};

		gpu_commands->reset();

		gpu_commands->begin();

		// TODO(marco): we shouldn't be doing this manually
		GpuThreadFramePools* thread_pools = gpu_commands->thread_frame_pool;
		thread_pools->time_queries->reset();
		vkCmdResetQueryPool(gpu_commands->vk_command_buffer, thread_pools->vulkan_timestamp_query_pool, 0, thread_pools->time_queries->time_queries.size);

		vkCmdResetQueryPool(gpu_commands->vk_command_buffer, thread_pools->vulkan_pipeline_stats_query_pool, 0, GpuPipelineStatistics::Count);

		vkCmdBeginQuery(gpu_commands->vk_command_buffer, thread_pools->vulkan_pipeline_stats_query_pool, 0, 0);

		gpu->vkCmdBuildAccelerationStructuresKHR(gpu_commands->vk_command_buffer, 1, &as_info, tlas_ranges);

		gpu->submit_immediate(gpu_commands);

		scene->geometries.shutdown();
		scene->build_range_infos.shutdown();

		gpu->destroy_buffer(blas_scratch_buffer_handle);
		gpu->destroy_buffer(tlas_scratch_buffer_handle);
		gpu->destroy_buffer(tlas_instance_buffer_handle);

		max_primitives_count.shutdown();
	}
	

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

	// Destroy resources built here.
	gpu->destroy_buffer(scene->blas_buffer);
	gpu->vkDestroyAccelerationStructureKHR(gpu->vulkan_device, scene->blas, gpu->vulkan_allocation_callbacks);
	gpu->destroy_buffer(scene->tlas_buffer);
	gpu->vkDestroyAccelerationStructureKHR(gpu->vulkan_device, scene->tlas, gpu->vulkan_allocation_callbacks);
	gpu->destroy_sampler(repeat_nearest_sampler);
	gpu->destroy_sampler(repeat_sampler);

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
			scene->on_resize(*gpu, &s_frame_graph, window->GetWidth(), window->GetHeight());

			s_frame_renderer.update_dependent_resources();

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
		static vec2s last_clicked_position = vec2s{ 1280 / 2.0f, 800 / 2.0f };

		static vec3s raytraced_shadow_light_direction = vec3s{ 0, 1, -0.2f };
		static vec3s raytraced_shadow_light_position = vec3s{ 0, 1, 0 };
		static float raytraced_shadow_light_intensity = 5.0f;
		static i32 raytraced_shadow_light_type = 0;
		static f32 raytraced_shadow_light_radius = 10.f;
		static vec3s raytraced_shadow_light_color = vec3s{ 1, 1, 1 };



		// Jittering update
		static u32 jitter_index = 0;
		static JitterType::Enum jitter_type = JitterType::Halton;
		static u32 jitter_period = 2;
		vec2s jitter_values = vec2s{ 0.0f, 0.0f };

		switch (jitter_type) {
		case JitterType::Halton:
			jitter_values = halton23_sequence(jitter_index);
			break;

		case JitterType::R2:
			jitter_values = m_robert_r2_sequence(jitter_index);
			break;

		case JitterType::InterleavedGradients:
			jitter_values = interleaved_gradient_sequence(jitter_index);
			break;

		case JitterType::Hammersley:
			jitter_values = hammersley_sequence(jitter_index, jitter_period);
			break;
		}
		jitter_index = (jitter_index + 1) % jitter_period;

		vec2s jitter_offsets = vec2s{ jitter_values.x * 2 - 1.0f, jitter_values.y * 2 - 1.0f };
		static f32 jitter_scale = 1.f;

		jitter_offsets.x *= jitter_scale;
		jitter_offsets.y *= jitter_scale;

		// Update also projection matrix of the camera.
		if (scene->taa_enabled && scene->taa_jittering_enabled) {
			s_game_camera.apply_jittering(jitter_offsets.x / gpu->swapchain_width, jitter_offsets.y / gpu->swapchain_height);
		}
		else {
			s_game_camera.camera.set_zoom(1.0f);
			s_game_camera.camera.update();
		}

		// -- ImGui New Frame -------------- //
		imgui->BeginFrame();
		

		// Draw GUI
		{
            MF_PROFILE_SCOPE("Draw GUI");

			if (ImGui::Begin("Magnefu ImGui"))
			{
                if(ImGui::CollapsingHeader("MODELS"))
				{
					ImGui::InputFloat("Scene Global Scale scale", &scene->global_scale, 0.001f);
				}

                if(ImGui::CollapsingHeader("CAMERA"))
				{
					ImGui::InputFloat3("Camera position", s_game_camera.camera.position.raw);
					ImGui::InputFloat3("Camera target movement", s_game_camera.target_movement.raw);
				}

				ImGui::SliderFloat("Force Roughness", &scene->forced_roughness, -1, 1);
				ImGui::SliderFloat("Force Metalness", &scene->forced_metalness, -1, 1);

				if(ImGui::CollapsingHeader("PHYSICS"))
				{
					ImGui::InputFloat3("Wind direction", render_data.wind_direction.raw);
					ImGui::InputFloat("Air density", &render_data.air_density);
					ImGui::InputFloat("Spring stiffness", &render_data.spring_stiffness);
					ImGui::InputFloat("Spring damping", &render_data.spring_damping);
					ImGui::Checkbox("Reset simulation", &render_data.reset_simulation);
				}

				if(ImGui::CollapsingHeader("DEBUG AND TESTING"))
				{
					ImGui::Checkbox("Enable AABB cubemap test", &enable_aabb_cubemap_test);
					ImGui::Checkbox("Enable light cluster debug", &enable_light_cluster_debug);
					ImGui::Checkbox("Enable light tile debug", &enable_light_tile_debug);
					ImGui::SliderFloat3("AABB test position", aabb_test_position.raw, -1.5f, 1.5f, "%1.2f");
				}
				
				if(ImGui::CollapsingHeader("LIGHTING"))
				{
					ImGui::SliderUint("Active Lights", &scene->active_lights, 1, k_num_lights - 1);
					ImGui::SliderUint("Light Index", &light_to_debug, 0, scene->active_lights - 1);

					Light& selected_light = scene->lights[light_to_debug];
					ImGui::SliderFloat3("Light position", selected_light.world_position.raw, -10.f, 10.f, "%2.3f");
					ImGui::SliderFloat("Light radius", &selected_light.radius, 0.01f, 10.f, "%2.3f");
					ImGui::SliderFloat("Light intensity", &selected_light.intensity, 0.01f, 10.f, "%2.3f");

					f32 light_color[3] = { selected_light.color.x, selected_light.color.y, selected_light.color.z };
					ImGui::ColorEdit3("Light color", light_color);
					selected_light.color = { light_color[0], light_color[1], light_color[2] };
					ImGui::Checkbox("Light Edit Debug Draws", &scene->show_light_edit_debug_draws);
				}

				if (ImGui::CollapsingHeader("Meshlets")) {
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
				}
				if (ImGui::CollapsingHeader("Clustered Lighting")) {

					ImGui::Checkbox("Enable Camera Inside approximation", &enable_camera_inside);
					ImGui::Checkbox("Use McGuire method for AABB sphere", &use_mcguire_method);
					ImGui::Checkbox("Skip invisible lights", &skip_invisible_lights);
					ImGui::Checkbox("use view aabb", &use_view_aabb);
					ImGui::Checkbox("force fullscreen light aabb", &force_fullscreen_light_aabb);
					ImGui::Checkbox("debug show light tiles", &debug_show_light_tiles);
					ImGui::Checkbox("debug show tiles", &debug_show_tiles);
					ImGui::Checkbox("debug show bins", &debug_show_bins);
					ImGui::SliderUint("Lighting debug modes", &lighting_debug_modes, 0, 10);
				}
				if (ImGui::CollapsingHeader("PointLight Shadows")) {
					ImGui::Checkbox("Pointlight rendering", &scene->pointlight_rendering);
					ImGui::Checkbox("Pointlight rendering use meshlets", &scene->pointlight_use_meshlets);
					ImGui::Checkbox("Disable shadows", &disable_shadows);
					ImGui::Checkbox("Use tetrahedron shadows", &scene->use_tetrahedron_shadows);
					ImGui::Checkbox("Cubeface switch Pos X", &scene->cubeface_flip[0]);
					ImGui::Checkbox("Cubeface switch Neg X", &scene->cubeface_flip[1]);
					ImGui::Checkbox("Cubeface switch Pos Y", &scene->cubeface_flip[2]);
					ImGui::Checkbox("Cubeface switch Neg Y", &scene->cubeface_flip[3]);
					ImGui::Checkbox("Cubeface switch Pos Z", &scene->cubeface_flip[4]);
					ImGui::Checkbox("Cubeface switch Neg Z", &scene->cubeface_flip[5]);
				}
				if (ImGui::CollapsingHeader("Volumetric Fog")) {
					ImGui::SliderFloat("Fog Constant Density", &scene->volumetric_fog_density, 0.0f, 1.0f);
					ImGui::SliderFloat("Fog Scattering Factor", &scene->volumetric_fog_scattering_factor, 0.0f, 1.0f);
					ImGui::SliderFloat("Height Fog Density", &scene->volumetric_fog_height_fog_density, 0.0f, 10.0f);
					ImGui::SliderFloat("Height Fog Falloff", &scene->volumetric_fog_height_fog_falloff, 0.0f, 10.0f);
					ImGui::SliderUint("Phase Function Type", &scene->volumetric_fog_phase_function_type, 0, 3);
					ImGui::SliderFloat("Phase Anisotropy", &scene->volumetric_fog_phase_anisotropy_01, 0.0f, 1.0f);
					ImGui::SliderFloat("Fog Noise Scale", &scene->volumetric_fog_noise_scale, 0.0f, 1.0f);
					ImGui::SliderFloat("Lighting Noise Scale", &scene->volumetric_fog_lighting_noise_scale, 0.0f, 1.0f);
					ImGui::SliderUint("Fog Noise Type", &scene->volumetric_fog_noise_type, 0, 2);
					ImGui::SliderFloat("Temporal Reprojection Percentage", &scene->volumetric_fog_temporal_reprojection_percentage, 0.0f, 1.0f);
					ImGui::SliderFloat("Temporal Reprojection Jittering Scale", &scene->volumetric_fog_temporal_reprojection_jittering_scale, 0.0f, 10.0f);
					ImGui::Checkbox("Use Temporal Reprojection", &scene->volumetric_fog_use_temporal_reprojection);
					ImGui::Checkbox("Use Spatial Filtering", &scene->volumetric_fog_use_spatial_filtering);
					ImGui::SliderFloat("Fog Application Scale", &scene->volumetric_fog_application_dithering_scale, 0.0f, 1.0f);
					ImGui::Checkbox("Fog Application Opacity AA", &scene->volumetric_fog_application_apply_opacity_anti_aliasing);
					ImGui::Checkbox("Fog Application Tricubic", &scene->volumetric_fog_application_apply_tricubic_filtering);
					ImGui::SliderFloat("Fog Volumetric Noise Position Scale", &scene->volumetric_fog_noise_position_scale, 0.0f, 1.0f);
					ImGui::SliderFloat("Fog Volumetric Noise Speed Scale", &scene->volumetric_fog_noise_speed_scale, 0.0f, 1.0f);
					ImGui::SliderFloat3("Box position", scene->volumetric_fog_box_position.raw, -10.f, 10.f, "%2.3f");
					ImGui::SliderFloat3("Box size", scene->volumetric_fog_box_size.raw, -4.f, 4.f, "%1.3f");
					ImGui::SliderFloat("Box density", &scene->volumetric_fog_box_density, 0.0f, 10.0f);

					Color box_color = { scene->volumetric_fog_box_color };
					f32 box_color_floats[3] = { box_color.r(), box_color.g(), box_color.b() };
					if (ImGui::ColorEdit3("Box color", box_color_floats)) {

						box_color.set(box_color_floats[0], box_color_floats[1], box_color_floats[2], 1.0f);

						scene->volumetric_fog_box_color = box_color.abgr;
					}
				}
				if (ImGui::CollapsingHeader("Temporal Anti-Aliasing")) {
					ImGui::Checkbox("Enable", &scene->taa_enabled);
					ImGui::Checkbox("Jittering Enable", &scene->taa_jittering_enabled);

					static i32 current_jitter_type = (i32)jitter_type;
					ImGui::Combo("Jitter Type", &current_jitter_type, JitterType::names, ArraySize(JitterType::names));
					jitter_type = (JitterType::Enum)current_jitter_type;

					ImGui::SliderUint("Jittering Period", &jitter_period, 1, 16);
					ImGui::SliderFloat("Jitter Scale", &jitter_scale, 0.0f, 4.0f);

					static cstring taa_mode_names[] = { "OnlyReprojection", "Full" };
					ImGui::Combo("Modes", &scene->taa_mode, taa_mode_names, ArraySize(taa_mode_names));

					static cstring taa_velocity_mode_names[] = { "None", "3x3 Neighborhood" };
					ImGui::Combo("Velocity sampling modes", &scene->taa_velocity_sampling_mode, taa_velocity_mode_names, ArraySize(taa_velocity_mode_names));

					static cstring taa_history_sampling_names[] = { "None", "CatmullRom" };
					ImGui::Combo("History sampling filter", &scene->taa_history_sampling_filter, taa_history_sampling_names, ArraySize(taa_history_sampling_names));

					static cstring taa_history_constraint_names[] = { "None", "Clamp", "Clip", "Variance Clip", "Variance Clip with Color Clamping" };
					ImGui::Combo("History constraint mode", &scene->taa_history_constraint_mode, taa_history_constraint_names, ArraySize(taa_history_constraint_names));

					static cstring taa_current_color_filter_names[] = { "None", "Mitchell-Netravali", "Blackman-Harris", "Catmull-Rom" };
					ImGui::Combo("Current color filter", &scene->taa_current_color_filter, taa_current_color_filter_names, ArraySize(taa_current_color_filter_names));

					ImGui::Checkbox("Inverse Luminance Filtering", &scene->taa_use_inverse_luminance_filtering);
					ImGui::Checkbox("Temporal Filtering", &scene->taa_use_temporal_filtering);
					ImGui::Checkbox("Luminance Difference Filtering", &scene->taa_use_luminance_difference_filtering);
					ImGui::Checkbox("Use YCoCg color space", &scene->taa_use_ycocg);
				}
				if (ImGui::CollapsingHeader("Post-Process")) {
					static cstring tonemap_names[] = { "None", "ACES" };
					ImGui::Combo("Tonemap", &scene->post_tonemap_mode, tonemap_names, ArraySize(tonemap_names));
					ImGui::SliderFloat("Exposure", &scene->post_exposure, -4.0f, 4.0f);
					ImGui::SliderFloat("Sharpening amount", &scene->post_sharpening_amount, 0.0f, 4.0f);
					ImGui::Checkbox("Enable Magnifying Zoom", &scene->post_enable_zoom);
					ImGui::Checkbox("Block Magnifying Zoom Input", &scene->post_block_zoom_input);
					ImGui::SliderUint("Magnifying Zoom Scale", &scene->post_zoom_scale, 2, 4);
				}
				if (ImGui::CollapsingHeader("Raytraced Shadows")) {
					static cstring light_type[] = { "Point", "Directional" };
					ImGui::Combo("RT Light Type", &raytraced_shadow_light_type, light_type, ArraySize(light_type));

					ImGui::SliderFloat("RT Light intensity", &raytraced_shadow_light_intensity, 0.01f, 10.f, "%2.2f");
					ImGui::ColorEdit3("RT Light Color", raytraced_shadow_light_color.raw);

					// If directional light, disable light position and light radius controls
					if (raytraced_shadow_light_type == 1) {
						ImGui::BeginDisabled();
					}
					ImGui::SliderFloat("RT Light Radius", &raytraced_shadow_light_radius, 0.01f, 10.f);
					ImGui::SliderFloat3("RT Light Position", raytraced_shadow_light_position.raw, -10.f, 10.f, "%2.2f");
					if (raytraced_shadow_light_type == 1) {
						ImGui::EndDisabled();
					}

					// If type is a pointlight, disable the light direction
					if (raytraced_shadow_light_type == 0) {
						ImGui::BeginDisabled();
					}
					ImGui::SliderFloat3("RT Directional Direction", raytraced_shadow_light_direction.raw, -1.f, 1.f, "%2.2f");
					if (raytraced_shadow_light_type == 0) {
						ImGui::EndDisabled();
					}

				}
				if (ImGui::CollapsingHeader("Global Illumination")) {

					ImGui::Text("Total Rays: %u, Rays per probe %u, Total Probes %u", s_frame_renderer.indirect_pass.get_total_rays(), s_frame_renderer.indirect_pass.probe_rays, s_frame_renderer.indirect_pass.get_total_probes());
					ImGui::SliderInt("Per frame probe updates", &scene->gi_per_frame_probes_update, 0, s_frame_renderer.indirect_pass.get_total_probes());
					// Check if probe offsets needs to be recalculated.
					scene->gi_recalculate_offsets = false;

					ImGui::SliderFloat("Indirect Intensity", &scene->gi_intensity, 0.0f, 1.0f);
					if (ImGui::SliderFloat3("Probe Grid Position", scene->gi_probe_grid_position.raw, -5.f, 5.f, "%2.3f")) {
						scene->gi_recalculate_offsets = true;
					}

					ImGui::Checkbox("Use Infinite Bounces", &scene->gi_use_infinite_bounces);
					ImGui::SliderFloat("Infinite bounces multiplier", &scene->gi_infinite_bounces_multiplier, 0.0f, 1.0f);

					if (ImGui::SliderFloat3("Probe Spacing", scene->gi_probe_spacing.raw, -2.f, 2.f, "%2.3f")) {
						scene->gi_recalculate_offsets = true;
					}

					ImGui::SliderFloat("Hysteresis", &scene->gi_hysteresis, 0.0f, 1.0f);
					ImGui::SliderFloat("Max Probe Offset", &scene->gi_max_probe_offset, 0.0f, 0.5f);
					ImGui::SliderFloat("Sampling self shadow bias", &scene->gi_self_shadow_bias, 0.0f, 1.0f);
					ImGui::SliderFloat("Probe Sphere Scale", &scene->gi_probe_sphere_scale, 0.0f, 1.0f);
					ImGui::Checkbox("Show debug probes", &scene->gi_show_probes);
					ImGui::Checkbox("Use Visibility", &scene->gi_use_visibility);
					ImGui::Checkbox("Use Smooth Backface", &scene->gi_use_backface_smoothing);
					ImGui::Checkbox("Use Perceptual Encoding", &scene->gi_use_perceptual_encoding);
					ImGui::Checkbox("Use Backface Blending", &scene->gi_use_backface_blending);
					ImGui::Checkbox("Use Probe Offsetting", &scene->gi_use_probe_offsetting);
					ImGui::Checkbox("Use Probe Status", &scene->gi_use_probe_status);
					if (ImGui::Checkbox("Use Half Resolution Output", &scene->gi_use_half_resolution)) {
						s_frame_renderer.indirect_pass.half_resolution_output = scene->gi_use_half_resolution;
						s_frame_renderer.indirect_pass.on_resize(*gpu, &s_frame_graph, gpu->swapchain_width, gpu->swapchain_height);
					}
					ImGui::Checkbox("Debug border vs inside", &scene->gi_debug_border);
					ImGui::Checkbox("Debug border type (corner, row, column)", &scene->gi_debug_border_type);
					ImGui::Checkbox("Debug border source pixels", &scene->gi_debug_border_source);
				}
				ImGui::Separator();

				ImGui::Checkbox("Show Debug GPU Draws", &scene->show_debug_gpu_draws);
				ImGui::Checkbox("Dynamically recreate descriptor sets", &recreate_per_thread_descriptors);
				ImGui::Checkbox("Use secondary command buffers", &use_secondary_command_buffers);
				ImGui::Separator();
				ImGui::SliderFloat("Animation Speed Multiplier", &animation_speed_multiplier, 0.0f, 10.0f);
				ImGui::Separator();

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

			/*if (ImGui::Begin("Scene")) {

				static u32 selected_node = u32_max;
				static u32 meshlet_count = u32_max;

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
					
					ImGui::Text("Meshlet Count: %u", meshlet_count);

					ImGui::Separator();
				}

				for (u32 n = 0; n < s_scene_graph.nodes_hierarchy.size; ++n) {
					const SceneGraphNodeDebugData& node_debug_data = s_scene_graph.nodes_debug_data[n];
					if (ImGui::Selectable(node_debug_data.name ? node_debug_data.name : "-", n == selected_node)) {
						selected_node = n;
						if(scene->mesh_instances.count() > node_debug_data.mesh_index)
							meshlet_count = scene->mesh_instances[node_debug_data.mesh_index].mesh->meshlet_count;
					}
				}
			}
			ImGui::End();*/

			if (ImGui::Begin("Scene")) {
				static u32 selected_node = u32_max;
				static u32 meshlet_count = u32_max;

				if (selected_node < s_scene_graph.nodes_hierarchy.size) {
					mat4s& local_transform = s_scene_graph.local_matrices[selected_node];
					f32 position[3] = { local_transform.m30, local_transform.m31, local_transform.m32 };

					if (ImGui::SliderFloat3("Node Position", position, -100.0f, 100.0f)) {
						local_transform.m30 = position[0];
						local_transform.m31 = position[1];
						local_transform.m32 = position[2];
						s_scene_graph.set_local_matrix(selected_node, local_transform);
					}
					ImGui::Separator();
					ImGui::Text("Meshlet Count: %u", meshlet_count);
					ImGui::Separator();
				}

				// Iterate over root nodes and draw the full hierarchy
				for (u32 n = 0; n < s_scene_graph.nodes_hierarchy.size; ++n) {
					if (s_scene_graph.nodes_hierarchy[n].parent == -1) {  // Assuming u32_max indicates no parent
						DrawNode(n, selected_node, meshlet_count, s_scene_graph, scene);
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

			if (ImGui::Begin("Frame Graph Debug")) {

				s_frame_graph.debug_ui();

				static u32 texture_to_debug = 116;
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
		render_data.jitter_offsets = jitter_offsets;
		render_data.last_clicked_position = last_clicked_position;

		render_data.raytraced_shadow_light_color = raytraced_shadow_light_color;
		render_data.raytraced_shadow_light_direction = raytraced_shadow_light_direction;
		render_data.raytraced_shadow_light_position = raytraced_shadow_light_position;
		render_data.raytraced_shadow_light_intensity = raytraced_shadow_light_intensity;
		render_data.raytraced_shadow_light_type = raytraced_shadow_light_type;
		render_data.raytraced_shadow_light_radius = raytraced_shadow_light_radius;


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
			scene_data.halton_x = render_data->jitter_offsets.x;
			scene_data.halton_y = render_data->jitter_offsets.y;

			// Cache previous view projection
			scene_data.previous_view_projection = scene_data.view_projection;
			// Frame 0 jittering or disable jittering as option.
			if (gpu->absolute_frame == 0 || (scene->taa_jittering_enabled == false)) {
				scene_data.jitter_xy = vec2s{ 0.0f, 0.0f };
			}
			// Cache previous jitter and calculate new one
			scene_data.previous_jitter_xy = scene_data.jitter_xy;

			if (scene->taa_jittering_enabled && scene->taa_enabled) {
				scene_data.jitter_xy = vec2s{ (scene_data.halton_x) / gpu->swapchain_width, (scene_data.halton_y) / gpu->swapchain_height };
			}

			scene_data.view_projection = s_game_camera.camera.view_projection;
			scene_data.inverse_view_projection = glms_mat4_inv(s_game_camera.camera.view_projection);
			scene_data.inverse_projection = glms_mat4_inv(s_game_camera.camera.projection);
			scene_data.inverse_view = glms_mat4_inv(s_game_camera.camera.view);
			scene_data.world_to_camera = s_game_camera.camera.view;
			scene_data.camera_position = vec4s{ s_game_camera.camera.position.x, s_game_camera.camera.position.y, s_game_camera.camera.position.z, 1.0f };
			scene_data.camera_direction = s_game_camera.camera.direction;
			scene_data.dither_texture_index = dither_texture ? dither_texture->handle.index : 0;
			scene_data.current_frame = (u32)gpu->absolute_frame;
			scene_data.forced_metalness = scene->forced_metalness;
			scene_data.forced_roughness = scene->forced_roughness;

			FrameGraphResource* depth_resource = (FrameGraphResource*)s_frame_graph.get_resource("depth");
			if (depth_resource) {
				scene_data.depth_texture_index = depth_resource->resource_info.texture.handle.index;
			}

			scene_data.blue_noise_128_rg_texture_index = blue_noise_128_rg_texture->handle.index;

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
			scene_data.volumetric_fog_application_dithering_scale = scene->volumetric_fog_application_dithering_scale;
			scene_data.volumetric_fog_application_options = (scene->volumetric_fog_application_apply_opacity_anti_aliasing ? 1 : 0)
				| (scene->volumetric_fog_application_apply_tricubic_filtering ? 2 : 0);


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
				gpu_lighting_data->gi_intensity = scene->gi_intensity;

				FrameGraphResource* resource = s_frame_graph.get_resource("shadow_visibility");
				if (resource) {
					gpu_lighting_data->shadow_visibility_texture_index = resource->resource_info.texture.handle.index;
				}

				resource = (FrameGraphResource*)s_frame_graph.get_resource("indirect_lighting");
				if (resource) {
					gpu_lighting_data->indirect_lighting_texture_index = resource->resource_info.texture.handle.index;
				}

				resource = (FrameGraphResource*)s_frame_graph.get_resource("bilateral_weights");
				if (resource) {
					gpu_lighting_data->bilateral_weights_texture_index = resource->resource_info.texture.handle.index;
				}

				resource = (FrameGraphResource*)s_frame_graph.get_resource("svgf_output");
				if (resource) {
					gpu_lighting_data->reflections_texture_index = resource->resource_info.texture.handle.index;
				}

				// Volumetric fog data
				// TODO: parametrize it
				gpu_lighting_data->volumetric_fog_texture_index = scene->volumetric_fog_texture_index;
				gpu_lighting_data->volumetric_fog_num_slices = scene->volumetric_fog_slices;
				gpu_lighting_data->volumetric_fog_near = s_game_camera.camera.near_plane;
				gpu_lighting_data->volumetric_fog_far = s_game_camera.camera.far_plane;

				// linear_depth_to_uv_optimize offloads this calculations here:
				const float one_over_log_f_over_n = 1.0f / log2(s_game_camera.camera.far_plane / s_game_camera.camera.near_plane);
				gpu_lighting_data->volumetric_fog_distribution_scale = scene->volumetric_fog_slices * one_over_log_f_over_n;
				gpu_lighting_data->volumetric_fog_distribution_bias = -(scene->volumetric_fog_slices * log2(s_game_camera.camera.near_plane) * one_over_log_f_over_n);

				Color raytraced_light_color_type_packed;
				raytraced_light_color_type_packed.set(render_data->raytraced_shadow_light_color.x, render_data->raytraced_shadow_light_color.y, render_data->raytraced_shadow_light_color.z, render_data->raytraced_shadow_light_type);
				gpu_lighting_data->raytraced_shadow_light_color_type = raytraced_light_color_type_packed.abgr;
				gpu_lighting_data->raytraced_shadow_light_radius = render_data->raytraced_shadow_light_radius;
				gpu_lighting_data->raytraced_shadow_light_position = render_data->raytraced_shadow_light_type == 0 ? render_data->raytraced_shadow_light_position : render_data->raytraced_shadow_light_direction;
				gpu_lighting_data->raytraced_shadow_light_intensity = render_data->raytraced_shadow_light_intensity;

				gpu->unmap_buffer(cb_map);
			}

			// TODO: move light placement here.
			if (render_data->light_placement) 
			{
				render_data->light_placement = false;

				//place_lights( scene->lights, true );
			}

			// Update mouse clicked position
			if ((input->IsMouseClicked(MOUSE_BUTTON_LEFT) || input->IsMouseDragging(MOUSE_BUTTON_LEFT)) && !ImGui::IsAnyItemHovered()) {
				render_data->last_clicked_position = vec2s{ input->mouse_position.x, input->mouse_position.y };
			}
			


			UploadGpuDataContext upload_context{ s_game_camera, &MemoryService::Instance()->tempStackAllocator };
			upload_context.enable_camera_inside = render_data->enable_camera_inside;
			upload_context.force_fullscreen_light_aabb = render_data->force_fullscreen_light_aabb;
			upload_context.skip_invisible_lights = render_data->skip_invisible_lights;
			upload_context.use_mcguire_method = render_data->use_mcguire_method;
			upload_context.use_view_aabb = render_data->use_view_aabb;
			upload_context.last_clicked_position_left_button = render_data->last_clicked_position;
			s_frame_renderer.upload_gpu_data(upload_context);



			// Place light AABB with a smaller aabb to indicate the center.
			
			if (scene->show_light_edit_debug_draws) {
				const Light& light = scene->lights[render_data->light_to_debug];
				f32 half_radius = light.radius;
				scene->debug_renderer.aabb(glms_vec3_sub(light.world_position, { half_radius, half_radius ,half_radius }), glms_vec3_add(light.world_position, { half_radius, half_radius , half_radius }), { Color::white });
				scene->debug_renderer.aabb(glms_vec3_sub(light.world_position, { .1, .1, .1 }), glms_vec3_add(light.world_position, { .1, .1, .1 }), { Color::green });
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