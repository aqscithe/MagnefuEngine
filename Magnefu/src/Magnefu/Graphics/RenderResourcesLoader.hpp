#pragma once

#include "Renderer.hpp"

namespace Magnefu
{

    struct FrameGraph;

    //
    //
    struct RenderResourcesLoader 
    {

        void                init(Renderer* renderer, StackAllocator* temp_allocator, FrameGraph* frame_graph);
        void                shutdown();

        GpuTechnique*       load_gpu_technique(cstring json_path);
        TextureResource*    load_texture(cstring path, bool generate_mipmaps = true);


        Renderer*           renderer;
        FrameGraph*         frame_graph;
        StackAllocator*     temp_allocator;

    }; // struct RenderResourcesLoader

} // namespace Magnefu