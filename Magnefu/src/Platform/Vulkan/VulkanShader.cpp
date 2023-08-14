#include "mfpch.h"
#include "VulkanShader.h"
#include "Magnefu/Application.h"
#include "VulkanContext.h"
#include "VulkanBindGroup.h"

#include "shaderc/shaderc.hpp"

namespace Magnefu
{
	VulkanShader::VulkanShader(const ShaderDesc& desc) : Shader(desc)
	{
		// Switch on pipeline type : graphics or compute

		CreateGraphicsPipeline(desc);

		//CreateComputePipeline(desc);

	}

	VulkanShader::~VulkanShader()
	{
		VkDevice device = VulkanContext::Get().GetDevice();
		vkDestroyPipeline(device, m_Pipeline, nullptr);
		vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
	}

	void VulkanShader::CreateGraphicsPipeline(const ShaderDesc& desc)
	{
		Application& app = Application::Get();
		ResourceManager& rm = Application::Get().GetResourceManager();
		VulkanContext& context = VulkanContext::Get();

		// Shader Modules
		ShaderList shaderList = ParseShader(desc.Path);
		VkShaderModule vertShaderModule = CreateShaderModule(shaderList.Vertex);
		VkShaderModule fragShaderModule = CreateShaderModule(shaderList.Fragment);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VulkanCommon::GetShaderStageFlagBits(desc.StageDescriptions.VS.Stage);
		vertShaderStageInfo.module = vertShaderModule;

		// shader entrypoint - for fragment shaders, this means it is possible to include several shaders(of the same type) in a single shader module and 
		// use different entry points to differentiate between their behaviors
		vertShaderStageInfo.pName = desc.StageDescriptions.VS.EntryPoint;
		vertShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VulkanCommon::GetShaderStageFlagBits(desc.StageDescriptions.FS.Stage);
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = desc.StageDescriptions.FS.EntryPoint;
		fragShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// Vertex Input
		auto bindingDescription = Vertex::GetBindingDescription();
		auto attributeDescriptions = Vertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional

		// Input Assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;  // TODO: should be a parameter in create shader function
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// Viewport & Scissors
		VkExtent2D swapChainExtent = context.GetSwapChainExtent();

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChainExtent.width;
		viewport.height = (float)swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;

		// Dynamic State
		std::vector<VkDynamicState> dynamicStates = {
			TranslateDynamicState(desc.GraphicsPipeline.DynamicStates[0]), // TODO: change the way these are intialized, i shouldn't have to add a line here everytime 
			TranslateDynamicState(desc.GraphicsPipeline.DynamicStates[1]), // is a new dynamic state
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = desc.GraphicsPipeline.ViewportInfo.ViewportCount;
		viewportState.scissorCount = desc.GraphicsPipeline.ViewportInfo.ScissorCount;

		// Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = TranslatePolygonMode(desc.GraphicsPipeline.RasterizerInfo.PolygonMode);  
		rasterizer.lineWidth = desc.GraphicsPipeline.RasterizerInfo.LineWidth;
		rasterizer.cullMode = TranslateCullMode(desc.GraphicsPipeline.RasterizerInfo.CullMode);
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; 
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		// MSAA
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = desc.GraphicsPipeline.MSAAInfo.EnableSampleShading;
		multisampling.rasterizationSamples = context.GetMSAASamples();
		multisampling.minSampleShading = desc.GraphicsPipeline.MSAAInfo.MinSampleShading; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		// Depth & Stencil Testing
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = desc.GraphicsPipeline.DepthAndStencilInfo.DepthTestEnable;
		depthStencil.depthWriteEnable = desc.GraphicsPipeline.DepthAndStencilInfo.DepthWriteEnable;
		depthStencil.depthCompareOp = TranslateDepthCompareOp(desc.GraphicsPipeline.DepthAndStencilInfo.CompareOp);
		depthStencil.depthBoundsTestEnable = desc.GraphicsPipeline.DepthAndStencilInfo.DepthBoundsTestEnable;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = desc.GraphicsPipeline.DepthAndStencilInfo.StencilTestEnable;
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional

		// Color blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		// Push Constants
		VkPushConstantRange pushConstantRange = {};
		if (desc.GraphicsPipeline.PushConstantInfo.Enabled)
		{
			pushConstantRange.stageFlags = VulkanCommon::GetShaderStageFlags(desc.GraphicsPipeline.PushConstantInfo.Stages); // The shader stages that will use the push constants.
			pushConstantRange.offset = desc.GraphicsPipeline.PushConstantInfo.Offset;
			pushConstantRange.size = desc.GraphicsPipeline.PushConstantInfo.ByteSize;
			MF_CORE_DEBUG("Size of Push Constants: {}", sizeof(PushConstants));
		}

		VulkanBindGroup& renderpassGlobals = static_cast<VulkanBindGroup&>(rm.GetBindGroup(desc.BindGroups[0]));
		VulkanBindGroup& material = static_cast<VulkanBindGroup&>(rm.GetBindGroup(desc.BindGroups[1]));

		std::vector<VkDescriptorSetLayout> layouts = {
			renderpassGlobals.GetDescriptorSetLayout(),
			material.GetDescriptorSetLayout()
		};

		// Pipeline Layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pushConstantRangeCount = desc.GraphicsPipeline.PushConstantInfo.Enabled ? 1 : 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = desc.GraphicsPipeline.PushConstantInfo.Enabled ? &pushConstantRange : nullptr; // Optional
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();

		VkDevice device = context.GetDevice();
		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to create pipeline layout!");


		// Pipeline struct
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;

		pipelineInfo.layout = m_PipelineLayout;

		pipelineInfo.renderPass = context.GetRenderPass();
		pipelineInfo.subpass = 0;

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to create graphics pipeline");

		vkDestroyShaderModule(device, vertShaderModule, nullptr);
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
	}

	ShaderList VulkanShader::ParseShader(const char* filepath)
	{
		std::ifstream stream(filepath);

		if (!stream.good())
			MF_CORE_ASSERT(false, "Failed to load shader contents");

		String line;

		std::stringstream ss[3];
		ShaderType type = ShaderType::None;
		while (std::getline(stream, line))
		{
			if (line.find("#shader") != String::npos)
			{
				if (line.find("vertex") != String::npos)
				{
					type = ShaderType::Vertex;
				}
				else if (line.find("fragment") != String::npos)
				{
					type = ShaderType::Fragment;
				}
				else if (line.find("compute") != String::npos)
				{
					type = ShaderType::Compute;
				}
			}
			else
			{
				ss[(int)type] << line << '\n';
			}
		}

		return {
			{ ss[0].str(), ShaderType::Vertex },
			{ ss[1].str(), ShaderType::Fragment },
			{ ss[2].str(), ShaderType::Compute }
		};
	}

	VkShaderModule VulkanShader::CreateShaderModule(const ShaderSource& source)
	{
		// Compile shader to spv binary
		// Create an instance of the compiler
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		// Compile the shader code
		shaderc::SpvCompilationResult result;

		switch (source.Type)
		{
		case Magnefu::ShaderType::Vertex:
		{
			result = compiler.CompileGlslToSpv(source.Text, shaderc_vertex_shader, "vertex.glsl", options);
			break;
		}
		case Magnefu::ShaderType::Fragment:
		{
			result = compiler.CompileGlslToSpv(source.Text, shaderc_fragment_shader, "fragment.glsl", options);
			break;
		}
		case Magnefu::ShaderType::Compute:
		{
			result = compiler.CompileGlslToSpv(source.Text, shaderc_compute_shader, "compute.glsl", options);
			break;
		}
		default:
		{
			MF_CORE_ASSERT(false, "Unknown shader type: {}", static_cast<int>(source.Type));
			break;
		}
		}


		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			// Compilation failed, handle the error
			MF_CORE_ASSERT(false, "Error: Compilation failed: {} ", result.GetErrorMessage());
		}

		// The result object is an iterable object providing a begin() and end() iterator for the SPIR-V binary.
		std::vector<uint32_t> spirv_binary(result.cbegin(), result.cend());

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = spirv_binary.size() * sizeof(uint32_t);
		createInfo.pCode = spirv_binary.data();

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(VulkanContext::Get().GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to create a shader module");
	

		return shaderModule;
	}

	VkDynamicState VulkanShader::TranslateDynamicState(const DynamicState& state)
	{
		switch (state)
		{
		case DynamicState::DYNAMIC_STATE_VIEWPORT:
		{
			return VK_DYNAMIC_STATE_VIEWPORT;
		}
		case DynamicState::DYNAMIC_STATE_SCISSOR:
		{
			return VK_DYNAMIC_STATE_SCISSOR;
		}
		default:
		{
			MF_CORE_ASSERT(false, "Unknown Dynamic State");
			break;
		}
		}
	}

	VkPolygonMode VulkanShader::TranslatePolygonMode(const PolygonMode& mode)
	{
		bool fillModeNonsolid = VulkanContext::Get().GetEnabledFeatures().fillModeNonSolid;

		switch (mode)
		{
			case PolygonMode::POLYGON_MODE_FILL:
			{
				return VK_POLYGON_MODE_FILL;
			}

			case PolygonMode::POLYGON_MODE_LINE:
			{
				return fillModeNonsolid ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
			}

			case PolygonMode::POLYGON_MODE_POINT:
			{
				return fillModeNonsolid ? VK_POLYGON_MODE_POINT : VK_POLYGON_MODE_FILL;
			}

			default:
			{
				MF_CORE_ASSERT(false, "Unknown polygon mode");
				break;
			}
		}
	}

	VkCullModeFlags VulkanShader::TranslateCullMode(const CullMode& mode)
	{
		switch (mode)
		{
			case CullMode::CULL_MODE_BACK:
			{
				return VK_CULL_MODE_BACK_BIT;
			}

			case CullMode::CULL_MODE_FRONT:
			{
				return VK_CULL_MODE_FRONT_BIT;
			}

			case CullMode::CULL_MODE_FRONT_AND_BACK:
			{
				return VK_CULL_MODE_FRONT_AND_BACK;
			}

			case CullMode::CULL_MODE_NONE:
			{
				return VK_CULL_MODE_NONE;
			}

			default:
			{
				MF_CORE_ASSERT(false, "Unknown Cull Mode");
				break;
			}
		}
	}

	VkCompareOp VulkanShader::TranslateDepthCompareOp(const DepthCompareOp& compareOp)
	{
		switch (compareOp)
		{
			case DepthCompareOp::COMPARE_OP_NEVER:
			{
				return VK_COMPARE_OP_NEVER;
			}

			case DepthCompareOp::COMPARE_OP_EQUAL:
			{
				return VK_COMPARE_OP_EQUAL;
			}

			case DepthCompareOp::COMPARE_OP_LESS:
			{
				return VK_COMPARE_OP_LESS;
			}

			case DepthCompareOp::COMPARE_OP_LESS_OR_EQUAL:
			{
				return VK_COMPARE_OP_LESS_OR_EQUAL;
			}

			case DepthCompareOp::COMPARE_OP_GREATER:
			{
				return VK_COMPARE_OP_GREATER;
			}

			case DepthCompareOp::COMPARE_OP_GREATER_OR_EQUAL:
			{
				return VK_COMPARE_OP_GREATER_OR_EQUAL;
			}

			case DepthCompareOp::COMPARE_OP_NOT_EQUAL:
			{
				return VK_COMPARE_OP_NOT_EQUAL;
			}

			default:
			{
				MF_CORE_ASSERT(false, "Unrecognized Depth Comparison Operation");
				break;
			}
		}
	}
}