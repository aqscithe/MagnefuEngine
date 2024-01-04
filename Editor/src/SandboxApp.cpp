#include "SandboxApp.hpp"

// -- App ---------------- //
#include "AppLayers/SandboxLayer.hpp"
#include "AppLayers/Overlay.hpp"
#include "GameCamera.hpp"


// -- Graphics --------------------------- //


// -- Core -------------------------- //
#include "Magnefu/Core/File.hpp"
#include "Magnefu/Core/glTF.hpp"
//#include "Magnefu/Core/String.hpp"

#include "imgui/imgui.h"

#include "cglm/struct/mat3.h"
#include "cglm/struct/mat4.h"
#include "cglm/struct/quat.h"
#include "cglm/struct/cam.h"
#include "cglm/struct/affine.h"



// -- Sandbox App -------------------------------------------------------------------------- //

#if defined(MF_PLATFORM_WINDOWS)

static Magnefu::WindowsWindow s_window;

#elif defined(MF_PLATFORM_LINUX)
// TODO: Setup an application window implementation for OS other than Microsoft Windows
static Magnefu::LinuxWindow s_window;

#elif defined(MF_PLATFORM_MAC)
static Magnefu::MacWindow s_window;

#endif

static Magnefu::ResourceManager s_rm;
static Magnefu::GPUProfiler s_gpu_profiler;


// ----------------------------------------------------------------------------------------------------------- //


Magnefu::BufferHandle                    scene_cb;

struct MeshDraw
{
	Magnefu::Material* material;

	Magnefu::BufferHandle    index_buffer;
	Magnefu::BufferHandle    position_buffer;
	Magnefu::BufferHandle    tangent_buffer;
	Magnefu::BufferHandle    normal_buffer;
	Magnefu::BufferHandle    texcoord_buffer;
	Magnefu::BufferHandle    material_buffer;

	u32         index_offset;
	u32         position_offset;
	u32         tangent_offset;
	u32         normal_offset;
	u32         texcoord_offset;

	u32         primitive_count;

	// Indices used for bindless textures.
	u16         diffuse_texture_index;
	u16         roughness_texture_index;
	u16         normal_texture_index;
	u16         occlusion_texture_index;

	vec4s       base_color_factor;
	vec4s       metallic_roughness_occlusion_factor;
	vec3s       scale;

	f32         alpha_cutoff;
	u32         flags;
}; // struct MeshDraw

//
//
enum DrawFlags {
	DrawFlags_AlphaMask = 1 << 0,
}; // enum DrawFlags

//
//
struct UniformData {
	mat4s       vp;
	vec4s       eye;
	vec4s       light;
	float       light_range;
	float       light_intensity;
}; // struct UniformData

//
//
struct MeshData 
{
	mat4s       m;
	mat4s       inverseM;

	u32         textures[4]; // diffuse, roughness, normal, occlusion
	vec4s       base_color_factor;
	vec4s       metallic_roughness_occlusion_factor; // metallic, roughness, occlusion
	float       alpha_cutoff;
	float       padding_[3];
	u32         flags;
}; // struct MeshData

//
//
struct GpuEffect
{

	Magnefu::PipelineHandle          pipeline_cull;
	Magnefu::PipelineHandle          pipeline_no_cull;

}; // struct GpuEffect

// TODO: Will eventually create a full on class for ECS entt data
// When creating scene manager take inspiration from the service
// manager implementation...
struct Scene 
{

	Magnefu::Array<MeshDraw>                 mesh_draws;

	// All graphics resources used by the scene
	Magnefu::Array<Magnefu::TextureResource>  images;
	Magnefu::Array<Magnefu::SamplerResource>  samplers;
	Magnefu::Array<Magnefu::BufferResource>   buffers;

	Magnefu::glTF::glTF                      gltf_scene; // Source gltf scene

}; // struct GltfScene

static const u16 INVALID_TEXTURE_INDEX = ~0u;
static Scene* scene = nullptr;
static GameCamera game_camera;




