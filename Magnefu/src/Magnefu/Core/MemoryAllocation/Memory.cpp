// -- PCH -- //
#include "mfpch.h"

// -- header -- //
#include "Memory.hpp"

// -- Core Includes ------ //

// -- vender Includes -- //
#include "tlsf/tlsf.h"
#include "imgui/imgui.h"




namespace Magnefu
{
    //#define MF_MEMORY_DEBUG
#if defined (MF_MEMORY_DEBUG)
#define MF_MEMORY_ASSERT(cond) hy_assert(cond)
#else
#define MF_MEMORY_ASSERT(cond)
#endif // MF_MEMORY_DEBUG


// -- Memory Service -- //
    static MemoryService    s_MemoryService;

    // Locals
    static size_t s_size = mfmega(32) + tlsf_size() + 8;

    
    // Walker methods
    static void exitWalker(void* ptr, size_t size, int used, void* user);
    static void imguiWalker(void* ptr, size_t size, int used, void* user);

    MemoryService* MemoryService::Instance() 
    {
        return &s_MemoryService;
    }

    
    void MemoryService::Init(void* configuration) {

        MF_CORE_INFO("Memory Service Init");
        MemoryServiceConfiguration* memory_configuration = static_cast<MemoryServiceConfiguration*>(configuration);
        systemAllocator.init(memory_configuration ? memory_configuration->maxDynamicSize : s_size);

        tempStackAllocator.init(mfmega(3));
        
    }

    void MemoryService::Shutdown() {

        systemAllocator.shutdown();
        tempStackAllocator.shutdown();

        MF_CORE_INFO("Memory Service Shutdown\n");
    }

    void exitWalker(void* ptr, size_t size, int used, void* user) {
        MemoryStatistics* stats = (MemoryStatistics*)user;
        stats->add(used ? size : 0);

        if (used)
            MF_CORE_INFO("Found active allocation {0} | {1} bytes", ptr, size);
    }

#if defined MF_IMGUI
    void imguiWalker(void* ptr, size_t size, int used, void* user) 
    {

        u32 memory_size = (u32)size;
        cstring memory_unit = "b";
        if (memory_size > 1024 * 1024) {
            memory_size /= 1024 * 1024;
            memory_unit = "Mb";
        }
        else if (memory_size > 1024) {
            memory_size /= 1024;
            memory_unit = "kb";
        }
        ImGui::Text("\t%p %s size: %4llu %s\n", ptr, used ? "used" : "free", memory_size, memory_unit);

        MemoryStatistics* stats = (MemoryStatistics*)user;
        stats->add(used ? size : 0);
    }


    void MemoryService::imguiDraw() 
    {

        systemAllocator.debugUI();

    }
#endif 

    void MemoryService::test() 
    {

        //static u8 mem[ 1024 ];
        //LinearAllocator la;
        //la.init( mem, 1024 );

        //// Allocate 3 times
        //void* a1 = mfalloca( 16, &la );
        //void* a2 = mfalloca( 20, &la );
        //void* a4 = mfalloca( 10, &la );
        //// Free based on size
        //la.free( 10 );
        //void* a3 = mfalloca( 10, &la );
        ////MF_CORE_ASSERT( a3 == a4 );

        //// Free based on pointer
        //mffree( a2, &la );
        //void* a32 = mfalloca( 10, &la );
        ////MF_CORE_ASSERT( a32 == a2 );
        //// Test out of bounds 
        //u8* out_bounds = ( u8* )a1 + 10000;
        //mffree( out_bounds, &la );
    }

    // -- Memory Structs -- //

    // -- HeapAllocator -- //
    HeapAllocator::~HeapAllocator() 
    {
    }

    void HeapAllocator::init(sizet size) 
    {
        // Allocate
        memory = malloc(size);
        maxSize = size;
        allocatedSize = 0;

        tlsfHandle = tlsf_create_with_pool(memory, size);

        MF_CORE_INFO("HeapAllocator of size {} bytes created", size);
    }

