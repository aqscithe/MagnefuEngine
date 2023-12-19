#pragma once

// -- Core Includes ---------------------------------- //
#include "Magnefu/Core/Service.hpp"

// -- other Includes -------------- //


namespace Magnefu
{
	// -- Memory Methods -- //

	// Copy memory
	void	memoryCopy(void* destination, void* source, sizet size);

	// Align memory
	sizet	memoryAlign(sizet size, sizet alignment);


	// -- Memory Structs -- //

	struct MemoryStatistics
	{
		sizet	allocatedBytes;
		sizet	totalBytes;
		u32		allocationCount;

		void add(sizet bytes)
		{
			allocatedBytes += bytes;
			allocationCount++;
		}
	};

	// Allocator Interface
	struct Allocator
	{
		virtual ~Allocator() = default;

		virtual void*	allocate(sizet size, sizet alignment) = 0;
		virtual void*	allocate(sizet size, sizet alignment, cstring file, i32 line) = 0;

		virtual void	deallocate(void* ptr) = 0;
	};

	

	struct HeapAllocator : public Allocator
	{
		~HeapAllocator() override;

		// -- Methods -- //

		void	init(sizet size);
		void	shutdown();


#if defined MF_IMGUI
		void	debugUI();
#endif

		void*	allocate(sizet size, sizet alignment) override;
		void*	allocate(sizet size, sizet alignment, cstring file, i32 line) override;

		void	deallocate(void* ptr) override;


		// -- Members -- //

		void*	tlsfHandle;
		void*	memory;
		sizet	allocatedSize = 0;;
		sizet	maxSize = 0;

	};


	struct StackAllocator : public Allocator
	{
		// -- Methods -- //
		void	init(sizet size);
		void	shutdown();

		void*	allocate(sizet size, sizet alignment) override;
		void*	allocate(sizet size, sizet alignment, cstring file, i32 line) override;

		void	deallocate(void* pointer) override;

		sizet   getMarker();
		void    freeToMarker(sizet marker);

		void    clear();


		// -- Members -- //
		u8*		memory = nullptr;
		sizet   totalSize = 0;
		sizet   allocatedSize = 0;
	};


	struct DoubleStackAllocator : public Allocator
	{
		// -- Methods -- //
		void	init(sizet size);
		void	shutdown();

		void*	allocate(sizet size, sizet alignment) override;
		void*	allocate(sizet size, sizet alignment, cstring file, i32 line) override;

		void	deallocate(void* pointer) override;

		void*	allocateTop(sizet size, sizet alignment);
		void*	allocateBottom(sizet size, sizet alignment);

		void	deallocateTop(sizet size);
		void	deallocateBottom(sizet size);


		sizet   getTopMarker();
		sizet   getBottomMarker();

		void    freeToTopMarker(sizet marker);
		void    freeToBottomMarker(sizet marker);

		void    clearTop();
		void    clearBottom();


		// -- Members -- //
		u8*		memory = nullptr;
		sizet   totalSize = 0;
		sizet   top = 0;
		sizet	bottom = 0;
	};

	// Allocator that can only be reset
	struct LinearAllocator : public Allocator 
	{

		~LinearAllocator();

		void    init(sizet size);
		void    shutdown();

		void*	allocate(sizet size, sizet alignment) override;
		void*	allocate(sizet size, sizet alignment, cstring file, i32 line) override;

		void    deallocate(void* pointer) override;

		void    clear();

		u8*		memory = nullptr;
		sizet   totalSize = 0;
		sizet   allocatedSize = 0;
	}; 

	
	// DANGER: this should be used for NON runtime processes, like compilation of resources.
	struct MallocAllocator : public Allocator 
	{
		void*	allocate(sizet size, sizet alignment) override;
		void*	allocate(sizet size, sizet alignment, cstring file, i32 line) override;

		void    deallocate(void* pointer) override;
	};

	// -- Memory Service -- //
	
	struct MemoryServiceConfiguration 
	{

		sizet   maxDynamicSize = 32 * 1024 * 1024;    // Defaults to max 32MB of dynamic memory.

	}; 
	
	struct MemoryService : public Service 
	{

		MF_DECLARE_SERVICE(MemoryService);

		void						Init(void* configuration);
		void						Shutdown();

#if defined MF_IMGUI
		void                        imguiDraw();
#endif 

		// Frame allocator
		LinearAllocator             scratchAllocator;
		HeapAllocator               systemAllocator;

		
		// Test allocators.
		void                        test();

		static constexpr cstring    kName = "Magnefu_Memory_Service";

	}; 



	// -- Macro helpers -- //
#define mfalloca(size, allocator)    ((allocator)->allocate( size, 1, __FILE__, __LINE__ ))
#define mfallocam(size, allocator)   ((u8*)(allocator)->allocate( size, 1, __FILE__, __LINE__ ))
#define mfallocat(type, allocator)   ((type*)(allocator)->allocate( sizeof(type), 1, __FILE__, __LINE__ ))

#define mfallocaa(size, allocator, alignment)    ((allocator)->allocate( size, alignment, __FILE__, __LINE__ ))

#define mffree(pointer, allocator) (allocator)->deallocate(pointer)

#define mfkilo(size)                 (size * 1024)
#define mfmega(size)                 (size * 1024 * 1024)
#define mfgiga(size)                 (size * 1024 * 1024 * 1024)
}