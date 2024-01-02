#pragma once

#include "Magnefu/Graphics/GPUResources.hpp"

#include "cglm/struct/mat3.h"
#include "cglm/struct/mat4.h"
#include "cglm/struct/quat.h"
#include "cglm/struct/cam.h"
#include "cglm/struct/affine.h"



enum MaterialFeatures 
{
    MaterialFeatures_ColorTexture = 1 << 0,
    MaterialFeatures_NormalTexture = 1 << 1,
    MaterialFeatures_RoughnessTexture = 1 << 2,
    MaterialFeatures_OcclusionTexture = 1 << 3,
    MaterialFeatures_EmissiveTexture = 1 << 4,

    MaterialFeatures_TangentVertexAttribute = 1 << 5,
    MaterialFeatures_TexcoordVertexAttribute = 1 << 6,
};

struct alignas(16) MaterialData 
{
    vec4s base_color_factor;
    mat4s model;
    mat4s model_inv;

    vec3s emissive_factor;
    f32   metallic_factor;

    f32   roughness_factor;
    f32   occlusion_factor;
    u32   flags;
};

struct MeshDraw 
{
    Magnefu::BufferHandle index_buffer;
    Magnefu::BufferHandle position_buffer;
    Magnefu::BufferHandle tangent_buffer;
    Magnefu::BufferHandle normal_buffer;
    Magnefu::BufferHandle texcoord_buffer;

    Magnefu::BufferHandle material_buffer;
    MaterialData         material_data;

    u32 index_offset;
    u32 position_offset;
    u32 tangent_offset;
    u32 normal_offset;
    u32 texcoord_offset;

    u32 count;

    VkIndexType index_type;

    Magnefu::DescriptorSetHandle descriptor_set;
};


struct UniformData 
{
    mat4s m;
    mat4s vp;
    vec4s eye;
    vec4s light;
};


struct Transform 
{

    vec3s                   scale;
    versors                 rotation;
    vec3s                   translation;

    void                    reset();

    mat4s                   calculate_matrix() const 
    {
        const mat4s translation_matrix = glms_translate_make(translation);
        const mat4s scale_matrix = glms_scale_make(scale);
        const mat4s local_matrix = glms_mat4_mul(glms_mat4_mul(translation_matrix, glms_quat_mat4(rotation)), scale_matrix);
        return local_matrix;
    }
}; // struct Transform