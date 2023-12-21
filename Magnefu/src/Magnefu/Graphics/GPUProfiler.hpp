#pragma once

// -- Graphics Includes ------------------------------- //
#include "GraphicsContext.h"

// -- Core Includes -------------------------------- //
#include "Magnefu/Core/MemoryAllocation/Memory.hpp"

namespace Magnefu
{

    // -- GPUProfiler ------------------------------------------------------------ //

    struct GPUProfiler 
    {
        // -- Methods ------------------------------------------------------------- //
        void                        init(Allocator* allocator, u32 max_frames);
        void                        shutdown();

        void                        update(GraphicsContext& gpu);

        void                        imgui_draw();


        // -- Members ----------------------------------------------------- //

        Allocator*                  allocator;
        GPUTimestamp*               timestamps;
        u16*                         per_frame_active;

        u32                         max_frames;
        u32                         current_frame;

        f32                         max_time;
        f32                         min_time;
        f32                         average_time;

        f32                         max_duration;
        bool                        paused;

    }; // struct GPUProfiler

} // namespace magnefu
