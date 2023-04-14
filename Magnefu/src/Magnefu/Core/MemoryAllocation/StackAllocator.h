#pragma once

#include "Magnefu/Core/Assertions.h"
#include "Magnefu/Core/Log.h"
#include <stdint.h>
#include <cstdlib>


namespace Magnefu
{
	struct MemAllocData
	{
		MemAllocData()
		{
			FrameCounter = 0;
			AllocationCounter = 0;
			CurrentBytesAllocated = 0;
			TotalBytesAllocated = 0;
			AvgAllocsPerFramePerMin = 0;
			AvgBytePerFramePerMin = 0;
			Timer = 30000.f;
		}
		uint32_t FrameCounter;
		uint32_t AllocationCounter;
		uint32_t CurrentBytesAllocated;
		uint32_t TotalBytesAllocated;
		uint32_t AvgAllocsPerFramePerMin;
		uint32_t AvgBytePerFramePerMin;
		float Timer;
	};

	class StackAllocator
	{
	public:
		StackAllocator(uint32_t stackSizeBytes);
		~StackAllocator();

		uintptr_t AlignAddress(uintptr_t addr, size_t align);
		void* AllocAligned(size_t bytes, size_t align);
		void* Allocate(uint32_t bytes);

		// Rolls the stack back to a previous marker.
		inline void FreeToMarker() { m_Top = m_Marker; }

		// Returns a marker to the current stack top.
		inline void* GetMarker() { return m_Marker; }

		// Clears the entire stack (rolls the stack back to
		// zero).
		inline void Clear()
		{
			m_Top = m_Buffer;
			m_Marker = m_Buffer;
		}

		template<typename T>
		inline T* AlignPointer(T* ptr, size_t align)
		{
			const uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
			const uintptr_t addrAligned = AlignAddress(addr, align);
			return reinterpret_cast<T*>(addrAligned);
		}

		
		Ref<MemAllocData>& GetMemAllocData() { return m_Data; }

		static Scope<StackAllocator>& Get() { return s_Instance; }

	private:
		Ref<MemAllocData> m_Data;
		void* m_Marker;
		void* m_Buffer;
		void* m_Top;
		void* m_End;


		static Scope<StackAllocator> s_Instance;
	};
}
