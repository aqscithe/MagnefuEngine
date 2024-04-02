#pragma once

#include "GPUResources.hpp"




namespace Magnefu
{
	// -- forward Declarations -------- //
	struct Allocator;
	struct CommandBuffer;
	struct FrameGraph;
	struct GraphicsContext;
	struct RenderScene;


	// -- Typedefs ---------------------------- //

	typedef u32			FrameGraphHandle;


	struct FrameGraphResourceHandle 
	{
		FrameGraphHandle                index;
	};

	struct FrameGraphNodeHandle 
	{
		FrameGraphHandle                index;
	};


	enum FrameGraphResourceType
	{
		FrameGraphResourceType_Invalid = -1,

		FrameGraphResourceType_Buffer = 0,
		FrameGraphResourceType_Texture = 1,
		FrameGraphResourceType_Attachment = 2,
		FrameGraphResourceType_Reference = 3,
		FrameGraphResourceType_ShadingRate = 4
	};


	struct FrameGraphResourceInfo 
	{
		bool                                    external = false;

		union 
		{
			struct 
			{
				sizet                           size;
				VkBufferUsageFlags              flags;

				BufferHandle                    handle;
			} buffer;

			struct 
			{
				u32                             width;
				u32                             height;
				u32                             depth;

				u32								scale_width;
				u32								scale_height;

				VkFormat                        format;
				VkImageUsageFlags               flags;

				RenderPassOperation::Enum       load_op;

				TextureHandle                   handle;
				f32                             clear_values[4];  // Reused between color or depth/stencil.

				bool                            compute;
			} texture;
		};
	};


	// NOTE(marco): an input could be used as a texture or as an attachment.
// If it's an attachment we want to control whether to discard previous
// content - for instance the first time we use it - or to load the data
// from a previous pass
// NOTE(marco): an output always implies an attachment and a store op
	struct FrameGraphResource 
	{
		FrameGraphResourceType                  type;
		FrameGraphResourceInfo                  resource_info;

		FrameGraphNodeHandle                    producer;
		FrameGraphResourceHandle                output_handle;

		i32                                     ref_count = 0;

		const char* name = nullptr;
	};


	// -- Creation Structs ------------------------ //

	struct FrameGraphResourceInputCreation 
	{
		FrameGraphResourceType                  type;
		FrameGraphResourceInfo                  resource_info;

		const char* name;
	};


	struct FrameGraphResourceOutputCreation 
	{
		FrameGraphResourceType                  type;
		FrameGraphResourceInfo                  resource_info;

		const char* name;
	};


	struct FrameGraphNodeCreation
	{
		Array<FrameGraphResourceInputCreation>  inputs;
		Array<FrameGraphResourceOutputCreation> outputs;

		bool                                    enabled;

		const char* name;

		bool									compute;
	};


	struct FrameGraphRenderPass
	{
		virtual void                            add_ui() { }
		virtual void                            pre_render(u32 current_frame_index, CommandBuffer* gpu_commands, FrameGraph* frame_graph, RenderScene* render_scene) { }
		virtual void                            render(u32 current_frame_index, CommandBuffer* gpu_commands, RenderScene* render_scene) { }
		virtual void                            post_render(u32 current_frame_index, CommandBuffer* gpu_commands, FrameGraph* frame_graph, RenderScene* render_scene) { }
		virtual void                            on_resize(GraphicsContext& gpu, FrameGraph* frame_graph, u32 new_width, u32 new_height) {}

		bool                                    enabled = true;
	};

	struct FrameGraphNode
	{
		i32                                     ref_count = 0;

		RenderPassHandle                        render_pass;
		FramebufferHandle                       framebuffer;

		FrameGraphRenderPass* graph_render_pass;

		Array<FrameGraphResourceHandle>         inputs;
		Array<FrameGraphResourceHandle>         outputs;

		Array<FrameGraphNodeHandle>             edges;

