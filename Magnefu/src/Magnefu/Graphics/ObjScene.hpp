#pragma once

#include "GPUResources.hpp"
#include "RenderScene.hpp"
#include "Renderer.hpp"

#include "enkiTS/TaskScheduler.h"


#include "cglm/struct/mat3.h"
#include "cglm/struct/mat4.h"
#include "cglm/struct/quat.h"
#include "cglm/struct/cam.h"
#include "cglm/struct/affine.h"


namespace Magnefu
{
    struct Material;

    struct ObjMaterial
    {
        vec4s                                   diffuse;
        vec3s                                   ambient;
        vec3s                                   specular;
        f32                                     specular_exp;

        f32                                     transparency;

        u16                                     diffuse_texture_index = INVALID_TEXTURE_INDEX;
        u16                                     normal_texture_index = INVALID_TEXTURE_INDEX;
    };


    struct ObjDraw
    {
        BufferHandle                    geometry_buffer_cpu;
        BufferHandle                    geometry_buffer_gpu;
        BufferHandle                    mesh_buffer;

        DescriptorSetHandle             descriptor_set;

        u32                                     index_offset;
        u32                                     position_offset;
        u32                                     tangent_offset;
        u32                                     normal_offset;
        u32                                     texcoord_offset;

        u32                                     primitive_count;

        vec4s                                   diffuse;
        vec3s                                   ambient;
        vec3s                                   specular;
        f32                                     specular_exp;
        f32                                     transparency;

        u16                                     diffuse_texture_index = INVALID_TEXTURE_INDEX;
        u16                                     normal_texture_index = INVALID_TEXTURE_INDEX;

        u32                                     uploads_queued = 0;
        // TODO(marco): this should be an atomic value
        u32                                     uploads_completed = 0;

        Material* material;
    };

    struct ObjGpuData
    {
        mat4s                                   m;
        mat4s                                   inverseM;

        u32                                     textures[4];
        vec4s                                   diffuse;
        vec3s                                   specular;
        f32                                     specular_exp;
        vec3s                                   ambient;
    };

    struct ObjScene : public RenderScene
    {
        void                                    init(cstring filename, cstring path, Allocator* resident_allocator, StackAllocator* temp_allocator, AsynchronousLoader* async_loader_) override;
        void                                    shutdown(Renderer* renderer) override;

        void                                    prepare_draws(Renderer* renderer, StackAllocator* scratch_allocator, SceneGraph* scene_graph) override;
        void                                    upload_materials() override;
        void                                    submit_draw_task(ImGuiService* imgui, GPUProfiler* gpu_profiler, enki::TaskScheduler* task_scheduler) override;

        u32                                     load_texture(cstring texture_path, cstring path, StackAllocator* temp_allocator);
                                                
        // TAG: Change function parameters
        void                                    draw_mesh(Magnefu::CommandBuffer* gpu_commands, ObjDraw& mesh_draw);

        Array<ObjDraw>                  mesh_draws;

        // All graphics resources used by the scene
        Array<ObjMaterial>              materials;
        Array<TextureResource>  images;
        SamplerResource* sampler;

        AsynchronousLoader* async_loader;
        Renderer* renderer;

    }; // struct ObjScene


    struct SecondaryDrawTask : public enki::ITaskSet {

        Renderer* renderer = nullptr;
        ObjScene* scene = nullptr;
        CommandBuffer* parent = nullptr;
        CommandBuffer* cb = nullptr;
        u32                             start = 0;
        u32                             end = 0;

        void init(ObjScene* scene_, Renderer* renderer_, CommandBuffer* parent_, u32 start_, u32 end_);

        void ExecuteRange(enki::TaskSetPartition range_, uint32_t threadnum_) override;
    };

    //
    //
    struct ObjDrawTask : public enki::ITaskSet {

        enki::TaskScheduler* task_scheduler = nullptr;
        GraphicsContext* gpu = nullptr;
        Renderer* renderer = nullptr;
        ImGuiService* imgui = nullptr;
        GPUProfiler* gpu_profiler = nullptr;
        ObjScene* scene = nullptr;
        u32                             thread_id = 0;
        bool                            use_secondary = false;

        void init(enki::TaskScheduler* task_scheduler_, GraphicsContext* gpu_, Renderer* renderer_,
            ImGuiService* imgui_, GPUProfiler* gpu_profiler_,
            ObjScene* scene_, bool use_secondary_);

        void ExecuteRange(enki::TaskSetPartition range_, uint32_t threadnum_) override;

    }; // struct DrawTask
}