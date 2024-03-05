#pragma once

#include "Magnefu/Core/Bit.hpp"

#include "cglm/struct/mat4.h"

namespace Magnefu
{

    //
    //
    struct Hierarchy {
        i32                 parent : 24;
        i32                 level : 8;
    }; // struct Hierarchy

    //
    struct SceneGraphNodeDebugData {
        cstring             name;
    }; // struct SceneGraphNodeDebugData


    //
    //
    struct SceneGraph {

        void                init(Allocator* resident_allocator, u32 num_nodes);
        void                shutdown();

        void                resize(u32 num_nodes);
        void                update_matrices();

        void                set_hierarchy(u32 node_index, u32 parent_index, u32 level);
        void                set_local_matrix(u32 node_index, const mat4s& local_matrix);
        void                set_debug_data(u32 node_index, cstring name);

        Array<mat4s>        local_matrices;
        Array<mat4s>        world_matrices;
        Array<Hierarchy>    nodes_hierarchy;
        Array< SceneGraphNodeDebugData> nodes_debug_data;

        BitSet              updated_nodes;

        bool                sort_update_order = true;

    }; // struct SceneGraph

} // namespace Magnefu