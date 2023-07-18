#include "mfpch.h"
#include "VulkanBindGroup.h"
#include "VulkanContext.h"
#include "VulkanTexture.h"
#include "Magnefu/Application.h"
#include "Magnefu/Renderer/RenderConstants.h"


namespace Magnefu
{
	VulkanBindGroup::VulkanBindGroup(const BindGroupDesc& desc) : BindGroup(desc)
	{
		// do a switch on bind group type to determine which functions to run
		// render pass globals, material or shader specific bindings

		switch (desc.LayoutType)
		{
			case BindingLayoutType::LAYOUT_RENDERPASS:
			{
				CreateDescriptorSetLayout(desc.Layout);
				CreateBindingBuffers(desc.Buffers);
				CreateDescriptorPool(desc.LayoutType);
				CreateDescriptorSets(desc.LayoutType);
				break;
			}
			case BindingLayoutType::LAYOUT_MATERIAL:
			{
				CreateDescriptorSetLayout(desc.Layout);
				CreateBindingBuffers(desc.Buffers);
				CreateBindingTextures(desc.Textures);
				CreateDescriptorPool(desc.LayoutType);
				CreateDescriptorSets(desc.LayoutType);
				break;
			}
			default:
				break;
		}

		
	}

	VulkanBindGroup::~VulkanBindGroup()
	{
		VkDevice device = VulkanContext::Get().GetDevice();

		vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr);
	}

	void VulkanBindGroup::CreateDescriptorSetLayout(const BindingLayout& layout)
	{
		int BindingCount = layout.Bindings.size();
		//MF_CORE_ASSERT( BindingCount == 4, "Incorrect material binding layout size");

		std::vector<VkDescriptorSetLayoutBinding> layoutBindings(layout.Bindings.size());
		

		for (size_t i = 0; i < layoutBindings.size(); i++)
		{
			layoutBindings[i].binding = layout.Bindings[i].BindingPos;
			layoutBindings[i].descriptorType = GetDescriptorType(layout.Bindings[i].Type);
			layoutBindings[i].descriptorCount = layout.Bindings[i].Count;
			layoutBindings[i].pImmutableSamplers = nullptr; // For image sampling
			layoutBindings[i].stageFlags = GetShaderStageFlags(layout.Bindings[i].Stage);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
		layoutInfo.pBindings = layoutBindings.data();

		if (vkCreateDescriptorSetLayout(VulkanContext::Get().GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create descriptor set layout!");
	}

	void VulkanBindGroup::CreateBindingBuffers(const BindingBufferDescs& descriptions)
	{
		m_Uniforms = Application::Get().GetResourceManager().CreateBuffer(descriptions.Uniforms);
	}

	void VulkanBindGroup::CreateBindingTextures(const BindingTextureDescs& descriptions)
	{
		ResourceManager& rm = Application::Get().GetResourceManager();
		m_DiffuseTexture = rm.CreateTexture(descriptions.Diffuse);
		m_ARMTexture = rm.CreateTexture(descriptions.ARM);
		m_NormalTexture = rm.CreateTexture(descriptions.Normal);
	}

	void VulkanBindGroup::CreateDescriptorPool(const BindingLayoutType& type)
	{
		switch (type)
		{
			case BindingLayoutType::LAYOUT_MATERIAL:
			{
				VkDescriptorPoolSize uboPoolSize{};
				uboPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				uboPoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

				VkDescriptorPoolSize samplerPoolSize{};
				samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				samplerPoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

				std::array<VkDescriptorPoolSize, 2> poolSizes{ uboPoolSize, samplerPoolSize };

				VkDescriptorPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
				poolInfo.pPoolSizes = poolSizes.data();
				poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
				poolInfo.flags = 0; // VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT - determines if individual descriptor sets can be freed

				if (vkCreateDescriptorPool(VulkanContext::Get().GetDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
					MF_CORE_ASSERT(false, "failed to create descriptor pool!");

				break;
			}

			case BindingLayoutType::LAYOUT_RENDERPASS:
			{
				VkDescriptorPoolSize uboPoolSize{};
				uboPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				uboPoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

				std::array<VkDescriptorPoolSize, 1> poolSizes{ uboPoolSize };

				VkDescriptorPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
				poolInfo.pPoolSizes = poolSizes.data();
				poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
				poolInfo.flags = 0; // VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT - determines if individual descriptor sets can be freed

				if (vkCreateDescriptorPool(VulkanContext::Get().GetDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
					MF_CORE_ASSERT(false, "failed to create descriptor pool!");

				break;

			}

			default:
				break;
		}
		
	}

	void VulkanBindGroup::CreateDescriptorSets(const BindingLayoutType& type)
	{
		Application& app = Application::Get();
		ResourceManager& rm = app.GetResourceManager();
		VkDevice device = VulkanContext::Get().GetDevice();
		VulkanUniformBuffer& uniformBuffer = static_cast<VulkanUniformBuffer&>(rm.GetBuffer(m_Uniforms));

		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device, &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to allocate descriptor sets!");

		switch (type)
		{
			case BindingLayoutType::LAYOUT_MATERIAL:
			{
				std::vector<VkDescriptorImageInfo> imageInfo{};
				imageInfo.resize(sizeof(BindingTextureDescs) / sizeof(TextureDesc));

				MF_CORE_ASSERT(imageInfo.size() == 3, "Descriptor set texture image info count doesn't match texture handle count.");

				VkSampler& sampler = VulkanTexture::GetSampler();


				for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
				{
					// UBO
					VkDescriptorBufferInfo bufferInfo{};
					bufferInfo.buffer = uniformBuffer.GetBuffers()[i];
					bufferInfo.offset = 0;
					bufferInfo.range = sizeof(MaterialUniformBufferObject);

					// Diffuse
					imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo[0].imageView = static_cast<VulkanTexture&>(rm.GetTexture(m_DiffuseTexture)).GetImageView();
					imageInfo[0].sampler = sampler;

					// ARM
					imageInfo[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo[1].imageView = static_cast<VulkanTexture&>(rm.GetTexture(m_ARMTexture)).GetImageView();
					imageInfo[1].sampler = sampler;

					// Normal
					imageInfo[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo[2].imageView = static_cast<VulkanTexture&>(rm.GetTexture(m_NormalTexture)).GetImageView();
					imageInfo[2].sampler = sampler;


					std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

					// UBO
					descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrites[0].dstSet = m_DescriptorSets[i];
					descriptorWrites[0].dstBinding = 0;
					descriptorWrites[0].dstArrayElement = 0;
					descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					descriptorWrites[0].descriptorCount = 1;
					descriptorWrites[0].pBufferInfo = &bufferInfo;
					descriptorWrites[0].pImageInfo = nullptr; // Optional
					descriptorWrites[0].pTexelBufferView = nullptr; // Optional

					// Diffuse Texture
					descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrites[1].dstSet = m_DescriptorSets[i];
					descriptorWrites[1].dstBinding = 1;
					descriptorWrites[1].dstArrayElement = 0;
					descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorWrites[1].descriptorCount = 1;
					descriptorWrites[1].pBufferInfo = nullptr;
					descriptorWrites[1].pImageInfo = &imageInfo[0];
					descriptorWrites[1].pTexelBufferView = nullptr; // Optional

					// ARM Texture
					descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrites[2].dstSet = m_DescriptorSets[i];
					descriptorWrites[2].dstBinding = 2;
					descriptorWrites[2].dstArrayElement = 0;
					descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorWrites[2].descriptorCount = 1;
					descriptorWrites[2].pBufferInfo = nullptr;
					descriptorWrites[2].pImageInfo = &imageInfo[1];
					descriptorWrites[2].pTexelBufferView = nullptr; // Optional

					// Normal Texture
					descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrites[3].dstSet = m_DescriptorSets[i];
					descriptorWrites[3].dstBinding = 3;
					descriptorWrites[3].dstArrayElement = 0;
					descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorWrites[3].descriptorCount = 1;
					descriptorWrites[3].pBufferInfo = nullptr;
					descriptorWrites[3].pImageInfo = &imageInfo[2];
					descriptorWrites[3].pTexelBufferView = nullptr; // Optional

					vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

				}
				break;
			}

			case BindingLayoutType::LAYOUT_RENDERPASS:
			{
				for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
				{
					// UBO
					VkDescriptorBufferInfo bufferInfo{};
					bufferInfo.buffer = uniformBuffer.GetBuffers()[i];
					bufferInfo.offset = 0;
					bufferInfo.range = sizeof(RenderPassUniformBufferObject); 

					std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

					// UBO
					descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrites[0].dstSet = m_DescriptorSets[i];
					descriptorWrites[0].dstBinding = 0;
					descriptorWrites[0].dstArrayElement = 0;
					descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					descriptorWrites[0].descriptorCount = 1;
					descriptorWrites[0].pBufferInfo = &bufferInfo;
					descriptorWrites[0].pImageInfo = nullptr; // Optional
					descriptorWrites[0].pTexelBufferView = nullptr; // Optional

					vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
				}
				break;
			}

			default:
				break;
		}

	}

	VkDescriptorType VulkanBindGroup::GetDescriptorType(const BindingType& type)
	{
		switch (type)
		{
			case BindingType::BINDING_TYPE_UNIFORM_BUFFER:
			{
				return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			}
			case BindingType::BINDING_TYPE_COMBINED_IMAGE_SAMPLER:
			{
				return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			}
			default:
			{
				MF_CORE_ASSERT(false, "Unknown Descriptor Type");
				break;
			}
				
		}
	}

	VkShaderStageFlags VulkanBindGroup::GetShaderStageFlags(const ShaderStage& stage)
	{
		switch (stage)
		{
			case ShaderStage::SHADER_STAGE_VERTEX:
			{
				return VK_SHADER_STAGE_VERTEX_BIT;
			}

			case ShaderStage::SHADER_STAGE_FRAGMENT:
			{
				return VK_SHADER_STAGE_FRAGMENT_BIT;
			}
			default:
			{
				MF_CORE_ASSERT(false, "Unknown Shader Stage Flag");
				break;
			}
			
		}
	}
}
