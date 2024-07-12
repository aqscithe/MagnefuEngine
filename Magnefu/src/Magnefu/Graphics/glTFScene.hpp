#pragma once

#include "FrameGraph.hpp"
#include "GPUResources.hpp"
#include "RenderScene.hpp"
#include "Renderer.hpp"

#include "Magnefu/Core/glTF.hpp"

#include "enkiTS/TaskScheduler.h"


namespace Magnefu
{

	struct glTFScene : public RenderScene
	{

		void                    init(SceneGraph* scene_graph, Allocator* resident_allocator, Renderer* renderer) override;

		void                    add_mesh(cstring filename, cstring path, StackAllocator* temp_allocator, AsynchronousLoader* async_loader) override;
		void                    shutdown(Renderer* renderer) override;

		void                    prepare_draws(Renderer* renderer, StackAllocator* scratch_allocator, SceneGraph* scene_graph) override;

		void                    get_mesh_vertex_buffer(glTF::glTF& gltf_scene, u32 buffers_offset, i32 accessor_index, u32 flag, BufferHandle& out_buffer_handle, u32& out_buffer_offset, u32& out_flags);
		u16                     get_material_texture(GraphicsContext& gpu, glTF::glTF& gltf_scene, glTF::TextureInfo* texture_info);
		u16                     get_material_texture(GraphicsContext& gpu, glTF::glTF& gltf_scene, i32 gltf_texture_index);

		void                    fill_pbr_material(glTF::glTF& gltf_scene, Renderer& renderer, glTF::Material& material, PBRMaterial& pbr_material);

		void processNode(i32 gltf_node_index, i32 parent_index, glTF::Scene& gltf_scene, SceneGraph* scene_graph, u32 node_offset, Array<MeshInstance>& mesh_instances, u32& total_meshlets);



		// All graphics resources used by the scene
		Array<TextureResource>  images;
		Array<SamplerResource>  samplers;
		Array<BufferResource>   buffers;

		Array<glTF::glTF>       gltf_scenes; // Source gltf scene



	}; 


}