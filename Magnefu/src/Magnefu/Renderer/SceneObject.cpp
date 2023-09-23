#include "mfpch.h"
#include "SceneObject.h"
#include "Magnefu/Application.h"
#include "GraphicsContext.h"
#include "Magnefu/ResourceManagement/ResourceManager.h"



namespace Magnefu
{
    SceneObject::~SceneObject()
    {

    }

    void SceneObject::Init(uint32_t index, ModelType modelType)
	{
        ResourceInfo resourceInfo;

        switch (modelType)
        {
            case Magnefu::MODEL_DEFAULT:
            {
                resourceInfo = RESOURCE_PATHS[index];
                break;
            }
            default:
            {
                MF_CORE_ASSERT(false, "Invalid or Unknown Model Type -- SceneObject::Init");
                break;
            }
        }
        Application& app = Application::Get();
        ResourceManager& rm = app.GetResourceManager();
        GraphicsContext* context = app.GetWindow().GetGraphicsContext();

        

        m_Material = rm.CreateBindGroup({
            .DebugName = "SciFi Corridor",
            .LayoutType = BindingLayoutType::LAYOUT_MATERIAL_DEFAULT,
            .Layout = DEFAULT_MATERIAL_BINDING_LAYOUT,
            .IsTextured = resourceInfo.IsTextured,
            .Textures = {
                .Diffuse = {
                    "DiffuseTexture",
                    index,
                    TextureType::DIFFUSE,
                    TextureTiling::IMAGE_TILING_OPTIMAL,
                    TextureFormat::FORMAT_R8G8B8A8_SRGB,
                    //TextureChannels::CHANNELS_RGB_ALPHA,
                },
                .ARM = {
                    "ARMTexture",
                    index,
                    TextureType::ARM,
                    TextureTiling::IMAGE_TILING_OPTIMAL,
                    TextureFormat::FORMAT_R8G8B8A8_UNORM,
                    //TextureChannels::CHANNELS_RGB_ALPHA,
                },
                .Normal = {
                    "NormalTexture",
                    index,
                    TextureType::NORMAL,
                    TextureTiling::IMAGE_TILING_OPTIMAL,
                    TextureFormat::FORMAT_R8G8B8A8_UNORM,
                    //TextureChannels::CHANNELS_RGB_ALPHA
                }
            },
            .Buffers = MaterialUniformBufferDesc
        });

        m_GraphicsPipelineShader = rm.CreateShader({   // shader will be set by the object. Ex: drawStream.SetShader(sceneObject.shader);
            .DebugName = "Basic Shader",
            .Path = resourceInfo.ShaderPath,
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
                    .Enabled = true,
                    .Stages = ShaderStage::SHADER_STAGE_FRAGMENT,
                    .Offset = 0,
                    .ByteSize = sizeof(PushConstants)
                }
            }
        });

        m_VertexBuffer = rm.CreateBuffer({
            .DebugName = "VertexBuffer",
            .Offset = context->GetVBufferOffset(index),
            .ByteSize = static_cast<uint64_t>(m_Vertices.data.size()),
            .Usage = BufferUsage::USAGE_VERTEX,
            .UniformType = UniformBufferType::UNIFORM_NONE,
            .InitData = m_Vertices.span
            });

        m_IndexBuffer = rm.CreateBuffer({
            .DebugName = "IndexBuffer",
            .Offset = context->GetIBufferOffset(index),
            .ByteSize = static_cast<uint64_t>(m_Indices.data.size()),
            .Usage = BufferUsage::USAGE_INDEX,
            .UniformType = UniformBufferType::UNIFORM_NONE,
            .InitData = m_Indices.span
            });
	}

    TextureDataBlock& SceneObject::GetTextureData(TextureType type)
    {
        switch (type)
        {
            /*case Magnefu::NONE:
            {
                MF_CORE_ASSERT(false, "Texture type incorrectly set to none - SetTextureBlock");
                return;
            }*/
            case Magnefu::DIFFUSE:
            {
                return m_DiffuseTextureBlock;
            }
            case Magnefu::ARM:
            {
                return m_ARMTextureBlock;
            }
            case Magnefu::NORMAL:
            {
                return m_NormalTextureBlock;
            }
            /*case Magnefu::EMISSIVE:
            {
                break;
            }
            case Magnefu::DISPLACEMENT:
            {
                break;
            }*/
            /*default:
            {
                MF_CORE_ASSERT(false, "Unknown Texture type - SetTextureBlock");
                break;
            }*/
        }
    }

    void SceneObject::SetTextureBlock(TextureType type, DataBlock&& dataBlock, int width, int height, int channels)
    {
        switch (type)
        {
            case Magnefu::NONE:
            {
                MF_CORE_ASSERT(false, "Texture type incorrectly set to none - SetTextureBlock");
                break;
            }
            case Magnefu::DIFFUSE:
            {
                m_DiffuseTextureBlock.Pixels = std::move(dataBlock);
                m_DiffuseTextureBlock.Dimensions.Width = width;
                m_DiffuseTextureBlock.Dimensions.Height = height;
                m_DiffuseTextureBlock.Dimensions.Channels = channels;
                break;
            }
            case Magnefu::ARM:
            {
                m_ARMTextureBlock.Pixels = std::move(dataBlock);
                m_ARMTextureBlock.Dimensions.Width = width;
                m_ARMTextureBlock.Dimensions.Height = height;
                m_ARMTextureBlock.Dimensions.Channels = channels;
                break;
            }
            case Magnefu::NORMAL:
            {
                m_NormalTextureBlock.Pixels = std::move(dataBlock);
                m_NormalTextureBlock.Dimensions.Width = width;
                m_NormalTextureBlock.Dimensions.Height = height;
                m_NormalTextureBlock.Dimensions.Channels = channels;
                break;
            }
            /*case Magnefu::EMISSIVE:
            {
                break;
            }
            case Magnefu::DISPLACEMENT:
            {
                break;
            }*/
            default:
            {
                MF_CORE_ASSERT(false, "Unknown Texture type - SetTextureBlock");
                break;
            }
        }
    }
}