//
//
static void upload_material(MeshData& mesh_data, const MeshDraw& mesh_draw, const f32 global_scale)
{
	mesh_data.textures[0] = mesh_draw.diffuse_texture_index;
	mesh_data.textures[1] = mesh_draw.roughness_texture_index;
	mesh_data.textures[2] = mesh_draw.normal_texture_index;
	mesh_data.textures[3] = mesh_draw.occlusion_texture_index;
	mesh_data.base_color_factor = mesh_draw.base_color_factor;
	mesh_data.metallic_roughness_occlusion_factor = mesh_draw.metallic_roughness_occlusion_factor;
	mesh_data.alpha_cutoff = mesh_draw.alpha_cutoff;
	mesh_data.flags = mesh_draw.flags;

	// NOTE: for left-handed systems (as defined in cglm) need to invert positive and negative Z.
	mat4s model = glms_scale_make(glms_vec3_mul(mesh_draw.scale, { global_scale, global_scale, -global_scale }));
	mesh_data.m = model;
	mesh_data.inverseM = glms_mat4_inv(glms_mat4_transpose(model));
}

//
//
static void draw_mesh(Magnefu::Renderer& renderer, Magnefu::CommandBuffer* gpu_commands, MeshDraw& mesh_draw)
{
	// Descriptor Set
	Magnefu::DescriptorSetCreation ds_creation{};
	ds_creation.buffer(scene_cb, 0).buffer(mesh_draw.material_buffer, 1);
	Magnefu::DescriptorSetHandle descriptor_set = renderer.create_descriptor_set(gpu_commands, mesh_draw.material, ds_creation);

	gpu_commands->bind_vertex_buffer(mesh_draw.position_buffer, 0, mesh_draw.position_offset);
	gpu_commands->bind_vertex_buffer(mesh_draw.tangent_buffer, 1, mesh_draw.tangent_offset);
	gpu_commands->bind_vertex_buffer(mesh_draw.normal_buffer, 2, mesh_draw.normal_offset);
	gpu_commands->bind_vertex_buffer(mesh_draw.texcoord_buffer, 3, mesh_draw.texcoord_offset);
	gpu_commands->bind_index_buffer(mesh_draw.index_buffer, mesh_draw.index_offset);
	gpu_commands->bind_local_descriptor_set(&descriptor_set, 1, nullptr, 0);

	gpu_commands->draw_indexed(Magnefu::TopologyType::Triangle, mesh_draw.primitive_count, 1, 0, 0, 0);
}

//
//

