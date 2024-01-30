#include "mfpch.h"

#include "ObjScene.hpp"

#include "GPUProfiler.hpp"
#include "AsynchronousLoader.hpp"
#include "Magnefu/Application/ImGui/ImGuiService.hpp"

#include "stb_image/stb_image.h"

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"


namespace Magnefu
{
    // -- STATIC FUNCTIONS ------------------ //

    static void copy_gpu_material_data(ObjGpuData& mesh_data, const ObjDraw& mesh_draw, const f32 global_scale) 
    {
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

    static int obj_mesh_material_compare(const void* a, const void* b) 
    {
        const ObjDraw* mesh_a = (const ObjDraw*)a;
        const ObjDraw* mesh_b = (const ObjDraw*)b;

        if (mesh_a->material->render_index < mesh_b->material->render_index) return -1;
        if (mesh_a->material->render_index > mesh_b->material->render_index) return  1;
        return 0;
    }


    // -- ObjScene ------------------------------------------- //

    void ObjScene::draw_mesh(CommandBuffer* gpu_commands, ObjDraw& mesh_draw)
    {

        gpu_commands->bind_vertex_buffer(mesh_draw.geometry_buffer_gpu, 0, mesh_draw.position_offset);
        gpu_commands->bind_vertex_buffer(mesh_draw.geometry_buffer_gpu, 1, mesh_draw.tangent_offset);
        gpu_commands->bind_vertex_buffer(mesh_draw.geometry_buffer_gpu, 2, mesh_draw.normal_offset);
        gpu_commands->bind_vertex_buffer(mesh_draw.geometry_buffer_gpu, 3, mesh_draw.texcoord_offset);
        gpu_commands->bind_index_buffer(mesh_draw.geometry_buffer_gpu, mesh_draw.index_offset, VK_INDEX_TYPE_UINT32);

        if (recreate_per_thread_descriptors)
        {
            DescriptorSetCreation ds_creation{};
            ds_creation.buffer(scene_cb, 0).buffer(mesh_draw.geometry_buffer_gpu, 1);
            DescriptorSetHandle descriptor_set = renderer->create_descriptor_set(gpu_commands, mesh_draw.material, ds_creation);

            gpu_commands->bind_local_descriptor_set(&descriptor_set, 1, nullptr, 0);
        }
        else
        {
            gpu_commands->bind_descriptor_set(&mesh_draw.descriptor_set, 1, nullptr, 0);
        }

        gpu_commands->draw_indexed(TopologyType::Triangle, mesh_draw.primitive_count, 1, 0, 0, 0);
    }

    void ObjScene::init(cstring filename, cstring path, Allocator* resident_allocator, StackAllocator* temp_allocator, AsynchronousLoader* async_loader_)
    {
        using namespace Magnefu;

        async_loader = async_loader_;
        renderer = async_loader->renderer;

        enki::TaskScheduler* task_scheduler = async_loader->task_scheduler;
        sizet temp_allocator_initial_marker = temp_allocator->getMarker();

        // Time statistics
        i64 start_scene_loading = time_now();

        const struct aiScene* aiscene = aiImportFile(filename,
            aiProcess_CalcTangentSpace |
            aiProcess_GenNormals |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType);

        i64 end_loading_file = time_now();

        // If the import failed, report it
        if (aiscene == nullptr)
        {
            MF_ASSERT(false, "Failed to import scene");
            return;
        }

        SamplerCreation sampler_creation{ };
        sampler_creation.set_address_mode_uv(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT).set_min_mag_mip(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR);
        sampler = renderer->create_sampler(sampler_creation);

        images.init(resident_allocator, 1024);

        materials.init(resident_allocator, aiscene->mNumMaterials);

        for (u32 material_index = 0; material_index < aiscene->mNumMaterials; ++material_index) {
            aiMaterial* material = aiscene->mMaterials[material_index];

            ObjMaterial Magnefu_material{ };

            aiString texture_file;

            if (aiGetMaterialString(material, AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), &texture_file) == AI_SUCCESS) {
                Magnefu_material.diffuse_texture_index = load_texture(texture_file.C_Str(), path, temp_allocator);
            }

            if (aiGetMaterialString(material, AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), &texture_file) == AI_SUCCESS)
            {
                Magnefu_material.normal_texture_index = load_texture(texture_file.C_Str(), path, temp_allocator);
            }

            aiColor4D color;
            if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &color) == AI_SUCCESS) {
                Magnefu_material.diffuse = { color.r, color.g, color.b, 1.0f };
            }

            if (aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &color) == AI_SUCCESS) {
                Magnefu_material.ambient = { color.r, color.g, color.b };
            }

            if (aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &color) == AI_SUCCESS) {
                Magnefu_material.specular = { color.r, color.g, color.b };
            }

            float f_value;
            if (aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &f_value) == AI_SUCCESS) {
                Magnefu_material.specular_exp = f_value;
            }

            if (aiGetMaterialFloat(material, AI_MATKEY_OPACITY, &f_value) == AI_SUCCESS) {
                Magnefu_material.transparency = f_value;
                Magnefu_material.diffuse.w = f_value;
            }

            materials.push(Magnefu_material);
        }

        i64 end_loading_textures_files = time_now();

        i64 end_creating_textures = time_now();

        // Init runtime meshes
        mesh_draws.init(resident_allocator, aiscene->mNumMeshes);

        for (u32 mesh_index = 0; mesh_index < aiscene->mNumMeshes; ++mesh_index)
        {
            aiMesh* mesh = aiscene->mMeshes[mesh_index];

            MF_ASSERT((mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) != 0, "");

            Array<vec3s> positions;
            positions.init(resident_allocator, mesh->mNumVertices);

            Array<vec4s> tangents;
            tangents.init(resident_allocator, mesh->mNumVertices);

            Array<vec3s> normals;
            normals.init(resident_allocator, mesh->mNumVertices);

            Array<vec2s> uv_coords;
            uv_coords.init(resident_allocator, mesh->mNumVertices);

            for (u32 vertex_index = 0; vertex_index < mesh->mNumVertices; ++vertex_index)
            {
                positions.push(vec3s{
                    (float)mesh->mVertices[vertex_index].x,
                    (float)mesh->mVertices[vertex_index].y,
                    (float)mesh->mVertices[vertex_index].z
                    });

                tangents.push(vec4s{
                    (float)mesh->mTangents[vertex_index].x,
                    (float)mesh->mTangents[vertex_index].y,
                    (float)mesh->mTangents[vertex_index].z,
                    1.0f
                    });

                uv_coords.push(vec2s{
                    (float)mesh->mTextureCoords[0][vertex_index].x,
                    (float)mesh->mTextureCoords[0][vertex_index].y,
                    });

                normals.push(vec3s{
                    (float)mesh->mNormals[vertex_index].x,
                    (float)mesh->mNormals[vertex_index].y,
                    (float)mesh->mNormals[vertex_index].z
                    });
            }

            Array<u32> indices;
            indices.init(resident_allocator, mesh->mNumFaces * 3);

            for (u32 face_index = 0; face_index < mesh->mNumFaces; ++face_index) {
                MF_ASSERT(mesh->mFaces[face_index].mNumIndices == 3, "");

                indices.push(mesh->mFaces[face_index].mIndices[0]);
                indices.push(mesh->mFaces[face_index].mIndices[1]);
                indices.push(mesh->mFaces[face_index].mIndices[2]);
            }

            sizet buffer_size = (indices.size * sizeof(u32)) +
                (positions.size * sizeof(vec3s)) +
                (normals.size * sizeof(vec3s)) +
                (tangents.size * sizeof(vec4s)) +
                (uv_coords.size * sizeof(vec2s));

            // NOTE(marco): the target attribute of a BufferView is not mandatory, so we prepare for both uses
            VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

            BufferCreation creation{ };
            creation.set(flags, ResourceUsageType::Immutable, buffer_size).set_persistent(true).set_name(nullptr);

            BufferHandle br = renderer->gpu->create_buffer(creation);

            Buffer* buffer = renderer->gpu->access_buffer(br);

            ObjDraw& Magnefu_mesh = mesh_draws.push_use();
            memset(&Magnefu_mesh, 0, sizeof(ObjDraw));

            Magnefu_mesh.geometry_buffer_cpu = br;

            sizet offset = 0;

            memcpy(buffer->mapped_data + offset, indices.data, indices.size * sizeof(u32));
            Magnefu_mesh.index_offset = offset;
            offset += indices.size * sizeof(u32);

            memcpy(buffer->mapped_data + offset, positions.data, positions.size * sizeof(vec3s));
            Magnefu_mesh.position_offset = offset;
            offset += positions.size * sizeof(vec3s);

            memcpy(buffer->mapped_data + offset, tangents.data, tangents.size * sizeof(vec4s));
            Magnefu_mesh.tangent_offset = offset;
            offset += tangents.size * sizeof(vec4s);

            memcpy(buffer->mapped_data + offset, normals.data, normals.size * sizeof(vec3s));
            Magnefu_mesh.normal_offset = offset;
            offset += normals.size * sizeof(vec3s);

            memcpy(buffer->mapped_data + offset, uv_coords.data, uv_coords.size * sizeof(vec2s));
            Magnefu_mesh.texcoord_offset = offset;

            creation.reset().set(flags, ResourceUsageType::Immutable, buffer_size).set_device_only(true).set_name(nullptr);
            br = renderer->gpu->create_buffer(creation);
            Magnefu_mesh.geometry_buffer_gpu = br;

            // TODO(marco): ideally the CPU buffer would be using staging memory and
            // freed after it has been copied!
            async_loader->request_buffer_copy(Magnefu_mesh.geometry_buffer_cpu, Magnefu_mesh.geometry_buffer_gpu, &Magnefu_mesh.uploads_completed);
            Magnefu_mesh.uploads_queued++;

            Magnefu_mesh.primitive_count = mesh->mNumFaces * 3;

            ObjMaterial& material = materials[mesh->mMaterialIndex];

            Magnefu_mesh.diffuse = material.diffuse;
            Magnefu_mesh.ambient = material.ambient;
            Magnefu_mesh.specular = material.ambient;
            Magnefu_mesh.specular_exp = material.specular_exp;

            Magnefu_mesh.diffuse_texture_index = material.diffuse_texture_index;
            Magnefu_mesh.normal_texture_index = material.normal_texture_index;

            Magnefu_mesh.transparency = material.transparency;

            creation.reset();
            creation.set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, ResourceUsageType::Dynamic, sizeof(ObjGpuData)).set_name("mesh_data");

            Magnefu_mesh.mesh_buffer = renderer->gpu->create_buffer(creation);

            positions.shutdown();
            normals.shutdown();
            uv_coords.shutdown();
            tangents.shutdown();
            indices.shutdown();
        }

        temp_allocator->freeToMarker(temp_allocator_initial_marker);

        i64 end_reading_buffers_data = time_now();

        i64 end_creating_buffers = time_now();

        i64 end_loading = time_now();

        MF_APP_INFO("Loaded scene {} in {} seconds.\nStats:\n\tReading GLTF file {} seconds\n\tTextures Creating {} seconds\n\tReading Buffers Data {} seconds\n\tCreating Buffers {} seconds", filename,
            time_delta_seconds(start_scene_loading, end_loading), time_delta_seconds(start_scene_loading, end_loading_file), time_delta_seconds(end_loading_file, end_creating_textures),
            time_delta_seconds(end_creating_textures, end_reading_buffers_data), time_delta_seconds(end_reading_buffers_data, end_creating_buffers));

        // We're done. Release all resources associated with this import
        aiReleaseImport(aiscene);
    }

    u32 ObjScene::load_texture(cstring texture_path, cstring path, StackAllocator* temp_allocator) {
        using namespace Magnefu;

        int comp, width, height;

        stbi_info(texture_path, &width, &height, &comp);

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

        TextureCreation tc;
        tc.set_data(nullptr).set_format_type(VK_FORMAT_R8G8B8A8_UNORM, TextureType::Texture2D).set_flags(mip_levels, 0).set_size((u16)width, (u16)height, 1).set_name(nullptr);
        TextureResource* tr = renderer->create_texture(tc);
        MF_ASSERT((tr != nullptr), "");

        images.push(*tr);

        renderer->gpu->link_texture_sampler(tr->handle, sampler->handle);

        StringBuffer name_buffer;
        name_buffer.init(4096, temp_allocator);

        // Reconstruct file path
        char* full_filename = name_buffer.append_use_f("%s%s", path, texture_path);
        async_loader->request_texture_data(full_filename, tr->handle);
        // Reset name buffer
        name_buffer.clear();

        return tr->handle.index;
    }



    void ObjScene::shutdown(Renderer* renderer) {
        // Free scene buffers
        images.shutdown();
    }

    void ObjScene::upload_materials() 
    {
        // Update per mesh material buffer
        for (u32 mesh_index = 0; mesh_index < mesh_draws.size; ++mesh_index) {
            ObjDraw& mesh_draw = mesh_draws[mesh_index];

            MapBufferParameters cb_map = { mesh_draw.mesh_buffer, 0, 0 };
            ObjGpuData* mesh_data = (ObjGpuData*)renderer->gpu->map_buffer(cb_map);
            if (mesh_data) {
                copy_gpu_material_data(*mesh_data, mesh_draw, 1.f);

                renderer->gpu->unmap_buffer(cb_map);
            }
        }
    }


    void ObjScene::submit_draw_task(ImGuiService* imgui, GPUProfiler* gpu_profiler, enki::TaskScheduler* task_scheduler) {
        ObjDrawTask draw_task;
        draw_task.init(task_scheduler, renderer->gpu, renderer, imgui, gpu_profiler, this, use_secondary_command_buffers);
        task_scheduler->AddTaskSetToPipe(&draw_task);
        task_scheduler->WaitforTaskSet(&draw_task);

        // Avoid using the same command buffer
        renderer->add_texture_update_commands((draw_task.thread_id + 1) % task_scheduler->GetNumTaskThreads());
    }

    void ObjScene::prepare_draws(Renderer* renderer, StackAllocator* scratch_allocator, SceneGraph* scene_graph)
    {

        using namespace Magnefu;

        // Create pipeline state
        PipelineCreation pipeline_creation;

        sizet cached_scratch_size = scratch_allocator->getMarker();

        StringBuffer path_buffer;
        path_buffer.init(1024, scratch_allocator);

        const char* vert_file = "phong.vert";
        char* vert_path = path_buffer.append_use_f("%s%s", MAGNEFU_SHADER_FOLDER, vert_file);
        FileReadResult vert_code = file_read_text(vert_path, scratch_allocator);

        const char* frag_file = "phong.frag";
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

        pipeline_creation.shaders.set_name("main").add_stage(vert_code.data, vert_code.size, VK_SHADER_STAGE_VERTEX_BIT).add_stage(frag_code.data, frag_code.size, VK_SHADER_STAGE_FRAGMENT_BIT);

        pipeline_creation.rasterization.cull_mode = VK_CULL_MODE_BACK_BIT;

        // Constant buffer
        BufferCreation buffer_creation;
        buffer_creation.reset().set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, ResourceUsageType::Dynamic, sizeof(GpuSceneData)).set_name("scene_cb");
        scene_cb = renderer->gpu->create_buffer(buffer_creation);

        pipeline_creation.name = "phong_opaque";
        GpuTechniqueCreation gtc;
        gtc.reset().add_pipeline(pipeline_creation);

        // Blend
        pipeline_creation.name = "phong_transparent";
        pipeline_creation.blend_state.add_blend_state().set_color(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD);

        GpuTechnique* technique = renderer->create_technique(gtc);

        MaterialCreation material_creation;

        material_creation.set_name("material_phong_opaque").set_technique(technique).set_render_index(0);
        Material* phong_material_opaque = renderer->create_material(material_creation);

        material_creation.set_name("material_phong_transparent").set_technique(technique).set_render_index(1);
        Material* phong_material_tranparent = renderer->create_material(material_creation);

        for (u32 mesh_index = 0; mesh_index < mesh_draws.size; ++mesh_index) {
            ObjDraw& mesh_draw = mesh_draws[mesh_index];

            if (mesh_draw.transparency == 1.0f) {
                mesh_draw.material = phong_material_opaque;
            }
            else {
                mesh_draw.material = phong_material_tranparent;
            }

            // Resource list
            DescriptorSetCreation ds_creation{};
            DescriptorSetLayoutHandle layout = renderer->gpu->get_descriptor_set_layout(mesh_draw.material->technique->passes[0].pipeline, k_material_descriptor_set_index);
            ds_creation.buffer(scene_cb, 0).buffer(mesh_draw.mesh_buffer, 1);
            mesh_draw.descriptor_set = renderer->gpu->create_descriptor_set(ds_creation);
        }

        qsort(mesh_draws.data, mesh_draws.size, sizeof(ObjDraw), obj_mesh_material_compare);
    }
    

    // -- Secondary Draw Task ----------------------------- //

    void SecondaryDrawTask::init(ObjScene* scene_, Renderer* renderer_, CommandBuffer* parent_, u32 start_, u32 end_)
    {
        renderer = renderer_;
        scene = scene_;
        parent = parent_;
        start = start_;
        end = end_;
    }

    void SecondaryDrawTask::ExecuteRange(enki::TaskSetPartition range_, uint32_t threadnum_)
    {

        cb = renderer->gpu->get_secondary_command_buffer(threadnum_);

        // TODO(marco): loop by material so that we can deal with multiple passes
        cb->begin_secondary(parent->current_render_pass, parent->current_framebuffer);

        cb->set_scissor(nullptr);
        cb->set_viewport(nullptr);

        Material* last_material = nullptr;
        for (u32 mesh_index = start; mesh_index < end; ++mesh_index) {
            ObjDraw& mesh_draw = scene->mesh_draws[mesh_index];

            if (mesh_draw.uploads_queued != mesh_draw.uploads_completed) {
                continue;
            }

            if (mesh_draw.material != last_material) {
                PipelineHandle pipeline = renderer->get_pipeline(mesh_draw.material, 0);

                cb->bind_pipeline(pipeline);

                last_material = mesh_draw.material;
            }

            scene->draw_mesh(cb, mesh_draw);
        }

        cb->end();
    }

    // -- ObjDrawTask ----------------------------- //

    void ObjDrawTask::init(enki::TaskScheduler* task_scheduler_, GraphicsContext* gpu_, Renderer* renderer_,
        ImGuiService* imgui_, GPUProfiler* gpu_profiler_,
        ObjScene* scene_, bool use_secondary_) {
        task_scheduler = task_scheduler_;
        gpu = gpu_;
        renderer = renderer_;
        imgui = imgui_;
        gpu_profiler = gpu_profiler_;
        scene = scene_;
        use_secondary = use_secondary_;
    }

    void ObjDrawTask::ExecuteRange(enki::TaskSetPartition range_, uint32_t threadnum_)
    {

        thread_id = threadnum_;

        //rprint( "Executing draw task from thread %u\n", threadnum_ );
        // TODO: improve getting a command buffer/pool
        CommandBuffer* gpu_commands = gpu->get_command_buffer(threadnum_, true);
        gpu_commands->push_marker("Frame");
       
        gpu_commands->clear(0.3f, 0.3f, 0.3f, 1.f);
        gpu_commands->clear_depth_stencil(1.0f, 0);
        gpu_commands->set_scissor(nullptr);
        gpu_commands->set_viewport(nullptr);
        
        
        gpu_commands->bind_pass(gpu->get_swapchain_pass(), gpu->get_current_framebuffer(), use_secondary);


        if (use_secondary) {
            static const u32 parallel_recordings = 4;
            u32 draws_per_secondary = scene->mesh_draws.size / parallel_recordings;
            u32 offset = draws_per_secondary * parallel_recordings;

            SecondaryDrawTask secondary_tasks[parallel_recordings]{ };

            u32 start = 0;
            for (u32 secondary_index = 0; secondary_index < parallel_recordings; ++secondary_index) {
                SecondaryDrawTask& task = secondary_tasks[secondary_index];

                task.init(scene, renderer, gpu_commands, start, start + draws_per_secondary);
                start += draws_per_secondary;

                task_scheduler->AddTaskSetToPipe(&task);
            }

            CommandBuffer* cb = renderer->gpu->get_secondary_command_buffer(threadnum_);

            cb->begin_secondary(gpu_commands->current_render_pass, gpu_commands->current_framebuffer);

            cb->set_scissor(nullptr);
            cb->set_viewport(nullptr);

            Material* last_material = nullptr;
            // TODO(marco): loop by material so that we can deal with multiple passes
            for (u32 mesh_index = offset; mesh_index < scene->mesh_draws.size; ++mesh_index) {
                ObjDraw& mesh_draw = scene->mesh_draws[mesh_index];

                if (mesh_draw.uploads_queued != mesh_draw.uploads_completed) {
                    continue;
                }

                if (mesh_draw.material != last_material) {
                    PipelineHandle pipeline = renderer->get_pipeline(mesh_draw.material, 0);

                    cb->bind_pipeline(pipeline);

                    last_material = mesh_draw.material;
                }

                scene->draw_mesh(cb, mesh_draw);
            }


            for (u32 secondary_index = 0; secondary_index < parallel_recordings; ++secondary_index) {
                SecondaryDrawTask& task = secondary_tasks[secondary_index];
                task_scheduler->WaitforTask(&task);

                vkCmdExecuteCommands(gpu_commands->vk_command_buffer, 1, &task.cb->vk_command_buffer);
            }

            // NOTE(marco): ImGui also has to use a secondary command buffer, vkCmdExecuteCommands is
            // the only allowed command. We don't need this if we use a different render pass above
            imgui->Render(*cb, true);

            cb->end();

            vkCmdExecuteCommands(gpu_commands->vk_command_buffer, 1, &cb->vk_command_buffer);

            gpu_commands->end_current_render_pass();
        }
        else {
            Material* last_material = nullptr;
            // TODO(marco): loop by material so that we can deal with multiple passes
            for (u32 mesh_index = 0; mesh_index < scene->mesh_draws.size; ++mesh_index) {
                ObjDraw& mesh_draw = scene->mesh_draws[mesh_index];

                if (mesh_draw.uploads_queued != mesh_draw.uploads_completed) {
                    continue;
                }

                if (mesh_draw.material != last_material) {
                    PipelineHandle pipeline = renderer->get_pipeline(mesh_draw.material, 0);

                    gpu_commands->bind_pipeline(pipeline);

                    last_material = mesh_draw.material;
                }

                scene->draw_mesh(gpu_commands, mesh_draw);
            }

            imgui->Render(*gpu_commands, false);
        }

        gpu_commands->pop_marker();

        gpu_profiler->update(*gpu);

        // Send commands to GPU
        gpu->queue_command_buffer(gpu_commands);
    }
}