    void HeapAllocator::shutdown() 
    {

        // Check memory at the application exit.
        MemoryStatistics stats{ 0, maxSize };
        pool_t pool = tlsf_get_pool(tlsfHandle);
        tlsf_walk_pool(pool, exitWalker, (void*)&stats);

        if (stats.allocatedBytes) {
            MF_CORE_INFO("HeapAllocator Shutdown.\n===============\nFAILURE! Allocated memory detected. allocated {0}, total {1}\n===============\n\n", stats.allocatedBytes, stats.totalBytes);
        }
        else {
            MF_CORE_INFO("HeapAllocator Shutdown - all memory free!\n");
        }

        //MF_CORE_ASSERT(stats.allocatedBytes == 0, "Allocations still present. Check your code!");

        tlsf_destroy(tlsfHandle);

        free(memory);
    }

#if defined MF_IMGUI
    void HeapAllocator::debugUI() {

        ImGui::SeparatorText("HEAP ALLOCATOR");
        MemoryStatistics stats{ 0, maxSize };
        pool_t pool = tlsf_get_pool(tlsfHandle);
        tlsf_walk_pool(pool, imguiWalker, (void*)&stats);

        ImGui::Separator();
        ImGui::Text("\tAllocation count %d", stats.allocationCount);
        ImGui::Text("\tAllocated %llu K, free %llu Mb, total %llu Mb", stats.allocatedBytes / (1024 * 1024), (maxSize - stats.allocatedBytes) / (1024 * 1024), maxSize / (1024 * 1024));
    }
#endif 


#if defined (MF_MEMORY_STACK)
    class MagnefuStackWalker : public StackWalker 
    {
    public:
        MagnefuStackWalker() : StackWalker() {}
    protected:
        virtual void OnOutput(LPCSTR szText) 
        {
            MF_CORE_INFO("\nStack: \n{}\n", szText);
            StackWalker::OnOutput(szText);
        }
    }; // class MagnefuStackWalker

    void* HeapAllocator::allocate(sizet size, sizet alignment) {

        /*if ( size == 16 )
        {
            MagnefuStackWalker sw;
            sw.ShowCallstack();
        }*/

        void* mem = tlsf_malloc(tlsfHandle, size);
        MF_CORE_INFO("Mem: {0}, size {1} ", mem, size);
        return mem;
    }
#else

    void* HeapAllocator::allocate(sizet size, sizet alignment) {
#if defined (HEAP_ALLOCATOR_STATS)
        void* allocated_memory = alignment == 1 ? tlsf_malloc(tlsfHandle, size) : tlsf_memalign(tlsfHandle, alignment, size);
        sizet actual_size = tlsf_block_size(allocated_memory);
        allocatedSize += actual_size;

        /*if ( size == 52224 ) {
            return allocated_memory;
        }*/
        return allocated_memory;
#else
        return tlsf_malloc(tlsfHandle, size);
#endif // HEAP_ALLOCATOR_STATS
    }
#endif // Magnefu_MEMORY_STACK

    void* HeapAllocator::allocate(sizet size, sizet alignment, cstring file, i32 line) {
        return allocate(size, alignment);
    }

    void HeapAllocator::deallocate(void* pointer) {
#if defined (HEAP_ALLOCATOR_STATS)
        sizet actual_size = tlsf_block_size(pointer);
        allocatedSize -= actual_size;

        tlsf_free(tlsfHandle, pointer);
#else
        tlsf_free(tlsfHandle, pointer);
#endif
    }


    // -- LinearAllocator -- //

    LinearAllocator::~LinearAllocator() 
    {
    }

    void LinearAllocator::init(sizet size) 
    {

        memory = (u8*)malloc(size);
        totalSize = size;
        allocatedSize = 0;
    }

    void LinearAllocator::shutdown() 
    {
        clear();
        free(memory);
    }

    void* LinearAllocator::allocate(sizet size, sizet alignment) 
    {
        //MF_CORE_ASSERT(size > 0, "");

        const sizet new_start = memoryAlign(allocatedSize, alignment);
        //MF_CORE_ASSERT(new_start < totalSize, "");
        const sizet new_allocatedSize = new_start + size;
        if (new_allocatedSize > totalSize) 
        {
            MF_MEMORY_ASSERT(false && "Overflow");
            return nullptr;
        }

        allocatedSize = new_allocatedSize;
        return memory + new_start;
    }

    void* LinearAllocator::allocate(sizet size, sizet alignment, cstring file, i32 line) {
        return allocate(size, alignment);
    }

    void LinearAllocator::deallocate(void*) {
        // This allocator does not allocate on a per-pointer base!
    }

    void LinearAllocator::clear() {
        allocatedSize = 0;
    }

    // -- Memory Methods -- //
    void memoryCopy(void* destination, void* source, sizet size) {
        memcpy(destination, source, size);
    }

    sizet memoryAlign(sizet size, sizet alignment) {
        const sizet alignment_mask = alignment - 1;
        return (size + alignment_mask) & ~alignment_mask;
    }


    // -- MallocAllocator -- //
    void* MallocAllocator::allocate(sizet size, sizet alignment) {
        return malloc(size);
    }

    void* MallocAllocator::allocate(sizet size, sizet alignment, cstring file, i32 line) {
        return malloc(size);
    }

    void MallocAllocator::deallocate(void* pointer) {
        free(pointer);
    }

	// -- StackAllocator -- //
	
	void StackAllocator::init(sizet size)
	{
		memory = (u8*)malloc(size);
		//MF_CORE_ASSERT(memory, "Stack Allocator failed to create buffer.");

		allocatedSize = 0;
		totalSize = size;
	}

	void StackAllocator::shutdown()
	{
		free(memory);
	}