static void scene_load_from_gltf(cstring filename, Magnefu::Renderer& renderer, Magnefu::Allocator* allocator, Scene& scene) {

	using namespace Magnefu;

	scene.gltf_scene = gltf_load_file(filename);

	// Load all textures
	scene.images.init(allocator, scene.gltf_scene.images_count);

	for (u32 image_index = 0; image_index < scene.gltf_scene.images_count; ++image_index) {
		glTF::Image& image = scene.gltf_scene.images[image_index];
		TextureResource* tr = renderer.create_texture(image.uri.data, image.uri.data, true);
		MF_CORE_ASSERT((tr != nullptr), "Texture doesn't exist");

		scene.images.push(*tr);
	}

	StringBuffer resource_name_buffer;
	resource_name_buffer.init(4096, allocator);

	// Load all samplers
	scene.samplers.init(allocator, scene.gltf_scene.samplers_count);

	for (u32 sampler_index = 0; sampler_index < scene.gltf_scene.samplers_count; ++sampler_index) {
		glTF::Sampler& sampler = scene.gltf_scene.samplers[sampler_index];

		char* sampler_name = resource_name_buffer.append_use_f("sampler_%u", sampler_index);

		SamplerCreation creation;
		switch (sampler.min_filter) {
		case glTF::Sampler::NEAREST:
			creation.min_filter = VK_FILTER_NEAREST;
			break;
		case glTF::Sampler::LINEAR:
			creation.min_filter = VK_FILTER_LINEAR;
			break;
		case glTF::Sampler::LINEAR_MIPMAP_NEAREST:
			creation.min_filter = VK_FILTER_LINEAR;
			creation.mip_filter = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case glTF::Sampler::LINEAR_MIPMAP_LINEAR:
			creation.min_filter = VK_FILTER_LINEAR;
			creation.mip_filter = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case glTF::Sampler::NEAREST_MIPMAP_NEAREST:
			creation.min_filter = VK_FILTER_NEAREST;
			creation.mip_filter = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case glTF::Sampler::NEAREST_MIPMAP_LINEAR:
			creation.min_filter = VK_FILTER_NEAREST;
			creation.mip_filter = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		}

		creation.mag_filter = sampler.mag_filter == glTF::Sampler::Filter::LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;

		switch (sampler.wrap_s) {
		case glTF::Sampler::CLAMP_TO_EDGE:
			creation.address_mode_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			break;
		case glTF::Sampler::MIRRORED_REPEAT:
			creation.address_mode_u = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			break;
		case glTF::Sampler::REPEAT:
			creation.address_mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			break;
		}

		switch (sampler.wrap_t) {
		case glTF::Sampler::CLAMP_TO_EDGE:
			creation.address_mode_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			break;
		case glTF::Sampler::MIRRORED_REPEAT:
			creation.address_mode_v = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			break;
		case glTF::Sampler::REPEAT:
			creation.address_mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			break;
		}

		creation.name = sampler_name;

		SamplerResource* sr = renderer.create_sampler(creation);
		MF_CORE_ASSERT((sr != nullptr), "");

		scene.samplers.push(*sr);
	}

	// Temporary array of buffer data
	Magnefu::Array<void*> buffers_data;
	buffers_data.init(allocator, scene.gltf_scene.buffers_count);

	for (u32 buffer_index = 0; buffer_index < scene.gltf_scene.buffers_count; ++buffer_index) {
		glTF::Buffer& buffer = scene.gltf_scene.buffers[buffer_index];

		FileReadResult buffer_data = file_read_binary(buffer.uri.data, allocator);
		buffers_data.push(buffer_data.data);
	}

	// Load all buffers and initialize them with buffer data
	scene.buffers.init(allocator, scene.gltf_scene.buffer_views_count);

	for (u32 buffer_index = 0; buffer_index < scene.gltf_scene.buffer_views_count; ++buffer_index) {
		glTF::BufferView& buffer = scene.gltf_scene.buffer_views[buffer_index];

		i32 offset = buffer.byte_offset;
		if (offset == glTF::INVALID_INT_VALUE) {
			offset = 0;
		}

		u8* data = (u8*)buffers_data[buffer.buffer] + offset;

		// NOTE(marco): the target attribute of a BufferView is not mandatory, so we prepare for both uses
		VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		char* buffer_name = buffer.name.data;
		if (buffer_name == nullptr) {
			buffer_name = resource_name_buffer.append_use_f("buffer_%u", buffer_index);
		}

		BufferResource* br = renderer.create_buffer(flags, ResourceUsageType::Immutable, buffer.byte_length, data, buffer_name);
		MF_CORE_ASSERT((br != nullptr), "");

		scene.buffers.push(*br);
	}

	for (u32 buffer_index = 0; buffer_index < scene.gltf_scene.buffers_count; ++buffer_index) {
		void* buffer = buffers_data[buffer_index];
		allocator->deallocate(buffer);
	}
	buffers_data.shutdown();

	resource_name_buffer.shutdown();

	// Init runtime meshes
	scene.mesh_draws.init(allocator, scene.gltf_scene.meshes_count);
}

static void scene_free_gpu_resources(Scene& scene, Magnefu::Renderer& renderer) {
	Magnefu::GraphicsContext& gpu = *renderer.gpu;

	for (u32 mesh_index = 0; mesh_index < scene.mesh_draws.count(); ++mesh_index) {
		MeshDraw& mesh_draw = scene.mesh_draws[mesh_index];
		gpu.destroy_buffer(mesh_draw.material_buffer);
	}

	scene.mesh_draws.shutdown();
}

static void scene_unload(Scene& scene, Magnefu::Renderer& renderer) {

	Magnefu::GraphicsContext& gpu = *renderer.gpu;

	// Free scene buffers
	scene.samplers.shutdown();
	scene.images.shutdown();
	scene.buffers.shutdown();

	// NOTE(marco): we can't destroy this sooner as textures and buffers
	// hold a pointer to the names stored here
	Magnefu::gltf_free(scene.gltf_scene);
}

static int mesh_material_compare(const void* a, const void* b) {
	const MeshDraw* mesh_a = (const MeshDraw*)a;
	const MeshDraw* mesh_b = (const MeshDraw*)b;

	if (mesh_a->material->render_index < mesh_b->material->render_index) return -1;
	if (mesh_a->material->render_index > mesh_b->material->render_index) return  1;
	return 0;
}

