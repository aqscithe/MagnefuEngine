#include "mfpch.h"
#include "StackAllocator.h"

namespace Magnefu
{
	const std::size_t STACK_SIZE = 2048;
	Scope<StackAllocator> StackAllocator::s_Instance = CreateScope<StackAllocator>(STACK_SIZE);


	StackAllocator::StackAllocator(uint32_t stackSizeBytes)
	{
		m_Buffer = std::malloc(stackSizeBytes);
		MF_CORE_ASSERT(m_Buffer, "Stack Allocator failed to create buffer.");

		m_Top = m_Buffer;
		m_Marker = m_Top;
		m_End = static_cast<char*>(m_Buffer) + stackSizeBytes;

	}

	StackAllocator::~StackAllocator()
	{
		std::free(m_Buffer);
	}

	void* StackAllocator::Allocate(uint32_t bytes, size_t alignment)
	{
		std::uintptr_t alignedAddress = (reinterpret_cast<std::uintptr_t>((char*)m_Top) + alignment - 1) & ~(alignment - 1);
		MF_CORE_ASSERT(alignedAddress + bytes > std::uintptr_t(m_Buffer) + STACK_SIZE, "Stack Allocator out of memory");

		m_Marker = m_Top;
		m_Top = reinterpret_cast<char*>(alignedAddress + bytes);

		//MF_CORE_DEBUG("Stack Allocator ALLOCATION - Top: {0} | Marker: {1} | Bytes: {2} | Alignment: {3}", m_Top, m_Marker, bytes, alignment);
		return reinterpret_cast<void*>(alignedAddress);
	}


}