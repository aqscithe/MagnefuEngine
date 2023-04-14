#include "mfpch.h"
#include "StackAllocator.h"

namespace Magnefu
{
	const std::size_t FRAME_SIZE = 2048;
	Scope<StackAllocator> StackAllocator::s_Instance = CreateScope<StackAllocator>(FRAME_SIZE);


	StackAllocator::StackAllocator(uint32_t stackSizeBytes)
	{
		m_Buffer = std::malloc(stackSizeBytes);
		MF_CORE_ASSERT(m_Buffer, "Stack Allocator failed to create buffer.");

		m_Top = m_Buffer;
		m_Marker = m_Top;
		m_End = static_cast<char*>(m_Buffer) + stackSizeBytes;

		m_Data = CreateRef<MemAllocData>();
	}

	StackAllocator::~StackAllocator()
	{
		std::free(m_Buffer);
	}

	// Shift the given address upwards if/as necessary to
	// ensure it is aligned to the given number of bytes.
	uintptr_t StackAllocator::AlignAddress(uintptr_t addr, size_t align)
	{
		const size_t mask = align - 1;
		MF_CORE_ASSERT((align & mask) == 0, "Alignment not power of 2!"); // pwr of 2
		return (addr + mask) & ~mask;
	}

	// Aligned allocation function. IMPORTANT: 'align'
	// must be a power of 2 (typically 4, 8 or 16).
	void* StackAllocator::AllocAligned(size_t bytes, size_t align)
	{
		// Allocate 'align' more bytes than we need.
		size_t actualBytes = bytes + align;

		// Allocate unaligned block.
		uint8_t* pRawMem = new uint8_t[actualBytes];

		// Align the block. If no alignment occurred,
		// shift it up the full 'align' bytes so we
		// always have room to store the shift.
		uint8_t* pAlignedMem = AlignPointer(pRawMem, align);
		if (pAlignedMem == pRawMem)
			pAlignedMem += align;

		// Determine the shift, and store it.
		// (This works for up to 256-byte alignment.)
		ptrdiff_t shift = pAlignedMem - pRawMem;
		MF_CORE_ASSERT(shift > 0 && shift <= 256, "Byte shift outside valid range: (0, 256)");
		pAlignedMem[-1] = static_cast<uint8_t>(shift & 0xFF);
		return pAlignedMem;
	}

	void* StackAllocator::Allocate(uint32_t bytes)
	{
		// Align allocation to the size of a pointer
		const std::size_t alignment = alignof(std::max_align_t);

		const std::size_t aligned_size = ((bytes + alignment - 1) / alignment) * alignment;

		// Check if there's enough space in the buffer
		//MF_CORE_ASSERT((static_cast<char*>(m_Top) + aligned_size <= m_End), "Stack allocator out of memory");

		void* ptr = m_Top;
		m_Marker = m_Top;
		m_Top = AllocAligned(bytes, alignment);

		// should be some MACRO defined to ensure this only calculates in debug mode
#ifdef MF_DEBUG
		m_Data->AllocationCounter++;
		m_Data->TotalBytesAllocated += bytes;
#endif
		return ptr;
	}


}