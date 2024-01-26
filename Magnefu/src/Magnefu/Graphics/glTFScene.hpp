#pragma once

//#include "FrameGraph.hpp"
#include "GPUResources.hpp"
#include "RenderScene.hpp"
#include "Renderer.hpp"

#include "Magnefu/Core/glTF.hpp"

#include "enkiTS/TaskScheduler.h"


namespace Magnefu
{
	// -- Forward Declarations -------------- //
	struct glTFScene;
	struct Material;


	struct PBRMaterial
	{
		Material* material;

		BufferHandle			material_buffer;
		DescriptorSetHandle		descriptor_set;

		// Indices used for bindless textures
		u16						diffuse_texture_index;
		u16						roughness_texture_index;
		u16						normal_texture_index;
		u16						occlusion_texture_index;

		vec4s					base_color_factor;
		vec4s					metallic_roughness_occlusion_factor;

		f32						alpha_cutoff;
		u32						flags;
	};


	struct Mesh
	{
		PBRMaterial				pbr_material;

		BufferHandle			index_buffer;
		BufferHandle			position_buffer;
		BufferHandle			tangent_buffer;
		BufferHandle			normal_buffer;
		BufferHandle			texcoord_buffer;

		u32						position_offset;
		u32						tangent_offset;
		u32						normal_offset;
		u32						texcoord_offset;

		VkIndexType				index_type;
		u32						index_offset;

		u32						primitive_count;
		u32						scene_graph_node_index = u32_max;

		bool					is_transparent() const { return (pbr_material.flags & (DrawFlags_AlphaMask | DrawFlags_Transparent)) != 0; }
		bool					is_double_sided() const { return (pbr_material.flags & DrawFlags_DoubleSided) == DrawFlags_DoubleSided; }


	};

	struct MeshInstance
	{
		Mesh*	mesh;
		u32		material_pass_index;
	};


	struct GpuMeshData 
	{
		mat4s               world;  // world matrix(model matrix)
		mat4s               inverse_world;

		u32                 textures[4]; // diffuse, roughness, normal, occlusion
		vec4s               base_color_factor;
		vec4s               metallic_roughness_occlusion_factor; // metallic, roughness, occlusion
		f32                 alpha_cutoff;
		f32                 padding_[3];

		u32                 flags;
		u32                 padding1_[3];
	}; // struct GpuMeshData


	// -- Render Passes ------------------------------------------------------------ //

	


	struct glTFScene : public Magnefu::RenderScene
	{

		void                                    load(cstring filename, cstring path, Magnefu::Allocator* resident_allocator, Magnefu::StackAllocator* temp_allocator, Magnefu::AsynchronousLoader* async_loader);
		void                                    free_gpu_resources(Magnefu::Renderer* renderer);
		void                                    unload(Magnefu::Renderer* renderer);

		void                                    prepare_draws(Magnefu::Renderer* renderer, Magnefu::StackAllocator* scratch_allocator);
		void                                    upload_materials(float model_scale);
		void                                    submit_draw_task(Magnefu::ImGuiService* imgui, Magnefu::GPUProfiler* gpu_profiler, enki::TaskScheduler* task_scheduler);

		Magnefu::Array<MeshDraw>                 mesh_draws;

		// All graphics resources used by the scene
		Magnefu::Array<Magnefu::TextureResource>  images;
		Magnefu::Array<Magnefu::SamplerResource>  samplers;
		Magnefu::Array<Magnefu::BufferResource>   buffers;

		Magnefu::glTF::glTF                      gltf_scene; // Source gltf scene

		Magnefu::Renderer* renderer;

	}; // struct GltfScene
}