	void* StackAllocator::allocate(sizet size, sizet alignment)
	{
		//MF_CORE_ASSERT(size > 0, "Allocation size is 0 or less.");

		const sizet newStart = memoryAlign(allocatedSize, alignment);
		//MF_CORE_ASSERT(newStart < totalSize, "New start > total size");

		const sizet newAllocatedSize = newStart + size;
		if (newAllocatedSize > totalSize)
		{
			//MF_CORE_ASSERT(false, "Overflow");
			return nullptr;
		}

		allocatedSize = newAllocatedSize;
		return memory + newStart;
	}

	void* StackAllocator::allocate(sizet size, sizet alignment, cstring file, i32 line)
	{
		return allocate(size, alignment);
	}

	void StackAllocator::deallocate(void* pointer)
	{
		//MF_CORE_ASSERT((pointer >= memory), "");

		//MF_CORE_ASSERT((pointer < memory + totalSize), "Out of bound free on stack allocator (outside bounds). Tempting to free %p, %llu after beginning of buffer (memory %p size %llu, allocated %llu)", (u8*)pointer, (u8*)pointer - memory, memory, totalSize, allocatedSize);

		//MF_CORE_ASSERT((pointer < memory + allocatedSize), "Out of bound free on linear allocator (inside bounds, after allocated). Tempting to free %p, %llu after beginning of buffer (memory %p size %llu, allocated %llu)", (u8*)pointer, (u8*)pointer - memory, memory, totalSize, allocatedSize);

		const sizet sizeAtPointer = (u8*)pointer - memory;

		allocatedSize = sizeAtPointer;

	}

	sizet StackAllocator::getMarker()
	{
		return allocatedSize;
	}

	void StackAllocator::freeToMarker(sizet marker)
	{
		const sizet difference = marker - allocatedSize;
		if (difference > 0) 
		{
			allocatedSize = marker;
		}
	}

	void StackAllocator::clear()
	{
		allocatedSize = 0;
	}


    // -- DoubleStackAllocator -- //
    void DoubleStackAllocator::init(sizet size) {
        memory = (u8*)malloc(size);
        top = size;
        bottom = 0;
        totalSize = size;
    }

    void DoubleStackAllocator::shutdown() {
        free(memory);
    }

    void* DoubleStackAllocator::allocate(sizet size, sizet alignment) 
    {
        //MF_CORE_ASSERT(false, "failed double stack allocation");
        return nullptr;
    }

    void* DoubleStackAllocator::allocate(sizet size, sizet alignment, cstring file, i32 line) 
    {
        //MF_CORE_ASSERT(false, "failed double stack allocation");
        return nullptr;
    }

    void DoubleStackAllocator::deallocate(void* pointer) 
    {
        //MF_CORE_ASSERT(false, "failed double stack deallocation");
    }

    void* DoubleStackAllocator::allocateTop(sizet size, sizet alignment) 
    {
        //MF_CORE_ASSERT(size > 0, "invalid allocation top size {} - double stack", size);

        const sizet new_start = memoryAlign(top - size, alignment);
        if (new_start <= bottom) {
            MF_MEMORY_ASSERT(false && "Overflow Crossing");
            return nullptr;
        }

        top = new_start;
        return memory + new_start;
    }

    void* DoubleStackAllocator::allocateBottom(sizet size, sizet alignment) 
    {
        //MF_CORE_ASSERT(size > 0, "invalid allocation bottom size {} - double stack", size);

        const sizet new_start = memoryAlign(bottom, alignment);
        const sizet new_allocatedSize = new_start + size;
        if (new_allocatedSize >= top) 
        {
            MF_MEMORY_ASSERT(false && "Overflow Crossing");
            return nullptr;
        }

        bottom = new_allocatedSize;
        return memory + new_start;
    }

    void DoubleStackAllocator::deallocateTop(sizet size) 
    {
        if (size > totalSize - top) 
        {
            top = totalSize;
        }
        else 
        {
            top += size;
        }
    }

    void DoubleStackAllocator::deallocateBottom(sizet size) 
    {
        if (size > bottom) 
        {
            bottom = 0;
        }
        else 
        {
            bottom -= size;
        }
    }

    sizet DoubleStackAllocator::getTopMarker() 
    {
        return top;
    }

    sizet DoubleStackAllocator::getBottomMarker() 
    {
        return bottom;
    }

    void DoubleStackAllocator::freeToTopMarker(sizet marker) 
    {
        if (marker > top && marker < totalSize)
        {
            top = marker;
        }
    }

    void DoubleStackAllocator::freeToBottomMarker(sizet marker) 
    {
        if (marker < bottom) 
        {
            bottom = marker;
        }
    }

    void DoubleStackAllocator::clearTop() 
    {
        top = totalSize;
    }

    void DoubleStackAllocator::clearBottom() 
    {
        bottom = 0;
    }

}