static void get_mesh_vertex_buffer(Scene& scene, i32 accessor_index, Magnefu::BufferHandle& out_buffer_handle, u32& out_buffer_offset) {
	using namespace Magnefu;

	if (accessor_index != -1) {
		glTF::Accessor& buffer_accessor = scene.gltf_scene.accessors[accessor_index];
		glTF::BufferView& buffer_view = scene.gltf_scene.buffer_views[buffer_accessor.buffer_view];
		BufferResource& buffer_gpu = scene.buffers[buffer_accessor.buffer_view];

		out_buffer_handle = buffer_gpu.handle;
		out_buffer_offset = buffer_accessor.byte_offset == glTF::INVALID_INT_VALUE ? 0 : buffer_accessor.byte_offset;
	}
}

static bool get_mesh_material(Magnefu::Renderer& renderer, Scene& scene, Magnefu::glTF::Material& material, MeshDraw& mesh_draw) {
	using namespace Magnefu;

	bool transparent = false;
	GraphicsContext& gpu = *renderer.gpu;

	if (material.pbr_metallic_roughness != nullptr) {
		if (material.pbr_metallic_roughness->base_color_factor_count != 0) {
			MF_CORE_ASSERT(material.pbr_metallic_roughness->base_color_factor_count == 4, "");

			mesh_draw.base_color_factor = {
				material.pbr_metallic_roughness->base_color_factor[0],
				material.pbr_metallic_roughness->base_color_factor[1],
				material.pbr_metallic_roughness->base_color_factor[2],
				material.pbr_metallic_roughness->base_color_factor[3],
			};
		}
		else {
			mesh_draw.base_color_factor = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

		if (material.pbr_metallic_roughness->roughness_factor != glTF::INVALID_FLOAT_VALUE) {
			mesh_draw.metallic_roughness_occlusion_factor.x = material.pbr_metallic_roughness->roughness_factor;
		}
		else {
			mesh_draw.metallic_roughness_occlusion_factor.x = 1.0f;
		}

		if (material.alpha_mode.data != nullptr && strcmp(material.alpha_mode.data, "MASK") == 0) {
			mesh_draw.flags |= DrawFlags_AlphaMask;
			transparent = true;
		}

		if (material.alpha_cutoff != glTF::INVALID_FLOAT_VALUE) {
			mesh_draw.alpha_cutoff = material.alpha_cutoff;
		}

		if (material.pbr_metallic_roughness->metallic_factor != glTF::INVALID_FLOAT_VALUE) {
			mesh_draw.metallic_roughness_occlusion_factor.y = material.pbr_metallic_roughness->metallic_factor;
		}
		else {
			mesh_draw.metallic_roughness_occlusion_factor.y = 1.0f;
		}

		if (material.pbr_metallic_roughness->base_color_texture != nullptr) {
			glTF::Texture& diffuse_texture = scene.gltf_scene.textures[material.pbr_metallic_roughness->base_color_texture->index];
			TextureResource& diffuse_texture_gpu = scene.images[diffuse_texture.source];
			SamplerResource& diffuse_sampler_gpu = scene.samplers[diffuse_texture.sampler];

			mesh_draw.diffuse_texture_index = diffuse_texture_gpu.handle.index;

			gpu.link_texture_sampler(diffuse_texture_gpu.handle, diffuse_sampler_gpu.handle);
		}
		else {
			mesh_draw.diffuse_texture_index = INVALID_TEXTURE_INDEX;
		}

		if (material.pbr_metallic_roughness->metallic_roughness_texture != nullptr) {
			glTF::Texture& roughness_texture = scene.gltf_scene.textures[material.pbr_metallic_roughness->metallic_roughness_texture->index];
			TextureResource& roughness_texture_gpu = scene.images[roughness_texture.source];
			SamplerResource& roughness_sampler_gpu = scene.samplers[roughness_texture.sampler];

			mesh_draw.roughness_texture_index = roughness_texture_gpu.handle.index;

			gpu.link_texture_sampler(roughness_texture_gpu.handle, roughness_sampler_gpu.handle);
		}
		else {
			mesh_draw.roughness_texture_index = INVALID_TEXTURE_INDEX;
		}
	}

	if (material.occlusion_texture != nullptr) {
		glTF::Texture& occlusion_texture = scene.gltf_scene.textures[material.occlusion_texture->index];

		TextureResource& occlusion_texture_gpu = scene.images[occlusion_texture.source];
		SamplerResource& occlusion_sampler_gpu = scene.samplers[occlusion_texture.sampler];

		mesh_draw.occlusion_texture_index = occlusion_texture_gpu.handle.index;

		if (material.occlusion_texture->strength != glTF::INVALID_FLOAT_VALUE) {
			mesh_draw.metallic_roughness_occlusion_factor.z = material.occlusion_texture->strength;
		}
		else {
			mesh_draw.metallic_roughness_occlusion_factor.z = 1.0f;
		}

		gpu.link_texture_sampler(occlusion_texture_gpu.handle, occlusion_sampler_gpu.handle);
	}
	else {
		mesh_draw.occlusion_texture_index = INVALID_TEXTURE_INDEX;
	}

	if (material.normal_texture != nullptr) {
		glTF::Texture& normal_texture = scene.gltf_scene.textures[material.normal_texture->index];
		TextureResource& normal_texture_gpu = scene.images[normal_texture.source];
		SamplerResource& normal_sampler_gpu = scene.samplers[normal_texture.sampler];

		gpu.link_texture_sampler(normal_texture_gpu.handle, normal_sampler_gpu.handle);

		mesh_draw.normal_texture_index = normal_texture_gpu.handle.index;
	}
	else {
		mesh_draw.normal_texture_index = INVALID_TEXTURE_INDEX;
	}

	// Create material buffer
	BufferCreation buffer_creation;
	buffer_creation.reset().set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, ResourceUsageType::Dynamic, sizeof(MeshData)).set_name("mesh_data");
	mesh_draw.material_buffer = gpu.create_buffer(buffer_creation);

	return transparent;
}


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
	MemoryService::Instance()->Init(nullptr);
	service_manager = Magnefu::ServiceManager::instance;

	service_manager->init(&Magnefu::MemoryService::Instance()->systemAllocator);
	

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

	//ResourceManager
	rm = &s_rm;
	rm->init(&MemoryService::Instance()->systemAllocator, nullptr);

	//GPUProfiler
	gpu_profiler = &s_gpu_profiler;
	gpu_profiler->init(&MemoryService::Instance()->systemAllocator, 100);


	// -- App specific create --------------------------------------------------- //
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
	game_camera.camera.init_perpective(0.1f, 4000.f, 60.f, wconf.width * 1.f / wconf.height);
	game_camera.init(true, 20.f, 6.f, 0.1f);

	// -- Loading glTF scene data ------------------------------------------------ //
	cstring file_path = "";
	InjectDefault3DModel(file_path);

	Directory cwd{ };
	directory_current(&cwd);

	char gltf_base_path[512]{ };

	memcpy(gltf_base_path, file_path, strlen(file_path));
	file_directory_from_path(gltf_base_path);

	directory_change(gltf_base_path);

	char gltf_file[512]{ };
	memcpy(gltf_file, file_path, strlen(file_path));
	file_name_from_path(gltf_file);

	// TODO: full scene class to manage glTF and entt resources
	scene = new Scene();

	scene_load_from_gltf(gltf_file, *renderer, &MemoryService::Instance()->systemAllocator, *scene);

	directory_change(cwd.path);

	
	{
			// Create pipeline state
		PipelineCreation pipeline_creation;

		StringBuffer path_buffer;
		path_buffer.init(1024, &MemoryService::Instance()->systemAllocator);

		const char* vert_file = "default-vertex.shader";
		char* vert_path = path_buffer.append_use_f("%s%s", MAGNEFU_SHADER_FOLDER, vert_file);
		FileReadResult vert_code = file_read_text(vert_path, &MemoryService::Instance()->systemAllocator);

		const char* frag_file = "default-fragment.shader";
		char* frag_path = path_buffer.append_use_f("%s%s", MAGNEFU_SHADER_FOLDER, frag_file);
		FileReadResult frag_code = file_read_text(frag_path, &MemoryService::Instance()->systemAllocator);

		// Vertex input
		// TODO(marco): could these be inferred from SPIR-V?
		pipeline_creation.vertex_input.add_vertex_attribute({ 0, 0, 0, VertexComponentFormat::Float3 }); // position
		pipeline_creation.vertex_input.add_vertex_stream({ 0, 12, VertexInputRate::PerVertex });

		pipeline_creation.vertex_input.add_vertex_attribute({ 1, 1, 0, VertexComponentFormat::Float4 }); // tangent
		pipeline_creation.vertex_input.add_vertex_stream({ 1, 16, VertexInputRate::PerVertex });

		pipeline_creation.vertex_input.add_vertex_attribute({ 2, 2, 0, VertexComponentFormat::Float3 }); // normal
		pipeline_creation.vertex_input.add_vertex_stream({ 2, 12, VertexInputRate::PerVertex });

		pipeline_creation.vertex_input.add_vertex_attribute({ 3, 3, 0, VertexComponentFormat::Float2 }); // texcoord
		pipeline_creation.vertex_input.add_vertex_stream({ 3, 8, VertexInputRate::PerVertex });

		// Render pass
		pipeline_creation.render_pass = gpu->get_swapchain_output();
		// Depth
		pipeline_creation.depth_stencil.set_depth(true, VK_COMPARE_OP_LESS_OR_EQUAL);

		// Blend
		pipeline_creation.blend_state.add_blend_state().set_color(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD);

		pipeline_creation.shaders.set_name("main").add_stage(vert_code.data, vert_code.size, VK_SHADER_STAGE_VERTEX_BIT).add_stage(frag_code.data, frag_code.size, VK_SHADER_STAGE_FRAGMENT_BIT);

		// Constant buffer
		BufferCreation buffer_creation;
		buffer_creation.reset().set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, ResourceUsageType::Dynamic, sizeof(UniformData)).set_name("scene_cb");
		scene_cb = gpu->create_buffer(buffer_creation);

		pipeline_creation.name = "main_no_cull";
		Program* program_no_cull = renderer->create_program({ pipeline_creation });

		pipeline_creation.rasterization.cull_mode = VK_CULL_MODE_BACK_BIT;

		pipeline_creation.name = "main_cull";
		Program* program_cull = renderer->create_program({ pipeline_creation });

		MaterialCreation material_creation;

		material_creation.set_name("material_no_cull_opaque").set_program(program_no_cull).set_render_index(0);
		Material* material_no_cull_opaque = renderer->create_material(material_creation);

		material_creation.set_name("material_cull_opaque").set_program(program_cull).set_render_index(1);
		Material* material_cull_opaque = renderer->create_material(material_creation);

		material_creation.set_name("material_no_cull_transparent").set_program(program_no_cull).set_render_index(2);
		Material* material_no_cull_transparent = renderer->create_material(material_creation);

		material_creation.set_name("material_cull_transparent").set_program(program_cull).set_render_index(3);
		Material* material_cull_transparent = renderer->create_material(material_creation);

		path_buffer.shutdown();
		MemoryService::Instance()->systemAllocator.deallocate(vert_code.data);
		MemoryService::Instance()->systemAllocator.deallocate(frag_code.data);

		glTF::Scene& root_gltf_scene = scene->gltf_scene.scenes[scene->gltf_scene.scene];

		for (u32 node_index = 0; node_index < root_gltf_scene.nodes_count; ++node_index)
		{
			glTF::Node& node = scene->gltf_scene.nodes[root_gltf_scene.nodes[node_index]];

			if (node.mesh == glTF::INVALID_INT_VALUE) {
				continue;
			}

			// TODO(marco): children

			glTF::Mesh& mesh = scene->gltf_scene.meshes[node.mesh];

			vec3s node_scale{ 1.0f, 1.0f, 1.0f };
			if (node.scale_count != 0) {
				MF_ASSERT((node.scale_count == 3), "");
				node_scale = vec3s{ node.scale[0], node.scale[1], node.scale[2] };
			}

			// Gltf primitives are conceptually submeshes.
			for (u32 primitive_index = 0; primitive_index < mesh.primitives_count; ++primitive_index) {
				MeshDraw mesh_draw{ };

				mesh_draw.scale = node_scale;

				glTF::MeshPrimitive& mesh_primitive = mesh.primitives[primitive_index];

				const i32 position_accessor_index = gltf_get_attribute_accessor_index(mesh_primitive.attributes, mesh_primitive.attribute_count, "POSITION");
				const i32 tangent_accessor_index = gltf_get_attribute_accessor_index(mesh_primitive.attributes, mesh_primitive.attribute_count, "TANGENT");
				const i32 normal_accessor_index = gltf_get_attribute_accessor_index(mesh_primitive.attributes, mesh_primitive.attribute_count, "NORMAL");
				const i32 texcoord_accessor_index = gltf_get_attribute_accessor_index(mesh_primitive.attributes, mesh_primitive.attribute_count, "TEXCOORD_0");

				get_mesh_vertex_buffer(*scene, position_accessor_index, mesh_draw.position_buffer, mesh_draw.position_offset);
				get_mesh_vertex_buffer(*scene, tangent_accessor_index, mesh_draw.tangent_buffer, mesh_draw.tangent_offset);
				get_mesh_vertex_buffer(*scene, normal_accessor_index, mesh_draw.normal_buffer, mesh_draw.normal_offset);
				get_mesh_vertex_buffer(*scene, texcoord_accessor_index, mesh_draw.texcoord_buffer, mesh_draw.texcoord_offset);

				// Create index buffer
				glTF::Accessor& indices_accessor =  scene->gltf_scene.accessors[mesh_primitive.indices];
				glTF::BufferView& indices_buffer_view =  scene->gltf_scene.buffer_views[indices_accessor.buffer_view];
				BufferResource& indices_buffer_gpu =  scene->buffers[indices_accessor.buffer_view];
				mesh_draw.index_buffer = indices_buffer_gpu.handle;
				mesh_draw.index_offset = indices_accessor.byte_offset == glTF::INVALID_INT_VALUE ? 0 : indices_accessor.byte_offset;
				mesh_draw.primitive_count = indices_accessor.count;

				// Create material
				glTF::Material& material =  scene->gltf_scene.materials[mesh_primitive.material];

				bool transparent = get_mesh_material(*renderer, *scene, material, mesh_draw);

				if (transparent) {
					if (material.double_sided) {
						mesh_draw.material = material_no_cull_transparent;
					}
					else {
						mesh_draw.material = material_cull_transparent;
					}
				}
				else {
					if (material.double_sided) {
						mesh_draw.material = material_no_cull_opaque;
					}
					else {
						mesh_draw.material = material_cull_opaque;
					}
				}

				scene->mesh_draws.push(mesh_draw);
			}
		}
	}

	qsort(scene->mesh_draws.begin(), scene->mesh_draws.count(), sizeof(MeshDraw), mesh_material_compare);


	// will eventually have an EditorLayer
	// may even have more specific layers like Physics Collisions and AI...
	// this is how updates can be controlled separately

	// TODO: How should i give the app access to its layer and overlay?

	layer_stack = new Magnefu::LayerStack();
	layer_stack->PushLayer(new SandboxLayer());
	layer_stack->PushOverlay(new Overlay());


	MF_CORE_INFO("Sandbox Application created successfully!");
}

