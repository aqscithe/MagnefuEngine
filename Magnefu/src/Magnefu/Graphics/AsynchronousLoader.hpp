#pragma once


#include "CommandBuffer.hpp"
#include "GraphicsContext.h"
#include "GPUResources.hpp"

#include <atomic>

namespace enki { struct TaskScheduler; }

namespace Magnefu
{
	struct Renderer;
	struct Allocator;
	struct GPUVisualProfiler;
	struct ImGuiService;
	struct StackAllocator;

	struct FileLoadRequest
	{

		char                            path[512];
		TextureHandle          texture = k_invalid_texture;
		BufferHandle           buffer = k_invalid_buffer;
	}; // struct FileLoadRequest

//
//
	struct UploadRequest
	{

		void* data = nullptr;
		TextureHandle           texture = k_invalid_texture;
		BufferHandle            cpu_buffer = k_invalid_buffer;
		BufferHandle            gpu_buffer = k_invalid_buffer;
	}; // struct UploadRequest


	//
	struct AsynchronousLoader
	{

		void								init(Renderer* renderer, enki::TaskScheduler* task_scheduler, Allocator* resident_allocator);
		void								update(Allocator* scratch_allocator);
		void								shutdown();

		void								request_texture_data(cstring filename, TextureHandle texture);
		void								request_buffer_upload(void* data, BufferHandle buffer);
		void								request_buffer_copy(BufferHandle src, BufferHandle dst);


		Allocator*							allocator = nullptr;
		Renderer*							renderer = nullptr;
		enki::TaskScheduler*				task_scheduler = nullptr;

		Array<FileLoadRequest>				file_load_requests;
		Array<UploadRequest>				upload_requests;

		Buffer*								staging_buffer = nullptr;

		std::atomic_size_t					staging_buffer_offset;
		TextureHandle						texture_ready;
		BufferHandle						cpu_buffer_ready;
		BufferHandle						gpu_buffer_ready;
		u32* completed;

		VkCommandPool						command_pools[k_max_frames];
		CommandBuffer						command_buffers[k_max_frames];
		VkSemaphore							transfer_complete_semaphore;
		VkFence								transfer_fence;

	}; // struct AsynchonousLoader
}
