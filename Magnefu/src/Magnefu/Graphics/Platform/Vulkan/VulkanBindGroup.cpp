// -- PCH --- //
#include "mfpch.h"

// -- header -- //
#include "VulkanBindGroup.h"

// -- Graphics includes -- //
#include "VulkanContext.h"
#include "VulkanTexture.h"
#include "Magnefu/Graphics/RenderConstants.h"



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
				CreateBindingTextures(desc.Textures);
				CreateDescriptorPool(desc.LayoutType, true);
				CreateDescriptorSets(desc.LayoutType, true);
				break;
			}
			case BindingLayoutType::LAYOUT_MATERIAL_DEFAULT:
			{
				CreateDescriptorSetLayout(desc.Layout);
				CreateBindingBuffers(desc.Buffers);

				if(desc.IsTextured)
					CreateBindingTextures(desc.Textures);

				CreateDescriptorPool(desc.LayoutType, desc.IsTextured);
				CreateDescriptorSets(desc.LayoutType, desc.IsTextured);
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

		std::vector<VkDescriptorSetLayoutBinding> layoutBindings(layout.Bindings.size());
		

		for (size_t i = 0; i < layoutBindings.size(); i++)
		{
			layoutBindings[i].binding = layout.Bindings[i].BindingPos;
			layoutBindings[i].descriptorType = GetDescriptorType(layout.Bindings[i].Type);
			layoutBindings[i].descriptorCount = layout.Bindings[i].Count;
			layoutBindings[i].pImmutableSamplers = nullptr; // For image sampling
			layoutBindings[i].stageFlags = VulkanCommon::GetShaderStageFlags(layout.Bindings[i].Stage);
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

		if(descriptions.Diffuse.Type == TextureType::DIFFUSE)
			m_DiffuseTexture = rm.CreateTexture(descriptions.Diffuse);

		if(descriptions.ARM.Type == TextureType::ARM)
			m_ARMTexture = rm.CreateTexture(descriptions.ARM);

		if(descriptions.Normal.Type == TextureType::NORMAL)
			m_NormalTexture = rm.CreateTexture(descriptions.Normal);

		if (descriptions.LTC1.Type == TextureType::LTC1)
			m_LTC1_Texture = rm.CreateTexture(descriptions.LTC1);

		if (descriptions.LTC2.Type == TextureType::LTC2)
			m_LTC2_Texture = rm.CreateTexture(descriptions.LTC2);
	}

	void VulkanBindGroup::CreateDescriptorPool(const BindingLayoutType& type, bool IsTextured)
	{
		switch (type)
		{
			case BindingLayoutType::LAYOUT_MATERIAL_DEFAULT:
			{
				VkDescriptorPoolSize uboPoolSize{};
				uboPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				uboPoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

				VkDescriptorPoolSize samplerPoolSize{};
				std::vector<VkDescriptorPoolSize> poolSizes;
				if (IsTextured)
				{
					
					samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					samplerPoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
					poolSizes.resize(2);
					poolSizes = { uboPoolSize, samplerPoolSize };
					
				}
				else
				{
					poolSizes.push_back(uboPoolSize);
				}
				

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

				VkDescriptorPoolSize samplerPoolSize{};

				std::vector<VkDescriptorPoolSize> poolSizes;

				if (IsTextured)
				{
					samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					samplerPoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
					poolSizes.resize(2);
					poolSizes = { uboPoolSize, samplerPoolSize };

				}
				else
				{
					poolSizes.push_back(uboPoolSize);
				}


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

	void VulkanBindGroup::CreateDescriptorSets(const BindingLayoutType& type, bool isTextured)
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
			case BindingLayoutType::LAYOUT_MATERIAL_DEFAULT:
			{
				std::vector<VkDescriptorImageInfo> imageInfo{};
				if (isTextured)
					imageInfo.resize(3);

				for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
				{
					// SET BUFFER AND IMAGE INFO
					
						// UBO
						VkDescriptorBufferInfo bufferInfo{};
						bufferInfo.buffer = uniformBuffer.GetBuffers()[i];  
						bufferInfo.offset = uniformBuffer.GetOffset();
						bufferInfo.range = uniformBuffer.GetRange();

						if (isTextured)
						{
							// Diffuse
							imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
							imageInfo[0].imageView = static_cast<VulkanTexture&>(rm.GetTexture(m_DiffuseTexture)).GetImageView();
							imageInfo[0].sampler = static_cast<VulkanTexture&>(rm.GetTexture(m_DiffuseTexture)).GetSampler();

							// ARM
							imageInfo[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
							imageInfo[1].imageView = static_cast<VulkanTexture&>(rm.GetTexture(m_ARMTexture)).GetImageView();
							imageInfo[1].sampler = static_cast<VulkanTexture&>(rm.GetTexture(m_ARMTexture)).GetSampler();

							// Normal
							imageInfo[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
							imageInfo[2].imageView = static_cast<VulkanTexture&>(rm.GetTexture(m_NormalTexture)).GetImageView();
							imageInfo[2].sampler = static_cast<VulkanTexture&>(rm.GetTexture(m_NormalTexture)).GetSampler();
						}

						
					// WRITE DESCRIPTORS

						std::vector<VkWriteDescriptorSet> descriptorWrites{};
						descriptorWrites.resize(1);

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

						if (isTextured)
						{
							descriptorWrites.resize(4);

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
						}
						

					vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

				}
				break;
			}

			case BindingLayoutType::LAYOUT_RENDERPASS:
			{
				std::vector<VkDescriptorImageInfo> imageInfo{};
				imageInfo.resize(2);

				for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
				{
					// SET BUFFER AND IMAGE INFO
					
						// UBO
						VkDescriptorBufferInfo bufferInfo{};
						bufferInfo.buffer = uniformBuffer.GetBuffers()[i];
						bufferInfo.offset = uniformBuffer.GetOffset();
						bufferInfo.range = uniformBuffer.GetRange();

						// LTC1
						imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						imageInfo[0].imageView = static_cast<VulkanTexture&>(rm.GetTexture(m_LTC1_Texture)).GetImageView();
						imageInfo[0].sampler = static_cast<VulkanTexture&>(rm.GetTexture(m_LTC1_Texture)).GetSampler();

						// LTC2
						imageInfo[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						imageInfo[1].imageView = static_cast<VulkanTexture&>(rm.GetTexture(m_LTC2_Texture)).GetImageView();
						imageInfo[1].sampler = static_cast<VulkanTexture&>(rm.GetTexture(m_LTC2_Texture)).GetSampler();


					// WRITE DESCRIPTORS

						std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

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

						// LTC1 Texture
						descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						descriptorWrites[1].dstSet = m_DescriptorSets[i];
						descriptorWrites[1].dstBinding = 1;
						descriptorWrites[1].dstArrayElement = 0;
						descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						descriptorWrites[1].descriptorCount = 1;
						descriptorWrites[1].pBufferInfo = nullptr;
						descriptorWrites[1].pImageInfo = &imageInfo[0];
						descriptorWrites[1].pTexelBufferView = nullptr; // Optional

						// LTC2 Texture
						descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						descriptorWrites[2].dstSet = m_DescriptorSets[i];
						descriptorWrites[2].dstBinding = 2;
						descriptorWrites[2].dstArrayElement = 0;
						descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						descriptorWrites[2].descriptorCount = 1;
						descriptorWrites[2].pBufferInfo = nullptr;
						descriptorWrites[2].pImageInfo = &imageInfo[1];
						descriptorWrites[2].pTexelBufferView = nullptr; // Optional

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
}
