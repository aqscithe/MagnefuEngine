#include "mfpch.h"
#include "VulkanBuffer.h"
#include "VulkanContext.h"
#include "Magnefu/Renderer/RenderConstants.h"

namespace Magnefu
{
	VulkanBuffer::VulkanBuffer(const BufferDesc& desc) : Buffer(desc)
	{
		switch (desc.Usage)
		{
			case USAGE_VERTEX:
			{
				break;
			}

			case USAGE_INDEX:
			{
				break;
			}

		default:
			break;
		}
		
	}

	VulkanBuffer::~VulkanBuffer()
	{

	}

	uint32_t VulkanBuffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(VulkanContext::Get().GetPhysicalDevice(), &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		MF_CORE_ASSERT(false, "failed to find suitable memory type!");

		return 0;
	}

	void VulkanBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkDevice device = VulkanContext::Get().GetDevice();

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		//bufferInfo.flags = 

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create vertex buffer!");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.allocationSize = memRequirements.size;
		memAllocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		// TODO:
		// It should be noted that in a real world application, you're not supposed to actually call
		// vkAllocateMemory for every individual buffer. The maximum number of simultaneous memory 
		// allocations is limited by the maxMemoryAllocationCount physical device limit, which may 
		// be as low as 4096 even on high end hardware like an NVIDIA GTX 1080. The right way to 
		// allocate memory for a large number of objects at the same time is to create a custom 
		// allocator that splits up a single allocation among many different objects by using the 
		// offset parameters that we've seen in many functions.

		// You can either implement such an allocator yourself, or use the VulkanMemoryAllocator 
		// library provided by the GPUOpen initiative.However, for this tutorial it's okay to use a 
		// separate allocation for every resource, because we won't come close to hitting any of 
		// these limits for now.
		if (vkAllocateMemory(device, &memAllocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to allocate vertex buffer memory!");

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	void VulkanBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(commandBuffer);
	}

	VkCommandBuffer VulkanBuffer::BeginSingleTimeCommands()
	{
		VulkanContext context = VulkanContext::Get();

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = context.GetCommandPool();
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(context.GetDevice(), &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void VulkanBuffer::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		VulkanContext context = VulkanContext::Get();

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(context.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);

		//A fence would allow you to schedule multiple transfers simultaneously
		// and wait for all of them complete, instead of executing one at a time.
		// That may give the driver more opportunities to optimize.
		// 
		//vkWaitForFences()
		vkQueueWaitIdle(context.GetGraphicsQueue());

		vkFreeCommandBuffers(context.GetDevice(), context.GetCommandPool(), 1, &commandBuffer);
	}


	VulkanUniformBuffer::VulkanUniformBuffer(const BufferDesc& desc) : VulkanBuffer(desc)
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		m_Buffers.resize(MAX_FRAMES_IN_FLIGHT);
		m_BuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		m_BuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			CreateBuffer(
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_Buffers[i],
				m_BuffersMemory[i]
			);

			vkMapMemory(VulkanContext::Get().GetDevice(), m_BuffersMemory[i], 0, bufferSize, 0, &m_BuffersMapped[i]);
		}
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{

	}
}