void Sandbox::Destroy()
{
	using namespace Magnefu;

	MF_CORE_INFO("Begin Sandbox Shutdown...");

	GraphicsContext* gpu = service_manager->get<GraphicsContext>();

	gpu->destroy_buffer(scene_cb);

	// Need to handle deletion of layers

	// Shutdown services
	imgui->Shutdown();
	input->Shutdown();

	gpu_profiler->shutdown();

	scene_free_gpu_resources(*scene, *renderer);

	rm->shutdown();
	renderer->shutdown();

	scene_unload(*scene, *renderer);

	delete scene;


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

	// for sandbox layer
	vec3s eye = vec3s{ 0.0f, 2.5f, 2.0f };
	vec3s look = vec3s{ 0.0f, 0.0, -1.0f };
	vec3s right = vec3s{ 1.0f, 0.0, 0.0f };

	f32 yaw = 0.0f;
	f32 pitch = 0.0f;

	vec3s light{ 0.f, 4.0f, 0.f };

	float model_scale = 1.0f;
	float light_range = 20.0f;
	float light_intensity = 80.0f;
	//

	accumulator = 0.0;
	auto start_time = Magnefu::time_now();

	while (!window->requested_exit)
	{
		//MF_PROFILE_SCOPE("Frame");

		if (!window->minimized)
		{
			//gpu->new_frame();
			renderer->begin_frame();
		}
		
		window->PollEvents();

		input->NewFrame();

		if (window->resized)
		{
			gpu->resize(window->GetWidth(), window->GetHeight());

			window->resized = false;
			game_camera.camera.set_aspect_ratio(window->GetWidth() * 1.f / window->GetHeight());
		}

		imgui->BeginFrame();
		
		auto end_time = Magnefu::time_now();
		f32 delta_time = (f32)Magnefu::time_delta_seconds(start_time, end_time);
		start_time = end_time;

		accumulator += delta_time;

		
		// updates
		input->Update(delta_time);
		game_camera.update(input, window->GetWidth(), window->GetHeight(), delta_time);
		window->CenterMouse(game_camera.mouse_dragging);

		while (accumulator >= step)
		{
			FixedUpdate(delta_time);

			accumulator -= step;
		}

		VariableUpdate(delta_time);

		// For sandbox layer
		if (ImGui::Begin("Magnefu ImGui")) 
		{
			ImGui::InputFloat("Model scale", &model_scale, 0.001f);
			ImGui::InputFloat3("Light position", light.raw);
			ImGui::InputFloat("Light range", &light_range);
			ImGui::InputFloat("Light intensity", &light_intensity);
			ImGui::InputFloat3("Camera position", game_camera.camera.position.raw);
			ImGui::InputFloat3("Camera target movement", game_camera.target_movement.raw);
		}
		ImGui::End();

		if (ImGui::Begin("GPU")) 
		{
			gpu_profiler->imgui_draw();
		}
		ImGui::End();

		mat4s global_model = { };
		{
			// Update rotating cube gpu data
			MapBufferParameters cb_map = { scene_cb, 0, 0 };
			float* cb_data = (float*)gpu->map_buffer(cb_map);
			if (cb_data)
			{
				UniformData uniform_data{ };
				uniform_data.vp = game_camera.camera.view_projection;
				uniform_data.eye = vec4s{ game_camera.camera.position.x, game_camera.camera.position.y, game_camera.camera.position.z, 1.0f };
				uniform_data.light = vec4s{ light.x, light.y, light.z, 1.0f };
				uniform_data.light_range = light_range;
				uniform_data.light_intensity = light_intensity;

				memcpy(cb_data, &uniform_data, sizeof(UniformData));

				gpu->unmap_buffer(cb_map);
			}

			for (u32 mesh_index = 0; mesh_index < scene->mesh_draws.count(); ++mesh_index) 
			{
				MeshDraw& mesh_draw = scene->mesh_draws[mesh_index];

				cb_map.buffer = mesh_draw.material_buffer;
				MeshData* mesh_data = (MeshData*)gpu->map_buffer(cb_map);
				if (mesh_data) {
					upload_material(*mesh_data, mesh_draw, model_scale);

					gpu->unmap_buffer(cb_map);
				}
			}
		}

		// //

		if (!window->minimized)
		{

			DrawGUI();

			auto* gpu_commands = gpu->get_command_buffer(Magnefu::QueueType::Graphics, true);
			
			gpu_commands->push_marker("Frame");
			gpu_commands->clear(0.3f, 0.3f, 0.3f, 1.0f);
			gpu_commands->clear_depth_stencil(1.0f, 0);
			gpu_commands->bind_pass(gpu->get_swapchain_pass());
			//gpu_commands->bind_pipeline(cube_pipeline);
			gpu_commands->set_scissor(nullptr);
			gpu_commands->set_viewport(nullptr);

			Material* last_material = nullptr;
			// TODO: loop by material so that we can deal with multiple passes

			for (u32 mesh_index = 0; mesh_index < scene->mesh_draws.count(); ++mesh_index)
			{
				MeshDraw& mesh_draw = scene->mesh_draws[mesh_index];

				if (mesh_draw.material != last_material)
				{
					PipelineHandle pipeline = renderer->get_pipeline(mesh_draw.material);
					gpu_commands->bind_pipeline(pipeline);

					last_material = mesh_draw.material;
				}

				draw_mesh(*renderer, gpu_commands, mesh_draw);

			}

			
			const f32 interpolation_factor = Maths::clamp(0.0f, 1.0f, (f32)(accumulator / step));
			Render(interpolation_factor);

			//imgui->Render(renderer, *gpu_commands);
			imgui->Render(*gpu_commands);

			gpu_commands->pop_marker();

			gpu_profiler->update(*gpu);

			// Send commands to GPU
			gpu->queue_command_buffer(gpu_commands);
			gpu->present();

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

void Sandbox::DrawGUI()
{
	using namespace Magnefu;


	// Not sure imgui
	for (auto it = layer_stack->end(); it != layer_stack->begin(); )
	{
		(*--it)->DrawGUI();
	}

	// Part of overlay
	//Magnefu::MemoryService::Instance()->imguiDraw();
}

void Sandbox::FixedUpdate(f32 delta)
{
}

void Sandbox::VariableUpdate(f32 delta)
{
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