		f32                                     resolution_scale_width = 0.f;
		f32                                     resolution_scale_height = 0.f;

		bool                                    compute = false;
		bool                                    ray_tracing = false;
		bool                                    enabled = true;

		const char* name = nullptr;
	};

	struct FrameGraphRenderPassCache 
	{
		void                                    init(Allocator* allocator);
		void                                    shutdown();

		FlatHashMap<u64, FrameGraphRenderPass*> render_pass_map;
	};

	struct FrameGraphResourceCache 
	{
		void                                        init(Allocator* allocator, GraphicsContext* device);
		void                                        shutdown();

		GraphicsContext* device;

		FlatHashMap<u64, u32>                       resource_map;
		ResourcePoolTyped<FrameGraphResource>       resources;
	};

	struct FrameGraphNodeCache
	{
		void                                    init(Allocator* allocator, GraphicsContext* device);
		void                                    shutdown();

		GraphicsContext* device;

		FlatHashMap<u64, u32>                   node_map;
		ResourcePool                            nodes;
	};


	// -- FrameGraphBuilder -------------------------------------------------- //

	struct FrameGraphBuilder : public Service 
	{
		void                            init(GraphicsContext* device);
		void                            shutdown();

		void                            register_render_pass(cstring name, FrameGraphRenderPass* render_pass);

		FrameGraphResourceHandle        create_node_output(const FrameGraphResourceOutputCreation& creation, FrameGraphNodeHandle producer);
		FrameGraphResourceHandle        create_node_input(const FrameGraphResourceInputCreation& creation);
		FrameGraphNodeHandle            create_node(const FrameGraphNodeCreation& creation);

		FrameGraphNode* get_node(cstring name);
		FrameGraphNode* access_node(FrameGraphNodeHandle handle);

		FrameGraphResource* get_resource(cstring name);
		FrameGraphResource* access_resource(FrameGraphResourceHandle handle);
		void                            add_resource(cstring name, FrameGraphResourceType type, FrameGraphResourceInfo resource_info);

		FrameGraphResourceCache         resource_cache;
		FrameGraphNodeCache             node_cache;
		FrameGraphRenderPassCache       render_pass_cache;

		Allocator* allocator;

		GraphicsContext* gpu;

		static constexpr u32            k_max_render_pass_count = 256;
		static constexpr u32            k_max_resources_count = 1024;
		static constexpr u32            k_max_nodes_count = 1024;

		static constexpr cstring        k_name = "Magnefu_frame_graph_builder_service";
	};

	

	// -- FrameGraph -------------------------------------------------------------- //

	struct FrameGraph {
		void                            init(FrameGraphBuilder* builder);
		void                            shutdown();

		void                            parse(cstring file_path, StackAllocator* temp_allocator);

		// NOTE(marco): each frame we rebuild the graph so that we can enable only
		// the nodes we are interested in
		void                            reset();
		void                            enable_render_pass(cstring render_pass_name);
		void                            disable_render_pass(cstring render_pass_name);
		void                            compile();
		void                            add_ui();
		void                            render(u32 current_frame_index, CommandBuffer* gpu_commands, RenderScene* render_scene);
		void                            on_resize(GraphicsContext& gpu, u32 new_width, u32 new_height);

		FrameGraphNode* get_node(cstring name);
		FrameGraphNode* access_node(FrameGraphNodeHandle handle);

		FrameGraphResource* get_resource(cstring name);
		FrameGraphResource* access_resource(FrameGraphResourceHandle handle);

		void                            add_node(FrameGraphNodeCreation& creation);
		void                            add_resource(cstring name, FrameGraphResourceType type, FrameGraphResourceInfo resource_info);


		// NOTE(marco): nodes sorted in topological order
		Array<FrameGraphNodeHandle>     nodes;
		Array<FrameGraphNodeHandle>     all_nodes;

		FrameGraphBuilder* builder;
		Allocator* allocator;

		LinearAllocator                 local_allocator;

		const char* name = nullptr;
	};
}