#include "SandboxApp.hpp"

// -- App ---------------- //
#include "AppLayers/SandboxLayer.hpp"
#include "AppLayers/Overlay.hpp"

#include "SandboxPrimitives.hpp"

// -- Graphics --------------------------- //


// -- Core -------------------------- //
#include "Magnefu/Core/File.hpp"
#include "Magnefu/Core/glTF.hpp"

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

static Magnefu::ResourceManager s_rm;
static Magnefu::GPUProfiler s_gpu_profiler;


// ----------------------------------------------------------------------------------------------------------- //


// Rotating cube test
Magnefu::BufferHandle                    cube_vb;
Magnefu::BufferHandle                    cube_ib;
Magnefu::PipelineHandle                  cube_pipeline;
Magnefu::BufferHandle                    cube_cb;
Magnefu::DescriptorSetHandle             cube_rl;
Magnefu::DescriptorSetLayoutHandle       cube_dsl;

f32 rx, ry;


// Should be moved to sandbox layer
Magnefu::BufferHandle					dummy_attribute_buffer;
Magnefu::Array<MeshDraw> mesh_draws;
Magnefu::Array<Magnefu::BufferHandle> custom_mesh_buffers{ };
Magnefu::TextureHandle dummy_texture;
Magnefu::SamplerHandle dummy_sampler;
Magnefu::glTF::glTF scene;
Magnefu::Array<Magnefu::TextureResource> images;
Magnefu::Array<Magnefu::SamplerResource> samplers;
Magnefu::Array<Magnefu::BufferResource> buffers;
Magnefu::StringBuffer resource_name_buffer;




