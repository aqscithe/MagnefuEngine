#include "SandboxApp.hpp"

// -- App ---------------- //
#include "AppLayers/SandboxLayer.hpp"
#include "AppLayers/Overlay.hpp"
#include "GameCamera.hpp"

#include "Magnefu/Application/Scene/Scene.h"

// -- Graphics --------------------------- //
#include "Magnefu/Graphics/AsynchronousLoader.hpp"


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




// -- Sandbox App -------------------------------------------------------------------------- //

// Forward Declarations
struct glTFScene;


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
static Magnefu::Scene* scene = nullptr;
static GameCamera game_camera;









struct RenderData
{
	vec3s eye = vec3s{ 0.0f, 2.5f, 2.0f };
	vec3s look = vec3s{ 0.0f, 0.0, -1.0f };
	vec3s right = vec3s{ 1.0f, 0.0, 0.0f };

	f32 yaw = 0.0f;
	f32 pitch = 0.0f;

	vec3s light{ 0.f, 4.0f, 0.f };

	float model_scale = 1.0f;
	float light_range = 20.0f;
	float light_intensity = 80.0f;
};



//
//
enum DrawFlags 
{
	DrawFlags_AlphaMask = 1 << 0,
}; // enum DrawFlags


struct MeshDraw 
{
    Magnefu::Material* material;

    Magnefu::BufferHandle    index_buffer;
    Magnefu::BufferHandle    position_buffer;
    Magnefu::BufferHandle    tangent_buffer;
    Magnefu::BufferHandle    normal_buffer;
    Magnefu::BufferHandle    texcoord_buffer;
    Magnefu::BufferHandle    material_buffer;

    VkIndexType index_type;
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

    Magnefu::DescriptorSetHandle descriptor_set;
}; // struct MeshDraw

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








static Magnefu::AsynchronousLoader           s_async_loader;

// ------------------ Static Methods ------------------------------ //

static void         get_mesh_vertex_buffer(glTFScene& scene, i32 accessor_index, Magnefu::BufferHandle& out_buffer_handle, u32& out_buffer_offset);
static bool         get_mesh_material(Magnefu::Renderer& renderer, glTFScene& scene, Magnefu::glTF::Material& material, MeshDraw& mesh_draw);
static void         upload_material(MeshData& mesh_data, const MeshDraw& mesh_draw, const f32 global_scale);
static void         upload_material(ObjGpuData& mesh_data, const ObjDraw& mesh_draw, const f32 global_scale);
static int          gltf_mesh_material_compare(const void* a, const void* b);
static int          obj_mesh_material_compare(const void* a, const void* b);


