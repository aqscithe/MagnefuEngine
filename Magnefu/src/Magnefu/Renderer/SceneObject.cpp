#include "mfpch.h"
#include "SceneObject.h"
#include "Magnefu/Application.h"
#include "Magnefu/ResourceManagement/ResourceManager.h"
#include "Magnefu/ResourceManagement/ResourcePaths.h"


namespace Magnefu
{
    SceneObject::~SceneObject()
    {

    }

    void SceneObject::Init(uint32_t index)
	{
        Application& app = Application::Get();
        ResourceManager& rm = app.GetResourceManager();

        m_Material = rm.CreateBindGroup({
            .DebugName = "SciFi Corridor",
            .LayoutType = BindingLayoutType::LAYOUT_MATERIAL,
            .Layout = DEFAULT_MATERIAL_BINDING_LAYOUT,
            .Textures = {
                .Diffuse = {
                    "DiffuseTexture",
                    index,
                    TextureType::DIFFUSE,
                    TextureTiling::IMAGE_TILING_OPTIMAL,
                    TextureFormat::FORMAT_R8G8B8A8_SRGB,
                    TextureChannels::CHANNELS_RGB_ALPHA,
                },
                .ARM = {
                    "ARMTexture",
                    index,
                    TextureType::ARM,
                    TextureTiling::IMAGE_TILING_OPTIMAL,
                    TextureFormat::FORMAT_R8G8B8A8_UNORM,
                    TextureChannels::CHANNELS_RGB_ALPHA,
                },
                .Normal = {
                    "NormalTexture",
                    index,
                    TextureType::NORMAL,
                    TextureTiling::IMAGE_TILING_OPTIMAL,
                    TextureFormat::FORMAT_R8G8B8A8_UNORM,
                    TextureChannels::CHANNELS_RGB_ALPHA
                }
            },
            .Buffers = MaterialUniformBufferDesc
        });

        m_GraphicsPipelineShader = rm.CreateShader({   // shader will be set by the object. Ex: drawStream.SetShader(sceneObject.shader);
            .DebugName = "Basic Shader",
            .Path = SHADER_PATH,
            .StageDescriptions = {
                .VS = DefaultVertexShaderDesc,
                .FS = DefaultFragmentShaderDesc
            },
            .BindGroups = {
                app.GetRenderPassBindGroup(),
                m_Material
            },
            .GraphicsPipeline = {
                .DynamicStates = {
                    DynamicState::DYNAMIC_STATE_VIEWPORT,
                    DynamicState::DYNAMIC_STATE_SCISSOR
                },
                .ViewportInfo = {
                    .ViewportCount = 1,
                    .ScissorCount = 1
                },
                .RasterizerInfo = {
                    .PolygonMode = PolygonMode::POLYGON_MODE_FILL,
                    .LineWidth = 1.f,
                    .CullMode = CullMode::CULL_MODE_BACK,
                },
                .MSAAInfo = {
                //.SampleCount         = MSAASampleCountFlag::SAMPLE_COUNT_8_BIT, // Should be a parameter for when I create the graphics context as that is when this value is found
                .EnableSampleShading = false,
                .MinSampleShading = 1.0f
                },
                .DepthAndStencilInfo = {
                    .CompareOp = DepthCompareOp::COMPARE_OP_LESS,
                    .DepthTestEnable = true,
                    .DepthWriteEnable = true,
                    .DepthBoundsTestEnable = false,
                    .StencilTestEnable = false
                },
                .PushConstantInfo = {
                    .Enabled = false,
                    .Stages = ShaderStage::SHADER_STAGE_VERTEX_AND_FRAGMENT,
                    .Offset = 0,
                    .ByteSize = 0
                }
            }
        });

        m_VertexBuffer = rm.CreateBuffer({
            .DebugName = "VertexBuffer",
            .ByteSize = static_cast<uint64_t>(m_Vertices.data.size()),
            .Usage = BufferUsage::USAGE_VERTEX,
            .UniformType = UniformBufferType::UNIFORM_NONE,
            .InitData = m_Vertices.span
            });

        m_IndexBuffer = rm.CreateBuffer({
            .DebugName = "IndexBuffer",
            .ByteSize = static_cast<uint64_t>(m_Indices.data.size()),
            .Usage = BufferUsage::USAGE_INDEX,
            .UniformType = UniformBufferType::UNIFORM_NONE,
            .InitData = m_Indices.span
            });
	}
}

