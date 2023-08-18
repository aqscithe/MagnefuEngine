#pragma once
#include "Magnefu/Renderer/Light.h"

namespace Magnefu
{

	enum BufferUsage
	{
		USAGE_NONE = 0,
		USAGE_TRANSFER_SRC = 0x00000001,
        USAGE_TRANSFER_DST = 0x00000002,
        USAGE_UNIFORM_TEXEL = 0x00000004,
        USAGE_STORAGE_TEXEL = 0x00000008,
        USAGE_UNIFORM = 0x00000010,
        USAGE_STORAGE = 0x00000020,
        USAGE_INDEX = 0x00000040,
        USAGE_VERTEX = 0x00000080,
        USAGE_INDIRECT = 0x00000100,
        USAGE_SHADER_DEVICE_ADDRESS = 0x00020000,
        USAGE_VIDEO_DECODE_SRC_KHR = 0x00002000,
        USAGE_VIDEO_DECODE_DST_KHR = 0x00004000,
        USAGE_TRANSFORM_FEEDBACK_BUFFER_EXT = 0x00000800,
        USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_EXT = 0x00001000,
        USAGE_CONDITIONAL_RENDERING_EXT = 0x00000200,
        USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_KHR = 0x00080000,
        USAGE_ACCELERATION_STRUCTURE_STORAGE_KHR = 0x00100000,
        USAGE_SHADER_BINDING_TABLE_KHR = 0x00000400,
	};

    enum UniformBufferType
    {
        UNIFORM_NONE,
        UNIFORM_RENDERPASS,
        UNIFORM_MATERIAL,
        UNIFORM_SHADER
    };


    //struct alignas(16) RenderPassUniformBufferObject
    //{
    //    // Camera Info
    //    Maths::mat4 ViewMatrix;
    //    Maths::mat4 ProjMatrix;
    //    alignas(16) Maths::vec3 CameraPos;

    //    // Light Info
    //    alignas(16) Maths::vec3 LightPos;
    //    alignas(16) Maths::vec3 LightColor;
    //    float                   MaxLightDist;
    //    float                   RadiantFlux;
    //    int                     LightEnabled;
    //    int                     LightCount;
    //};

    struct alignas(16) RenderPassUniformBufferObject
    {
        // Camera Info
        Maths::mat4 ViewMatrix;
        Maths::mat4 ProjMatrix;
        Maths::vec3 CameraPos;
        float padding1;

        // Light Info
        PointLight PointLights[3];
        int   LightCount;
    };

    struct alignas(16) RenderPassUniformBufferObjectLTC
    {
        // Camera Info
        Maths::mat4 ViewMatrix;
        Maths::mat4 ProjMatrix;
        Maths::vec3 CameraPos;
    };

    /*struct alignas(16) LightUniformBufferObject
    {
        Maths::vec3 LightPos;
        Maths::vec3 LightColor;
        float MaxLightDist;
        float RadiantFlux;
        int LightCount;
        int LightEnabled;
    };*/


    struct alignas(16) MaterialUniformBufferObject
    {
        Maths::mat4 ModelMatrix;
        alignas(16) Maths::vec3 Tint;
        float                   Reflectance; // fresnel reflectance for dielectrics [0.0, 1.0]
        float                   Opacity;
        bool                    IsTextured;
    };


    struct ShaderUniformBufferObject
    {

    };


	struct BufferDesc
	{
		const char*         DebugName = nullptr;
        uint64_t            Offset = 0;
		uint64_t            ByteSize = 0;
        BufferUsage         Usage = BufferUsage::USAGE_NONE;
        UniformBufferType   UniformType = UniformBufferType::UNIFORM_NONE;
        Span<const uint8_t> InitData;
	};

    const BufferDesc RenderPassUniformBufferDesc = {
        "Renderpass Uniform Buffer",
        0,
        sizeof(RenderPassUniformBufferObject),
        BufferUsage::USAGE_UNIFORM,
        UniformBufferType::UNIFORM_RENDERPASS,
        {0}
    };

    const BufferDesc RenderPassUniformBufferDescLTC = {
        "Renderpass Uniform Buffer",
        0,
        sizeof(RenderPassUniformBufferObjectLTC),
        BufferUsage::USAGE_UNIFORM,
        UniformBufferType::UNIFORM_RENDERPASS,
        {0}
    };

    const BufferDesc MaterialUniformBufferDesc = {
        "Material Uniform Buffer",
        0,
        sizeof(MaterialUniformBufferObject),
        BufferUsage::USAGE_UNIFORM,
        UniformBufferType::UNIFORM_MATERIAL,
        {0}
    };

    

    class Buffer
    {
    public:
        Buffer(const BufferDesc& desc);
        virtual ~Buffer() = default;


    };


    class BufferFactory
    {
    public:
        static Buffer* CreateBuffer(const BufferDesc& desc);

    };

    
}