//
//
static void upload_material(MeshData& mesh_data, const MeshDraw& mesh_draw, const f32 global_scale) {
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

static bool recreate_per_thread_descriptors = false;

//
//
static void upload_material(ObjGpuData& mesh_data, const ObjDraw& mesh_draw, const f32 global_scale) {
	mesh_data.textures[0] = mesh_draw.diffuse_texture_index;
	mesh_data.textures[1] = mesh_draw.normal_texture_index;
	mesh_data.textures[2] = 0;
	mesh_data.textures[3] = 0;
	mesh_data.diffuse = mesh_draw.diffuse;
	mesh_data.specular = mesh_draw.specular;
	mesh_data.specular_exp = mesh_draw.specular_exp;
	mesh_data.ambient = mesh_draw.ambient;

	mat4s model = glms_scale_make(vec3s{ global_scale, global_scale, global_scale });
	mesh_data.m = model;
	mesh_data.inverseM = glms_mat4_inv(glms_mat4_transpose(model));
}

//
//
static void draw_mesh(Magnefu::Renderer& renderer, Magnefu::CommandBuffer* gpu_commands, MeshDraw& mesh_draw) {

	gpu_commands->bind_vertex_buffer(mesh_draw.position_buffer, 0, mesh_draw.position_offset);
	gpu_commands->bind_vertex_buffer(mesh_draw.tangent_buffer, 1, mesh_draw.tangent_offset);
	gpu_commands->bind_vertex_buffer(mesh_draw.normal_buffer, 2, mesh_draw.normal_offset);
	gpu_commands->bind_vertex_buffer(mesh_draw.texcoord_buffer, 3, mesh_draw.texcoord_offset);
	gpu_commands->bind_index_buffer(mesh_draw.index_buffer, mesh_draw.index_offset, mesh_draw.index_type);

	if (recreate_per_thread_descriptors)
    {
		Magnefu::DescriptorSetCreation ds_creation{};
		ds_creation.buffer(scene_cb, 0).buffer(mesh_draw.material_buffer, 1);
		Magnefu::DescriptorSetHandle descriptor_set = renderer.create_descriptor_set(gpu_commands, mesh_draw.material, ds_creation);

		gpu_commands->bind_local_descriptor_set(&descriptor_set, 1, nullptr, 0);
	}
	else 
    {
		gpu_commands->bind_descriptor_set(&mesh_draw.descriptor_set, 1, nullptr, 0);
	}

	gpu_commands->draw_indexed(Magnefu::TopologyType::Triangle, mesh_draw.primitive_count, 1, 0, 0, 0);
}

//
//
static void draw_mesh(Magnefu::Renderer& renderer, Magnefu::CommandBuffer* gpu_commands, ObjDraw& mesh_draw)
{

	gpu_commands->bind_vertex_buffer(mesh_draw.geometry_buffer_gpu, 0, mesh_draw.position_offset);
	gpu_commands->bind_vertex_buffer(mesh_draw.geometry_buffer_gpu, 1, mesh_draw.tangent_offset);
	gpu_commands->bind_vertex_buffer(mesh_draw.geometry_buffer_gpu, 2, mesh_draw.normal_offset);
	gpu_commands->bind_vertex_buffer(mesh_draw.geometry_buffer_gpu, 3, mesh_draw.texcoord_offset);
	gpu_commands->bind_index_buffer(mesh_draw.geometry_buffer_gpu, mesh_draw.index_offset, VK_INDEX_TYPE_UINT32);

	if (recreate_per_thread_descriptors)
    {
		Magnefu::DescriptorSetCreation ds_creation{};
		ds_creation.buffer(scene_cb, 0).buffer(mesh_draw.geometry_buffer_gpu, 1);
		Magnefu::DescriptorSetHandle descriptor_set = renderer.create_descriptor_set(gpu_commands, mesh_draw.material, ds_creation);

		gpu_commands->bind_local_descriptor_set(&descriptor_set, 1, nullptr, 0);
	}
	else 
    {
		gpu_commands->bind_descriptor_set(&mesh_draw.descriptor_set, 1, nullptr, 0);
	}

	gpu_commands->draw_indexed(Magnefu::TopologyType::Triangle, mesh_draw.primitive_count, 1, 0, 0, 0);
}

//
//


//
//


//
//


// DrawTask ///////////////////////////////////////////////////////////////

//
//
struct glTFDrawTask : public enki::ITaskSet 
{

    Magnefu::GraphicsContext* gpu = nullptr;
    Magnefu::Renderer* renderer = nullptr;
    Magnefu::ImGuiService* imgui = nullptr;
    Magnefu::GPUProfiler* gpu_profiler = nullptr;
    glTFScene* scene = nullptr;
    u32                             thread_id = 0;

    void init(Magnefu::GraphicsContext* gpu_, Magnefu::Renderer* renderer_, Magnefu::ImGuiService* imgui_, Magnefu::GPUProfiler* gpu_profiler_,
        glTFScene* scene_) 
    {
        gpu = gpu_;
        renderer = renderer_;
        imgui = imgui_;
        gpu_profiler = gpu_profiler_;
        scene = scene_;
    }

    void ExecuteRange(enki::TaskSetPartition range_, u32 threadnum_) override 
    {

        using namespace Magnefu;

        thread_id = threadnum_;

        //rprint( "Executing draw task from thread %u\n", threadnum_ );
        // TODO: improve getting a command buffer/pool
        Magnefu::CommandBuffer* gpu_commands = gpu->get_command_buffer(threadnum_, true);
        gpu_commands->push_marker("Frame");

        gpu_commands->clear(0.3f, 0.3f, 0.3f, 1.f);
        gpu_commands->clear_depth_stencil(1.0f, 0);
        gpu_commands->bind_pass(gpu->get_swapchain_pass(), false);
        gpu_commands->set_scissor(nullptr);
        gpu_commands->set_viewport(nullptr);

        Material* last_material = nullptr;
        // TODO(marco): loop by material so that we can deal with multiple passes
        for (u32 mesh_index = 0; mesh_index < scene->mesh_draws.size; ++mesh_index) {
            MeshDraw& mesh_draw = scene->mesh_draws[mesh_index];

            if (mesh_draw.material != last_material) {
                PipelineHandle pipeline = renderer->get_pipeline(mesh_draw.material);

                gpu_commands->bind_pipeline(pipeline);

                last_material = mesh_draw.material;
            }

            draw_mesh(*renderer, gpu_commands, mesh_draw);
        }

        imgui->Render(*gpu_commands, false);

        gpu_commands->pop_marker();

        gpu_profiler->update(*gpu);

        // Send commands to GPU
        gpu->queue_command_buffer(gpu_commands);
    }

}; // struct DrawTask

//
//


void glTFScene::load(cstring filename, cstring path, Magnefu::Allocator* resident_allocator, Magnefu::StackAllocator* temp_allocator, Magnefu::AsynchronousLoader* async_loader) {

    using namespace Magnefu;

    renderer = async_loader->renderer;
    enki::TaskScheduler* task_scheduler = async_loader->task_scheduler;
    sizet temp_allocator_initial_marker = temp_allocator->getMarker();

    // Time statistics
    i64 start_scene_loading = time_now();

    gltf_scene = gltf_load_file(filename);

    i64 end_loading_file = time_now();

    // Load all textures
    images.init(resident_allocator, gltf_scene.images_count);

    Array<TextureCreation> tcs;
    tcs.init(temp_allocator, gltf_scene.images_count, gltf_scene.images_count);

    StringBuffer name_buffer;
    name_buffer.init(4096, temp_allocator);

    for (u32 image_index = 0; image_index < gltf_scene.images_count; ++image_index) {
        Magnefu::glTF::Image& image = gltf_scene.images[image_index];

        int comp, width, height;

        stbi_info(image.uri.data, &width, &height, &comp);

        u32 mip_levels = 1;
        if (true) {
            u32 w = width;
            u32 h = height;

            while (w > 1 && h > 1) {
                w /= 2;
                h /= 2;

                ++mip_levels;
            }
        }

        Magnefu::TextureCreation tc;
        tc.set_data(nullptr).set_format_type(VK_FORMAT_R8G8B8A8_UNORM, Magnefu::TextureType::Texture2D).set_flags(mip_levels, 0).set_size((u16)width, (u16)height, 1).set_name(image.uri.data);
        TextureResource* tr = renderer->create_texture(tc);
        MF_ASSERT((tr != nullptr), "no texture resource");

        images.push(*tr);

        // Reconstruct file path
        char* full_filename = name_buffer.append_use_f("%s%s", path, image.uri.data);
        async_loader->request_texture_data(full_filename, tr->handle);
        // Reset name buffer
        name_buffer.clear();
    }

    i64 end_loading_textures_files = time_now();

    i64 end_creating_textures = time_now();

    // Load all samplers
    samplers.init(resident_allocator, gltf_scene.samplers_count);

    for (u32 sampler_index = 0; sampler_index < gltf_scene.samplers_count; ++sampler_index) {
        glTF::Sampler& sampler = gltf_scene.samplers[sampler_index];

        char* sampler_name = name_buffer.append_use_f("sampler_%u", sampler_index);

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

        SamplerResource* sr = renderer->create_sampler(creation);
        MF_ASSERT((sr != nullptr), "");

        samplers.push(*sr);
    }

    i64 end_creating_samplers = time_now();

    // Temporary array of buffer data
    Magnefu::Array<void*> buffers_data;
    buffers_data.init(resident_allocator, gltf_scene.buffers_count);

    for (u32 buffer_index = 0; buffer_index < gltf_scene.buffers_count; ++buffer_index) {
        glTF::Buffer& buffer = gltf_scene.buffers[buffer_index];

        FileReadResult buffer_data = file_read_binary(buffer.uri.data, resident_allocator);
        buffers_data.push(buffer_data.data);
    }

    i64 end_reading_buffers_data = time_now();

    // Load all buffers and initialize them with buffer data
    buffers.init(resident_allocator, gltf_scene.buffer_views_count);

    for (u32 buffer_index = 0; buffer_index < gltf_scene.buffer_views_count; ++buffer_index) {
        glTF::BufferView& buffer = gltf_scene.buffer_views[buffer_index];

        i32 offset = buffer.byte_offset;
        if (offset == glTF::INVALID_INT_VALUE) {
            offset = 0;
        }

        u8* buffer_data = (u8*)buffers_data[buffer.buffer] + offset;

        // NOTE(marco): the target attribute of a BufferView is not mandatory, so we prepare for both uses
        VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        char* buffer_name = buffer.name.data;
        if (buffer_name == nullptr) {
            buffer_name = name_buffer.append_use_f("buffer_%u", buffer_index);
        }

        BufferResource* br = renderer->create_buffer(flags, ResourceUsageType::Immutable, buffer.byte_length, buffer_data, buffer_name);
        MF_ASSERT((br != nullptr), "");

        buffers.push(*br);
    }

    for (u32 buffer_index = 0; buffer_index < gltf_scene.buffers_count; ++buffer_index) {
        void* buffer = buffers_data[buffer_index];
        resident_allocator->deallocate(buffer);
    }
    buffers_data.shutdown();

    i64 end_creating_buffers = time_now();

    // This is not needed anymore, free all temp memory after.
    //resource_name_buffer.shutdown();
    temp_allocator->freeToMarker(temp_allocator_initial_marker);

    // Init runtime meshes
    mesh_draws.init(resident_allocator, gltf_scene.meshes_count);

    i64 end_loading = time_now();

    MF_APP_INFO("Loaded scene {} in {} seconds.\nStats:\n\tReading GLTF file {} seconds\n\tTextures Creating {} seconds\n\tCreating Samplers {} seconds\n\tReading Buffers Data {} seconds\n\tCreating Buffers {} seconds\n", filename,
        time_delta_seconds(start_scene_loading, end_loading), time_delta_seconds(start_scene_loading, end_loading_file), time_delta_seconds(end_loading_file, end_creating_textures),
        time_delta_seconds(end_creating_textures, end_creating_samplers),
        time_delta_seconds(end_creating_samplers, end_reading_buffers_data), time_delta_seconds(end_reading_buffers_data, end_creating_buffers));
}

void glTFScene::free_gpu_resources(Magnefu::Renderer* renderer) 
{
    Magnefu::GraphicsContext& gpu = *renderer->gpu;

    for (u32 mesh_index = 0; mesh_index < mesh_draws.size; ++mesh_index) {
        MeshDraw& mesh_draw = mesh_draws[mesh_index];
        gpu.destroy_buffer(mesh_draw.material_buffer);

        gpu.destroy_descriptor_set(mesh_draw.descriptor_set);
    }

    mesh_draws.shutdown();
}

void glTFScene::unload(Magnefu::Renderer* renderer) {
    Magnefu::GraphicsContext& gpu = *renderer->gpu;

    // Free scene buffers
    samplers.shutdown();
    images.shutdown();
    buffers.shutdown();

    // NOTE(marco): we can't destroy this sooner as textures and buffers
    // hold a pointer to the names stored here
    Magnefu::gltf_free(gltf_scene);
}

void glTFScene::prepare_draws(Magnefu::Renderer* renderer, Magnefu::StackAllocator* scratch_allocator) 
{

    using namespace Magnefu;

    // Create pipeline state
    PipelineCreation pipeline_creation;

    sizet cached_scratch_size = scratch_allocator->getMarker();

    StringBuffer path_buffer;
    path_buffer.init(1024, scratch_allocator);

    const char* vert_file = "main.vert";
    char* vert_path = path_buffer.append_use_f("%s%s", MAGNEFU_SHADER_FOLDER, vert_file);
    FileReadResult vert_code = file_read_text(vert_path, scratch_allocator);

    const char* frag_file = "main.frag";
    char* frag_path = path_buffer.append_use_f("%s%s", MAGNEFU_SHADER_FOLDER, frag_file);
    FileReadResult frag_code = file_read_text(frag_path, scratch_allocator);

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
    pipeline_creation.render_pass = renderer->gpu->get_swapchain_output();
    // Depth
    pipeline_creation.depth_stencil.set_depth(true, VK_COMPARE_OP_LESS_OR_EQUAL);

    // Blend
    pipeline_creation.blend_state.add_blend_state().set_color(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD);

    pipeline_creation.shaders.set_name("main").add_stage(vert_code.data, vert_code.size, VK_SHADER_STAGE_VERTEX_BIT).add_stage(frag_code.data, frag_code.size, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Constant buffer
    BufferCreation buffer_creation;
    buffer_creation.reset().set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, ResourceUsageType::Dynamic, sizeof(UniformData)).set_name("scene_cb");
    scene_cb = renderer->gpu->create_buffer(buffer_creation);

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

    scratch_allocator->freeToMarker(cached_scratch_size);

    glTF::Scene& root_gltf_scene = gltf_scene.scenes[gltf_scene.scene];

    for (u32 node_index = 0; node_index < root_gltf_scene.nodes_count; ++node_index) {
        glTF::Node& node = gltf_scene.nodes[root_gltf_scene.nodes[node_index]];

        if (node.mesh == glTF::INVALID_INT_VALUE) {
            continue;
        }

        // TODO(marco): children

        glTF::Mesh& mesh = gltf_scene.meshes[node.mesh];

        vec3s node_scale{ 1.0f, 1.0f, 1.0f };
        if (node.scale_count != 0) {
            MF_ASSERT(node.scale_count == 3, "");
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

            get_mesh_vertex_buffer(*this, position_accessor_index, mesh_draw.position_buffer, mesh_draw.position_offset);
            get_mesh_vertex_buffer(*this, tangent_accessor_index, mesh_draw.tangent_buffer, mesh_draw.tangent_offset);
            get_mesh_vertex_buffer(*this, normal_accessor_index, mesh_draw.normal_buffer, mesh_draw.normal_offset);
            get_mesh_vertex_buffer(*this, texcoord_accessor_index, mesh_draw.texcoord_buffer, mesh_draw.texcoord_offset);

            // Create index buffer
            glTF::Accessor& indices_accessor = gltf_scene.accessors[mesh_primitive.indices];
            MF_ASSERT(indices_accessor.component_type == glTF::Accessor::ComponentType::UNSIGNED_SHORT || indices_accessor.component_type == glTF::Accessor::ComponentType::UNSIGNED_INT, "");
            mesh_draw.index_type = (indices_accessor.component_type == glTF::Accessor::ComponentType::UNSIGNED_SHORT) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

            glTF::BufferView& indices_buffer_view = gltf_scene.buffer_views[indices_accessor.buffer_view];
            BufferResource& indices_buffer_gpu = buffers[indices_accessor.buffer_view];
            mesh_draw.index_buffer = indices_buffer_gpu.handle;
            mesh_draw.index_offset = indices_accessor.byte_offset == glTF::INVALID_INT_VALUE ? 0 : indices_accessor.byte_offset;
            mesh_draw.primitive_count = indices_accessor.count;

            // Create material
            glTF::Material& material = gltf_scene.materials[mesh_primitive.material];

            bool transparent = get_mesh_material(*renderer, *this, material, mesh_draw);

            Magnefu::DescriptorSetCreation ds_creation{};
            DescriptorSetLayoutHandle layout = renderer->gpu->get_descriptor_set_layout(program_cull->passes[0].pipeline, 0);
            ds_creation.buffer(scene_cb, 0).buffer(mesh_draw.material_buffer, 1).set_layout(layout);
            mesh_draw.descriptor_set = renderer->gpu->create_descriptor_set(ds_creation);

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

            mesh_draws.push(mesh_draw);
        }
    }

    qsort(mesh_draws.data, mesh_draws.size, sizeof(MeshDraw), gltf_mesh_material_compare);
}

void glTFScene::upload_materials(float model_scale) {
    // Update per mesh material buffer
    for (u32 mesh_index = 0; mesh_index < mesh_draws.size; ++mesh_index) {
        MeshDraw& mesh_draw = mesh_draws[mesh_index];

        Magnefu::MapBufferParameters cb_map = { mesh_draw.material_buffer, 0, 0 };
        MeshData* mesh_data = (MeshData*)renderer->gpu->map_buffer(cb_map);
        if (mesh_data) {
            upload_material(*mesh_data, mesh_draw, model_scale);

            renderer->gpu->unmap_buffer(cb_map);
        }
    }
}

void glTFScene::submit_draw_task(Magnefu::ImGuiService* imgui, Magnefu::GPUProfiler* gpu_profiler, enki::TaskScheduler* task_scheduler) {
    glTFDrawTask draw_task;
    draw_task.init(renderer->gpu, renderer, imgui, gpu_profiler, this);
    task_scheduler->AddTaskSetToPipe(&draw_task);
    task_scheduler->WaitforTaskSet(&draw_task);
    task_scheduler->WaitforTask(&draw_task);

    // Avoid using the same command buffer
    renderer->add_texture_update_commands((draw_task.thread_id + 1) % task_scheduler->GetNumTaskThreads());
}

int gltf_mesh_material_compare(const void* a, const void* b) {
    const MeshDraw* mesh_a = (const MeshDraw*)a;
    const MeshDraw* mesh_b = (const MeshDraw*)b;

    if (mesh_a->material->render_index < mesh_b->material->render_index) return -1;
    if (mesh_a->material->render_index > mesh_b->material->render_index) return  1;
    return 0;
}

int obj_mesh_material_compare(const void* a, const void* b) {
    const ObjDraw* mesh_a = (const ObjDraw*)a;
    const ObjDraw* mesh_b = (const ObjDraw*)b;

    if (mesh_a->material->render_index < mesh_b->material->render_index) return -1;
    if (mesh_a->material->render_index > mesh_b->material->render_index) return  1;
    return 0;
}

void get_mesh_vertex_buffer(glTFScene& scene, i32 accessor_index, Magnefu::BufferHandle& out_buffer_handle, u32& out_buffer_offset) {
    using namespace Magnefu;

    if (accessor_index != -1) {
        glTF::Accessor& buffer_accessor = scene.gltf_scene.accessors[accessor_index];
        glTF::BufferView& buffer_view = scene.gltf_scene.buffer_views[buffer_accessor.buffer_view];
        BufferResource& buffer_gpu = scene.buffers[buffer_accessor.buffer_view];

        out_buffer_handle = buffer_gpu.handle;
        out_buffer_offset = buffer_accessor.byte_offset == glTF::INVALID_INT_VALUE ? 0 : buffer_accessor.byte_offset;
    }
}

bool get_mesh_material(Magnefu::Renderer& renderer, glTFScene& scene, Magnefu::glTF::Material& material, MeshDraw& mesh_draw) {
    using namespace Magnefu;

    bool transparent = false;
    GraphicsContext& gpu = *renderer.gpu;

    if (material.pbr_metallic_roughness != nullptr) {
        if (material.pbr_metallic_roughness->base_color_factor_count != 0) {
            MF_ASSERT(material.pbr_metallic_roughness->base_color_factor_count == 4, "");

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
    memory_configuration.maxDynamicSize = mfmega(500);

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


	// will eventually have an EditorLayer
	// may even have more specific layers like Physics Collisions and AI...
	// this is how updates can be controlled separately

	// TODO: How should i give the app access to its layer and overlay?

	layer_stack = new Magnefu::LayerStack();
	layer_stack->PushLayer(new SandboxLayer());
	layer_stack->PushOverlay(new Overlay());

	// -- Loading glTF scene data ------------------------------------------------ //
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

    scene->load(file_name, file_base_path, allocator, scratch_allocator, &s_async_loader);

    // NOTE(marco): restore working directory
    directory_change(cwd.path);

    scene->prepare_draws(renderer, scratch_allocator);

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

    scene->free_gpu_resources(renderer);


	rm->shutdown();
	renderer->shutdown();

	scene->unload(renderer);
    scene->Shutdown();

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

	//

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

            // Change aspect ratio
			game_camera.camera.set_aspect_ratio(window->GetWidth() * 1.f / window->GetHeight());
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
				ImGui::InputFloat("Model scale", &render_data.model_scale, 0.001f);

                ImGui::SeparatorText("LIGHTS");
				ImGui::SliderFloat3("Light position", render_data.light.raw, -30.f, 30.f);
				ImGui::InputFloat("Light range", &render_data.light_range);
				ImGui::InputFloat("Light intensity", &render_data.light_intensity);
                
                ImGui::SeparatorText("CAMERA");
				ImGui::InputFloat3("Camera position", game_camera.camera.position.raw);
				ImGui::InputFloat3("Camera target movement", game_camera.target_movement.raw);

                ImGui::SeparatorText("OPTIONS");
                ImGui::Checkbox("Dynamically recreate descriptor sets", &recreate_per_thread_descriptors);
                ImGui::Checkbox("Use secondary command buffers", &use_secondary_command_buffers);
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

            MapBufferParameters cb_map = { scene_cb, 0, 0 };
            float* cb_data = (float*)gpu->map_buffer(cb_map);
            if (cb_data)
            {
                UniformData uniform_data{ };
                uniform_data.vp = game_camera.camera.view_projection;
                uniform_data.eye = vec4s{ game_camera.camera.position.x, game_camera.camera.position.y, game_camera.camera.position.z, 1.0f };
                uniform_data.light = vec4s{ render_data->light.x, render_data->light.y, render_data->light.z, 1.0f };
                uniform_data.light_range = render_data->light_range;
                uniform_data.light_intensity = render_data->light_intensity;

                memcpy(cb_data, &uniform_data, sizeof(UniformData));

                gpu->unmap_buffer(cb_map);
            }

            scene->upload_materials(render_data->model_scale);
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
	game_camera.update(input, window->GetWidth(), window->GetHeight(), delta_time);
	window->CenterMouse(game_camera.mouse_dragging);
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