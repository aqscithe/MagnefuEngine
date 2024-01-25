#include "mfpch.h"

#include "AsynchronousLoader.hpp"

#include "Renderer.hpp"

#include "enkiTS/TaskScheduler.h"
#include "stb_image/stb_image.h"



namespace Magnefu
{
	// -- Asynchronous Loader -------------------------------------------------- //

	void AsynchronousLoader::init(Renderer* renderer_, enki::TaskScheduler* task_scheduler_, Allocator* resident_allocator_)
	{
		renderer = renderer_;
		task_scheduler = task_scheduler_;
		allocator = resident_allocator_;

		file_load_requests.init(allocator, 16);
		upload_requests.init(allocator, 16);

		texture_ready.index = k_invalid_texture.index;
		cpu_buffer_ready.index = k_invalid_texture.index;
		gpu_buffer_ready.index = k_invalid_texture.index;

		completed = nullptr;

		using namespace Magnefu;

		// Persistently mapped staging buffer
		// Needed to optimally transfer data from CPU to GPU
		BufferCreation bc;
		bc.reset().set(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, ResourceUsageType::Stream, mfmega(64)).set_name("staging_buffer").set_persistent(true);

		BufferHandle staging_buffer_handle = renderer->gpu->create_buffer(bc);
		staging_buffer = renderer->gpu->access_buffer(staging_buffer_handle);

		staging_buffer_offset = 0;

		// Create command pools linked to transfer queue
		for (u32 i = 0; i < GraphicsContext::k_max_frames; i++)
		{
			VkCommandPoolCreateInfo cmd_pool_info{};
			cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmd_pool_info.pNext = nullptr;
			cmd_pool_info.queueFamilyIndex = renderer->gpu->vulkan_transfer_queue_family;
			cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			vkCreateCommandPool(renderer->gpu->vulkan_device, &cmd_pool_info, renderer->gpu->vulkan_allocation_callbacks, &command_pools[i]);
		}

		for (u32 i = 0; i < GraphicsContext::k_max_frames; i++)
		{
			VkCommandBufferAllocateInfo cmd_alloc_info{};
			cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmd_alloc_info.pNext = nullptr;
			cmd_alloc_info.commandPool = command_pools[i];
			cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmd_alloc_info.commandBufferCount = 1;

			vkAllocateCommandBuffers(renderer->gpu->vulkan_device, &cmd_alloc_info, &command_buffers[i].vk_command_buffer);

			command_buffers[i].is_recording = false;
			command_buffers[i].gpu = renderer->gpu;
		}

		VkSemaphoreCreateInfo semaphore_info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		vkCreateSemaphore(renderer->gpu->vulkan_device, &semaphore_info, renderer->gpu->vulkan_allocation_callbacks, &transfer_complete_semaphore);

		VkFenceCreateInfo fence_info{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(renderer->gpu->vulkan_device, &fence_info, renderer->gpu->vulkan_allocation_callbacks, &transfer_fence);
	}


	void AsynchronousLoader::shutdown()
	{

		renderer->gpu->destroy_buffer(staging_buffer->handle);

		file_load_requests.shutdown();
		upload_requests.shutdown();

		for (u32 i = 0; i < GraphicsContext::k_max_frames; ++i)
		{
			vkDestroyCommandPool(renderer->gpu->vulkan_device, command_pools[i], renderer->gpu->vulkan_allocation_callbacks);
			// Command buffers are destroyed with the pool associated.
		}

		vkDestroySemaphore(renderer->gpu->vulkan_device, transfer_complete_semaphore, renderer->gpu->vulkan_allocation_callbacks);
		vkDestroyFence(renderer->gpu->vulkan_device, transfer_fence, renderer->gpu->vulkan_allocation_callbacks);
	}

	void AsynchronousLoader::update(Allocator* scratch_allocator)
	{
		using namespace Magnefu;

		// Process upload requests
		if (upload_requests.size)
		{
			if (vkGetFenceStatus(renderer->gpu->vulkan_device, transfer_fence) != VK_SUCCESS)
			{
				return;
			}

			vkResetFences(renderer->gpu->vulkan_device, 1, &transfer_fence);

			// Get last request
			UploadRequest upload_request = upload_requests.back();
			upload_requests.pop();

			CommandBuffer* cb = &command_buffers[renderer->gpu->current_frame];
			cb->begin();

			if (upload_request.texture.index != k_invalid_texture.index)
			{
				Texture* texture = renderer->gpu->access_texture(upload_request.texture);

				const u32 k_texture_channels = 4;
				const u32 k_texture_alignment = 4;
				const sizet aligned_image_size = memoryAlign(texture->width * texture->height * k_texture_channels, k_texture_alignment);
				// Request place in buffer
				const sizet current_offset = std::atomic_fetch_add(&staging_buffer_offset, aligned_image_size);

				cb->upload_texture_data(texture->handle, upload_request.data, staging_buffer->handle, current_offset);

				free(upload_request.data);
			}
			else if (upload_request.cpu_buffer.index != k_invalid_buffer.index && upload_request.gpu_buffer.index != k_invalid_buffer.index)
			{
				Buffer* src = renderer->gpu->access_buffer(upload_request.cpu_buffer);
				Buffer* dst = renderer->gpu->access_buffer(upload_request.gpu_buffer);

				cb->upload_buffer_data(src->handle, dst->handle);
			}
			else if (upload_request.cpu_buffer.index != k_invalid_buffer.index)
			{
				Buffer* buffer = renderer->gpu->access_buffer(upload_request.cpu_buffer);
				// TODO: proper alignment
				const sizet aligned_image_size = memoryAlign(buffer->size, 64);
				const sizet current_offset = std::atomic_fetch_add(&staging_buffer_offset, aligned_image_size);
				cb->upload_buffer_data(buffer->handle, upload_request.data, staging_buffer->handle, current_offset);

				free(upload_request.data);
			}


			cb->end();

			VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cb->vk_command_buffer;
			VkPipelineStageFlags wait_flag[]{ VK_PIPELINE_STAGE_TRANSFER_BIT };
			VkSemaphore wait_semaphore[]{ transfer_complete_semaphore };
			submitInfo.pWaitSemaphores = wait_semaphore;
			submitInfo.pWaitDstStageMask = wait_flag;

			VkQueue used_queue = renderer->gpu->vulkan_transfer_queue;
			vkQueueSubmit(used_queue, 1, &submitInfo, transfer_fence);

			// TODO(marco): better management for state machine. We need to account for file -> buffer,
			// buffer -> texture and buffer -> buffer. One the CPU buffer has been used it should be freed.
			if (upload_request.texture.index != k_invalid_index)
			{
				MF_ASSERT(texture_ready.index == k_invalid_texture.index, "");
				texture_ready = upload_request.texture;
			}
			else if (upload_request.cpu_buffer.index != k_invalid_buffer.index && upload_request.gpu_buffer.index != k_invalid_buffer.index)
			{
				MF_ASSERT(cpu_buffer_ready.index == k_invalid_index, "");
				MF_ASSERT(gpu_buffer_ready.index == k_invalid_index, "");
				MF_ASSERT((completed == nullptr), "");
				cpu_buffer_ready = upload_request.cpu_buffer;
				gpu_buffer_ready = upload_request.gpu_buffer;
				completed = upload_request.completed;
			}
			else if (upload_request.cpu_buffer.index != k_invalid_index)
			{
				MF_ASSERT(cpu_buffer_ready.index == k_invalid_index, "");
				cpu_buffer_ready = upload_request.cpu_buffer;
			}
		}


		// Process File Requests
		if (file_load_requests.size)
		{
			FileLoadRequest load_request = file_load_requests.back();
			file_load_requests.pop();

			i64 start_file_load = time_now();

			// Load texture file
			int x, y, comp;
			u8* texture_data = stbi_load(load_request.path, &x, &y, &comp, 4);

			if (texture_data)
			{
				MF_APP_INFO("Read file {} in {} ms.", load_request.path, time_from_milliseconds(start_file_load));

				UploadRequest& upload_request = upload_requests.push_use();
				upload_request.data = texture_data;
				upload_request.texture = load_request.texture;
				upload_request.cpu_buffer = k_invalid_buffer;
			}
			else
			{
				MF_APP_ERROR("Error reading file {}", load_request.path);
			}

		}

		staging_buffer_offset = 0;
	}


	void AsynchronousLoader::request_texture_data(cstring filename, TextureHandle texture)
	{

		FileLoadRequest& request = file_load_requests.push_use();
		strcpy(request.path, filename);
		request.texture = texture;
		request.buffer = k_invalid_buffer;
	}

	void AsynchronousLoader::request_buffer_upload(void* data, BufferHandle buffer)
	{

		UploadRequest& upload_request = upload_requests.push_use();
		upload_request.data = data;
		upload_request.cpu_buffer = buffer;
		upload_request.texture = k_invalid_texture;
	}

	void AsynchronousLoader::request_buffer_copy(BufferHandle src, BufferHandle dst, u32* completed)
	{

		UploadRequest& upload_request = upload_requests.push_use();
		upload_request.completed = completed;
		upload_request.data = nullptr;
		upload_request.cpu_buffer = src;
		upload_request.gpu_buffer = dst;
		upload_request.texture = k_invalid_texture;
	}
}