static u8* get_buffer_data(Magnefu::glTF::BufferView* buffer_views, u32 buffer_index, Magnefu::Array<void*>& buffers_data, u32* buffer_size = nullptr, char** buffer_name = nullptr) 
{
	using namespace Magnefu;

	glTF::BufferView& buffer = buffer_views[buffer_index];

	i32 offset = buffer.byte_offset;
	if (offset == glTF::INVALID_INT_VALUE) {
		offset = 0;
	}

	if (buffer_name != nullptr) {
		*buffer_name = buffer.name.data;
	}

	if (buffer_size != nullptr) {
		*buffer_size = buffer.byte_length;
	}

	u8* data = (u8*)buffers_data[buffer.buffer] + offset;

	return data;
}


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

	scene = gltf_load_file(gltf_file);

	// Textures
	
	images.init(&MemoryService::Instance()->systemAllocator, scene.images_count);

	for (u32 image_index = 0; image_index < scene.images_count; ++image_index) 
	{
		glTF::Image& image = scene.images[image_index];
		TextureResource* tr = renderer->create_texture(image.uri.data, image.uri.data);
		MF_ASSERT((tr != nullptr), "NO");

		images.push(*tr);
	}

	TextureCreation texture_creation{ };
	u32 zero_value = 0;
	texture_creation.set_name("dummy_texture").set_size(1, 1, 1).set_format_type(VK_FORMAT_R8G8B8A8_UNORM, TextureType::Texture2D).set_flags(1, 0).set_data(&zero_value);
	dummy_texture = gpu->create_texture(texture_creation);

	// Samplers
	SamplerCreation sampler_creation{ };
	sampler_creation.min_filter = VK_FILTER_LINEAR;
	sampler_creation.mag_filter = VK_FILTER_LINEAR;
	sampler_creation.address_mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_creation.address_mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	dummy_sampler = gpu->create_sampler(sampler_creation);

	
	resource_name_buffer.init(mfkilo(64), &MemoryService::Instance()->systemAllocator);

	
	samplers.init(&MemoryService::Instance()->systemAllocator, scene.samplers_count);

	for (u32 sampler_index = 0; sampler_index < scene.samplers_count; ++sampler_index) 
	{
		glTF::Sampler& sampler = scene.samplers[sampler_index];

		char* sampler_name = resource_name_buffer.append_use_f("sampler_%u", sampler_index);

		SamplerCreation creation;
		creation.min_filter = sampler.min_filter == glTF::Sampler::Filter::LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
		creation.mag_filter = sampler.mag_filter == glTF::Sampler::Filter::LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
		creation.name = sampler_name;

		SamplerResource* sr = renderer->create_sampler(creation);
		MF_ASSERT((sr != nullptr), "No sampler resource - {}", sampler_name);

		samplers.push(*sr);
	}
	

	// Buffers
	Array<void*> buffers_data;
	buffers_data.init(&MemoryService::Instance()->systemAllocator, scene.buffers_count);

	for (u32 buffer_index = 0; buffer_index < scene.buffers_count; ++buffer_index) {
		glTF::Buffer& buffer = scene.buffers[buffer_index];

		FileReadResult buffer_data = file_read_binary(buffer.uri.data, &MemoryService::Instance()->systemAllocator);
		buffers_data.push(buffer_data.data);
	}

	
	buffers.init(&MemoryService::Instance()->systemAllocator, scene.buffer_views_count);

	for (u32 buffer_index = 0; buffer_index < scene.buffer_views_count; ++buffer_index) 
	{
		char* buffer_name = nullptr;
		u32 buffer_size = 0;
		u8* data = get_buffer_data(scene.buffer_views, buffer_index, buffers_data, &buffer_size, &buffer_name);

		// NOTE(marco): the target attribute of a BufferView is not mandatory, so we prepare for both uses
		VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		if (buffer_name == nullptr) {
			buffer_name = resource_name_buffer.append_use_f("buffer_%u", buffer_index);
		}
		else {
			// NOTE(marco); some buffers might have the same name, which causes issues in the renderer cache
			buffer_name = resource_name_buffer.append_use_f("%s_%u", buffer_name, buffer_index);
		}

		BufferResource* br = renderer->create_buffer(flags, ResourceUsageType::Immutable, buffer_size, data, buffer_name);
		MF_ASSERT((br != nullptr), "No buffer resource - {}", buffer_name);

		buffers.push(*br);
	}

	// NOTE(marco): restore working directory
	directory_change(cwd.path);

	
	mesh_draws.init(&MemoryService::Instance()->systemAllocator, scene.meshes_count);

	
	custom_mesh_buffers.init(&MemoryService::Instance()->systemAllocator, 8);

	vec4s dummy_data[3]{ };
	BufferCreation buffer_creation{ };
	buffer_creation.set(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, ResourceUsageType::Immutable, sizeof(vec4s) * 3).set_data(dummy_data).set_name("dummy_attribute_buffer");

	dummy_attribute_buffer = gpu->create_buffer(buffer_creation);

	{
		// Create pipeline state
		PipelineCreation pipeline_creation;

		// Vertex input
		// TODO(marco): component format should be based on buffer view type
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


		// Shader data
		cstring vs_path = "res/shaders/default-vertex.shader";
		cstring fs_path = "res/shaders/default-fragment.shader";

		FileReadResult vs_read = file_read_text(vs_path, &MemoryService::Instance()->systemAllocator);
		FileReadResult fs_read = file_read_text(fs_path, &MemoryService::Instance()->systemAllocator);
	
		const char* vs_code = vs_read.data;
		const char* fs_code = fs_read.data;

		pipeline_creation.shaders.set_name("Cube").add_stage(vs_code, (uint32_t)strlen(vs_code), VK_SHADER_STAGE_VERTEX_BIT).add_stage(fs_code, (uint32_t)strlen(fs_code), VK_SHADER_STAGE_FRAGMENT_BIT);

		// Descriptor set layout
		DescriptorSetLayoutCreation cube_rll_creation{};
		cube_rll_creation.add_binding({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1, "LocalConstants" });
		cube_rll_creation.add_binding({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, 1, "MaterialConstant" });
		cube_rll_creation.add_binding({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, 1, "diffuseTexture" });
		cube_rll_creation.add_binding({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, 1, "roughnessMetalnessTexture" });
		cube_rll_creation.add_binding({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, 1, "roughnessMetalnessTexture" });
		cube_rll_creation.add_binding({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, 1, "emissiveTexture" });
		cube_rll_creation.add_binding({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, 1, "occlusionTexture" });
		// Setting it into pipeline
		cube_dsl = gpu->create_descriptor_set_layout(cube_rll_creation);
		pipeline_creation.add_descriptor_set_layout(cube_dsl);

		// Constant buffer
		BufferCreation buffer_creation;
		buffer_creation.reset().set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, ResourceUsageType::Dynamic, sizeof(UniformData)).set_name("cube_cb");
		cube_cb = gpu->create_buffer(buffer_creation);

		cube_pipeline = gpu->create_pipeline(pipeline_creation);

		glTF::Scene& root_gltf_scene = scene.scenes[scene.scene];

		Array<i32> node_parents;
		node_parents.init(&MemoryService::Instance()->systemAllocator, scene.nodes_count, scene.nodes_count);

		Array<u32> node_stack;
		node_stack.init(&MemoryService::Instance()->systemAllocator, 8);

		Array<mat4s> node_matrix;
		node_matrix.init(&MemoryService::Instance()->systemAllocator, scene.nodes_count, scene.nodes_count);

		for (u32 node_index = 0; node_index < root_gltf_scene.nodes_count; ++node_index) 
		{
			u32 root_node = root_gltf_scene.nodes[node_index];
			node_parents[root_node] = -1;
			node_stack.push(root_node);
		}

		while (node_stack.count()) 
		{
			u32 node_index = node_stack.back();
			node_stack.pop();
			glTF::Node& node = scene.nodes[node_index];

			mat4s local_matrix{ };

			if (node.matrix_count) {
				// CGLM and glTF have the same matrix layout, just memcopy it
				memcpy(&local_matrix, node.matrix, sizeof(mat4s));
			}
			else {
				vec3s node_scale{ 1.0f, 1.0f, 1.0f };
				if (node.scale_count != 0) {
					MF_ASSERT((node.scale_count == 3), "Not 3");
					node_scale = vec3s{ node.scale[0], node.scale[1], node.scale[2] };
				}

				vec3s node_translation{ 0.f, 0.f, 0.f };
				if (node.translation_count) {
					MF_ASSERT((node.translation_count == 3), "Not 3");
					node_translation = vec3s{ node.translation[0], node.translation[1], node.translation[2] };
				}

				// Rotation is written as a plain quaternion
				versors node_rotation = glms_quat_identity();
				if (node.rotation_count) {
					MF_ASSERT((node.rotation_count == 4), "Not 3");
					node_rotation = glms_quat_init(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]);
				}

				Transform transform;
				transform.translation = node_translation;
				transform.scale = node_scale;
				transform.rotation = node_rotation;

				local_matrix = transform.calculate_matrix();
			}

			node_matrix[node_index] = local_matrix;

			for (u32 child_index = 0; child_index < node.children_count; ++child_index) {
				u32 child_node_index = node.children[child_index];
				node_parents[child_node_index] = node_index;
				node_stack.push(child_node_index);
			}

			if (node.mesh == glTF::INVALID_INT_VALUE) 
			{
				continue;
			}

			glTF::Mesh& mesh = scene.meshes[node.mesh];

			mat4s final_matrix = local_matrix;
			i32 node_parent = node_parents[node_index];
			while (node_parent != -1) {
				final_matrix = glms_mat4_mul(node_matrix[node_parent], final_matrix);
				node_parent = node_parents[node_parent];
			}

			// Final SRT composition
			for (u32 primitive_index = 0; primitive_index < mesh.primitives_count; ++primitive_index) {
				MeshDraw mesh_draw{ };

				mesh_draw.material_data.model = final_matrix;

				glTF::MeshPrimitive& mesh_primitive = mesh.primitives[primitive_index];

				glTF::Accessor& indices_accessor = scene.accessors[mesh_primitive.indices];
				MF_ASSERT((indices_accessor.component_type == glTF::Accessor::UNSIGNED_INT || indices_accessor.component_type == glTF::Accessor::UNSIGNED_SHORT), "Not uint or ushort");
				mesh_draw.index_type = indices_accessor.component_type == glTF::Accessor::UNSIGNED_INT ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;

				glTF::BufferView& indices_buffer_view = scene.buffer_views[indices_accessor.buffer_view];
				BufferResource& indices_buffer_gpu = buffers[indices_accessor.buffer_view];
				mesh_draw.index_buffer = indices_buffer_gpu.handle;
				mesh_draw.index_offset = indices_accessor.byte_offset == glTF::INVALID_INT_VALUE ? 0 : indices_accessor.byte_offset;
				mesh_draw.count = indices_accessor.count;
				MF_ASSERT((mesh_draw.count % 3) == 0, "Mesh draw count not divisible by 3");

				i32 position_accessor_index = gltf_get_attribute_accessor_index(mesh_primitive.attributes, mesh_primitive.attribute_count, "POSITION");
				i32 tangent_accessor_index = gltf_get_attribute_accessor_index(mesh_primitive.attributes, mesh_primitive.attribute_count, "TANGENT");
				i32 normal_accessor_index = gltf_get_attribute_accessor_index(mesh_primitive.attributes, mesh_primitive.attribute_count, "NORMAL");
				i32 texcoord_accessor_index = gltf_get_attribute_accessor_index(mesh_primitive.attributes, mesh_primitive.attribute_count, "TEXCOORD_0");

				vec3s* position_data = nullptr;
				u32* index_data_32 = (u32*)get_buffer_data(scene.buffer_views, indices_accessor.buffer_view, buffers_data);
				u16* index_data_16 = (u16*)index_data_32;
				u32 vertex_count = 0;

				if (position_accessor_index != -1) {
					glTF::Accessor& position_accessor = scene.accessors[position_accessor_index];
					glTF::BufferView& position_buffer_view = scene.buffer_views[position_accessor.buffer_view];
					BufferResource& position_buffer_gpu = buffers[position_accessor.buffer_view];

					vertex_count = position_accessor.count;

					mesh_draw.position_buffer = position_buffer_gpu.handle;
					mesh_draw.position_offset = position_accessor.byte_offset == glTF::INVALID_INT_VALUE ? 0 : position_accessor.byte_offset;

					position_data = (vec3s*)get_buffer_data(scene.buffer_views, position_accessor.buffer_view, buffers_data);
				}
				else {
					MF_ASSERT(false, "No position data found!");
					continue;
				}

				if (normal_accessor_index != -1) {
					glTF::Accessor& normal_accessor = scene.accessors[normal_accessor_index];
					glTF::BufferView& normal_buffer_view = scene.buffer_views[normal_accessor.buffer_view];
					BufferResource& normal_buffer_gpu = buffers[normal_accessor.buffer_view];

					mesh_draw.normal_buffer = normal_buffer_gpu.handle;
					mesh_draw.normal_offset = normal_accessor.byte_offset == glTF::INVALID_INT_VALUE ? 0 : normal_accessor.byte_offset;
				}
				else {
					// NOTE(marco): we could compute this at runtime
					Array<vec3s> normals_array{ };
					normals_array.init(&MemoryService::Instance()->systemAllocator, vertex_count, vertex_count);
					memset(normals_array.begin(), 0, normals_array.count() * sizeof(vec3s));

					u32 index_count = mesh_draw.count;
					for (u32 index = 0; index < index_count; index += 3) {
						u32 i0 = indices_accessor.component_type == glTF::Accessor::UNSIGNED_INT ? index_data_32[index] : index_data_16[index];
						u32 i1 = indices_accessor.component_type == glTF::Accessor::UNSIGNED_INT ? index_data_32[index + 1] : index_data_16[index + 1];
						u32 i2 = indices_accessor.component_type == glTF::Accessor::UNSIGNED_INT ? index_data_32[index + 2] : index_data_16[index + 2];

						vec3s p0 = position_data[i0];
						vec3s p1 = position_data[i1];
						vec3s p2 = position_data[i2];

						vec3s a = glms_vec3_sub(p1, p0);
						vec3s b = glms_vec3_sub(p2, p0);

						vec3s normal = glms_cross(a, b);

						normals_array[i0] = glms_vec3_add(normals_array[i0], normal);
						normals_array[i1] = glms_vec3_add(normals_array[i1], normal);
						normals_array[i2] = glms_vec3_add(normals_array[i2], normal);
					}

					for (u32 vertex = 0; vertex < vertex_count; ++vertex) {
						normals_array[vertex] = glms_normalize(normals_array[vertex]);
					}

					BufferCreation normals_creation{ };
					normals_creation.set(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, ResourceUsageType::Immutable, normals_array.count() * sizeof(vec3s)).set_name("normals").set_data(normals_array.begin());

					mesh_draw.normal_buffer = gpu->create_buffer(normals_creation);
					mesh_draw.normal_offset = 0;

					custom_mesh_buffers.push(mesh_draw.normal_buffer);

					normals_array.shutdown();
				}

				if (tangent_accessor_index != -1) {
					glTF::Accessor& tangent_accessor = scene.accessors[tangent_accessor_index];
					glTF::BufferView& tangent_buffer_view = scene.buffer_views[tangent_accessor.buffer_view];
					BufferResource& tangent_buffer_gpu = buffers[tangent_accessor.buffer_view];

					mesh_draw.tangent_buffer = tangent_buffer_gpu.handle;
					mesh_draw.tangent_offset = tangent_accessor.byte_offset == glTF::INVALID_INT_VALUE ? 0 : tangent_accessor.byte_offset;

					mesh_draw.material_data.flags |= MaterialFeatures_TangentVertexAttribute;
				}

				if (texcoord_accessor_index != -1) {
					glTF::Accessor& texcoord_accessor = scene.accessors[texcoord_accessor_index];
					glTF::BufferView& texcoord_buffer_view = scene.buffer_views[texcoord_accessor.buffer_view];
					BufferResource& texcoord_buffer_gpu = buffers[texcoord_accessor.buffer_view];

					mesh_draw.texcoord_buffer = texcoord_buffer_gpu.handle;
					mesh_draw.texcoord_offset = texcoord_accessor.byte_offset == glTF::INVALID_INT_VALUE ? 0 : texcoord_accessor.byte_offset;

					mesh_draw.material_data.flags |= MaterialFeatures_TexcoordVertexAttribute;
				}

				MF_ASSERT((mesh_primitive.material != glTF::INVALID_INT_VALUE), "Mesh with no material is not supported!");
				glTF::Material& material = scene.materials[mesh_primitive.material];

				// Descriptor Set
				DescriptorSetCreation ds_creation{};
				ds_creation.set_layout(cube_dsl).buffer(cube_cb, 0);

				buffer_creation.reset().set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, ResourceUsageType::Dynamic, sizeof(MaterialData)).set_name("material");
				mesh_draw.material_buffer = gpu->create_buffer(buffer_creation);
				ds_creation.buffer(mesh_draw.material_buffer, 1);

				if (material.pbr_metallic_roughness != nullptr) {
					if (material.pbr_metallic_roughness->base_color_factor_count != 0) {
						MF_ASSERT((material.pbr_metallic_roughness->base_color_factor_count == 4), "Color factor should be 4.");

						mesh_draw.material_data.base_color_factor = {
							material.pbr_metallic_roughness->base_color_factor[0],
							material.pbr_metallic_roughness->base_color_factor[1],
							material.pbr_metallic_roughness->base_color_factor[2],
							material.pbr_metallic_roughness->base_color_factor[3],
						};
					}
					else {
						mesh_draw.material_data.base_color_factor = { 1.0f, 1.0f, 1.0f, 1.0f };
					}

					if (material.pbr_metallic_roughness->base_color_texture != nullptr) {
						glTF::Texture& diffuse_texture = scene.textures[material.pbr_metallic_roughness->base_color_texture->index];
						TextureResource& diffuse_texture_gpu = images[diffuse_texture.source];

						SamplerHandle sampler_handle = dummy_sampler;
						if (diffuse_texture.sampler != glTF::INVALID_INT_VALUE) {
							sampler_handle = samplers[diffuse_texture.sampler].handle;
						}

						ds_creation.texture_sampler(diffuse_texture_gpu.handle, sampler_handle, 2);

						mesh_draw.material_data.flags |= MaterialFeatures_ColorTexture;
					}
					else {
						ds_creation.texture_sampler(dummy_texture, dummy_sampler, 2);
					}

					if (material.pbr_metallic_roughness->metallic_roughness_texture != nullptr) {
						glTF::Texture& roughness_texture = scene.textures[material.pbr_metallic_roughness->metallic_roughness_texture->index];
						TextureResource& roughness_texture_gpu = images[roughness_texture.source];

						SamplerHandle sampler_handle = dummy_sampler;
						if (roughness_texture.sampler != glTF::INVALID_INT_VALUE) {
							sampler_handle = samplers[roughness_texture.sampler].handle;
						}

						ds_creation.texture_sampler(roughness_texture_gpu.handle, sampler_handle, 3);

						mesh_draw.material_data.flags |= MaterialFeatures_RoughnessTexture;
					}
					else {
						ds_creation.texture_sampler(dummy_texture, dummy_sampler, 3);
					}

					if (material.pbr_metallic_roughness->metallic_factor != glTF::INVALID_FLOAT_VALUE) {
						mesh_draw.material_data.metallic_factor = material.pbr_metallic_roughness->metallic_factor;
					}
					else {
						mesh_draw.material_data.metallic_factor = 1.0f;
					}

					if (material.pbr_metallic_roughness->roughness_factor != glTF::INVALID_FLOAT_VALUE) {
						mesh_draw.material_data.roughness_factor = material.pbr_metallic_roughness->roughness_factor;
					}
					else {
						mesh_draw.material_data.roughness_factor = 1.0f;
					}
				}

				if (material.occlusion_texture != nullptr) {
					glTF::Texture& occlusion_texture = scene.textures[material.occlusion_texture->index];

					// NOTE(marco): this could be the same as the roughness texture, but for now we treat it as a separate
					// texture
					TextureResource& occlusion_texture_gpu = images[occlusion_texture.source];

					SamplerHandle sampler_handle = dummy_sampler;
					if (occlusion_texture.sampler != glTF::INVALID_INT_VALUE) {
						sampler_handle = samplers[occlusion_texture.sampler].handle;
					}

					ds_creation.texture_sampler(occlusion_texture_gpu.handle, sampler_handle, 4);

					mesh_draw.material_data.occlusion_factor = material.occlusion_texture->strength != glTF::INVALID_FLOAT_VALUE ? material.occlusion_texture->strength : 1.0f;
					mesh_draw.material_data.flags |= MaterialFeatures_OcclusionTexture;
				}
				else {
					mesh_draw.material_data.occlusion_factor = 1.0f;
					ds_creation.texture_sampler(dummy_texture, dummy_sampler, 4);
				}

				if (material.emissive_factor_count != 0) {
					mesh_draw.material_data.emissive_factor = vec3s{
						material.emissive_factor[0],
						material.emissive_factor[1],
						material.emissive_factor[2],
					};
				}

				if (material.emissive_texture != nullptr) {
					glTF::Texture& emissive_texture = scene.textures[material.emissive_texture->index];

					// NOTE(marco): this could be the same as the roughness texture, but for now we treat it as a separate
					// texture
					TextureResource& emissive_texture_gpu = images[emissive_texture.source];

					SamplerHandle sampler_handle = dummy_sampler;
					if (emissive_texture.sampler != glTF::INVALID_INT_VALUE) {
						sampler_handle = samplers[emissive_texture.sampler].handle;
					}

					ds_creation.texture_sampler(emissive_texture_gpu.handle, sampler_handle, 5);

					mesh_draw.material_data.flags |= MaterialFeatures_EmissiveTexture;
				}
				else {
					ds_creation.texture_sampler(dummy_texture, dummy_sampler, 5);
				}

				if (material.normal_texture != nullptr) {
					glTF::Texture& normal_texture = scene.textures[material.normal_texture->index];
					TextureResource& normal_texture_gpu = images[normal_texture.source];

					SamplerHandle sampler_handle = dummy_sampler;
					if (normal_texture.sampler != glTF::INVALID_INT_VALUE) {
						sampler_handle = samplers[normal_texture.sampler].handle;
					}

					ds_creation.texture_sampler(normal_texture_gpu.handle, sampler_handle, 6);

					mesh_draw.material_data.flags |= MaterialFeatures_NormalTexture;
				}
				else {
					ds_creation.texture_sampler(dummy_texture, dummy_sampler, 6);
				}

				mesh_draw.descriptor_set = gpu->create_descriptor_set(ds_creation);

				mesh_draws.push(mesh_draw);
			}
		}

		node_parents.shutdown();
		node_stack.shutdown();
		node_matrix.shutdown();

		rx = 0.0f;
		ry = 0.0f;

	}

	auto& allocator = MemoryService::Instance()->systemAllocator;

	for (u32 buffer_index = 0; buffer_index < scene.buffers_count; ++buffer_index) 
	{
		void* buffer = buffers_data[buffer_index];
		allocator.deallocate(buffer);
	}
	buffers_data.shutdown();


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

	// For sandbox layer
	for (u32 mesh_index = 0; mesh_index < mesh_draws.count(); ++mesh_index) 
	{
		MeshDraw& mesh_draw = mesh_draws[mesh_index];
		gpu->destroy_descriptor_set(mesh_draw.descriptor_set);
		gpu->destroy_buffer(mesh_draw.material_buffer);
	}

	for (u32 mi = 0; mi < custom_mesh_buffers.count(); ++mi) 
	{
		gpu->destroy_buffer(custom_mesh_buffers[mi]);
	}
	custom_mesh_buffers.shutdown();

	gpu->destroy_buffer(dummy_attribute_buffer);
	gpu->destroy_texture(dummy_texture);
	gpu->destroy_sampler(dummy_sampler);

	mesh_draws.shutdown();

	gpu->destroy_buffer(cube_cb);
	gpu->destroy_pipeline(cube_pipeline);
	gpu->destroy_descriptor_set_layout(cube_dsl);
	// // //

	// Need to handle deletion of layers

	// Shutdown services
	imgui->Shutdown();
	input->Shutdown();

	gpu_profiler->shutdown();

	rm->shutdown();
	renderer->shutdown();

	samplers.shutdown();
	images.shutdown();
	buffers.shutdown();

	resource_name_buffer.shutdown();


	// NOTE(marco): we can't destroy this sooner as textures and buffers
	// hold a pointer to the names stored here
	gltf_free(scene);


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

	float model_scale = 1.0f;
	//

	accumulator = 0.0;
	auto start_time = Magnefu::time_now();

	while (!window->requested_exit)
	{
		if (!window->minimized)
		{
			renderer->begin_frame();
		}
		
		
		window->PollEvents();

		if (window->resized)
		{
			renderer->resize_swapchain(window->GetWidth(), window->GetHeight());

			window->resized = false;
		}

		imgui->BeginFrame();
		
		auto end_time = Magnefu::time_now();
		f32 delta_time = (f32)Magnefu::time_delta_seconds(start_time, end_time);
		start_time = end_time;

		accumulator += delta_time;


		input->NewFrame();
		input->Update(delta_time);

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
			MapBufferParameters cb_map = { cube_cb, 0, 0 };
			float* cb_data = (float*)gpu->map_buffer(cb_map);
			if (cb_data) {
				if (input->IsMouseDown(MouseButtons::MOUSE_BUTTON_LEFT) && !ImGui::GetIO().WantCaptureMouse) {
					pitch += (input->mouse_position.y - input->previous_mouse_position.y) * 0.1f;
					yaw += (input->mouse_position.x - input->previous_mouse_position.x) * 0.3f;

					pitch = Maths::clamp(-60.0f, 60.0f, pitch);

					if (yaw > 360.0f) 
					{
						yaw -= 360.0f;
					}

					mat3s rxm = glms_mat4_pick3(glms_rotate_make(glm_rad(-pitch), vec3s{ 1.0f, 0.0f, 0.0f }));
					mat3s rym = glms_mat4_pick3(glms_rotate_make(glm_rad(-yaw), vec3s{ 0.0f, 1.0f, 0.0f }));

					look = glms_mat3_mulv(rxm, vec3s{ 0.0f, 0.0f, -1.0f });
					look = glms_mat3_mulv(rym, look);

					right = glms_cross(look, vec3s{ 0.0f, 1.0f, 0.0f });
				}

				if (input->IsKeyDown(Keys::MF_KEY_W)) 
				{
					eye = glms_vec3_add(eye, glms_vec3_scale(look, 5.0f * delta_time));
				}
				else if (input->IsKeyDown(Keys::MF_KEY_S))
				{
					eye = glms_vec3_sub(eye, glms_vec3_scale(look, 5.0f * delta_time));
				}

				if (input->IsKeyDown(Keys::MF_KEY_D)) 
				{
					eye = glms_vec3_add(eye, glms_vec3_scale(right, 5.0f * delta_time));
				}
				else if (input->IsKeyDown(Keys::MF_KEY_A)) 
				{
					eye = glms_vec3_sub(eye, glms_vec3_scale(right, 5.0f * delta_time));
				}

				mat4s view = glms_lookat(eye, glms_vec3_add(eye, look), vec3s{ 0.0f, 1.0f, 0.0f });
				mat4s projection = glms_perspective(glm_rad(60.0f), gpu->swapchain_width * 1.0f / gpu->swapchain_height, 0.01f, 1000.0f);

				// Calculate view projection matrix
				mat4s view_projection = glms_mat4_mul(projection, view);

				// Rotate cube:
				rx += 1.0f * delta_time;
				ry += 2.0f * delta_time;

				mat4s rxm = glms_rotate_make(rx, vec3s{ 1.0f, 0.0f, 0.0f });
				mat4s rym = glms_rotate_make(glm_rad(45.0f), vec3s{ 0.0f, 1.0f, 0.0f });

				mat4s sm = glms_scale_make(vec3s{ model_scale, model_scale, model_scale });
				global_model = glms_mat4_mul(rym, sm);

				UniformData uniform_data{ };
				uniform_data.vp = view_projection;
				uniform_data.m = global_model;
				uniform_data.eye = vec4s{ eye.x, eye.y, eye.z, 1.0f };
				uniform_data.light = vec4s{ 2.0f, 2.0f, 0.0f, 1.0f };

				memcpy(cb_data, &uniform_data, sizeof(UniformData));

				gpu->unmap_buffer(cb_map);
			}
		}

		// //

		if (!window->minimized)
		{

			DrawGUI();

			auto* gpu_commands = renderer->get_command_buffer(Magnefu::QueueType::Graphics, true);
			
			gpu_commands->push_marker("Frame");
			gpu_commands->clear(0.3f, 0.9f, 0.3f, 1.0f);
			gpu_commands->clear_depth_stencil(1.0f, 0);
			gpu_commands->bind_pass(gpu->get_swapchain_pass());
			gpu_commands->bind_pipeline(cube_pipeline);
			gpu_commands->set_scissor(nullptr);
			gpu_commands->set_viewport(nullptr);

			for (u32 mesh_index = 0; mesh_index < mesh_draws.count(); ++mesh_index) {
				MeshDraw mesh_draw = mesh_draws[mesh_index];
				mesh_draw.material_data.model_inv = glms_mat4_inv(glms_mat4_transpose(glms_mat4_mul(global_model, mesh_draw.material_data.model)));

				MapBufferParameters material_map = { mesh_draw.material_buffer, 0, 0 };
				MaterialData* material_buffer_data = (MaterialData*)gpu->map_buffer(material_map);

				memcpy(material_buffer_data, &mesh_draw.material_data, sizeof(MaterialData));

				gpu->unmap_buffer(material_map);

				gpu_commands->bind_vertex_buffer(mesh_draw.position_buffer, 0, mesh_draw.position_offset);
				gpu_commands->bind_vertex_buffer(mesh_draw.normal_buffer, 2, mesh_draw.normal_offset);

				if (mesh_draw.material_data.flags & MaterialFeatures_TangentVertexAttribute) {
					gpu_commands->bind_vertex_buffer(mesh_draw.tangent_buffer, 1, mesh_draw.tangent_offset);
				}
				else {
					gpu_commands->bind_vertex_buffer(dummy_attribute_buffer, 1, 0);
				}

				if (mesh_draw.material_data.flags & MaterialFeatures_TexcoordVertexAttribute) {
					gpu_commands->bind_vertex_buffer(mesh_draw.texcoord_buffer, 3, mesh_draw.texcoord_offset);
				}
				else {
					gpu_commands->bind_vertex_buffer(dummy_attribute_buffer, 3, 0);
				}

				gpu_commands->bind_index_buffer(mesh_draw.index_buffer, mesh_draw.index_offset, mesh_draw.index_type);
				gpu_commands->bind_descriptor_set(&mesh_draw.descriptor_set, 1, nullptr, 0);

				gpu_commands->draw_indexed(TopologyType::Triangle, mesh_draw.count, 1, 0, 0, 0);
			}

			
			const f32 interpolation_factor = Maths::clamp(0.0f, 1.0f, (f32)(accumulator / step));
			Render(interpolation_factor);

			//imgui->Render(renderer, *gpu_commands);
			imgui->Render(*gpu_commands);

			gpu_commands->pop_marker();

			gpu_profiler->update(*gpu);

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