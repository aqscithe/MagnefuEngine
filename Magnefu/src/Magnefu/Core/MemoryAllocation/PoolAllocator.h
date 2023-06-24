#pragma once
#include <stdint.h>
#include <cstdlib>

namespace Magnefu
{
    class PoolAllocator
    {
    public:
        PoolAllocator() :
            m_Buffer(nullptr), m_FreeList(nullptr), m_BlockSize(0), m_NumBlocks(0)
        {}

        PoolAllocator(size_t blockSizeBytes, size_t numBlocks);
        ~PoolAllocator();

        void Init(size_t blockSizeBytes, size_t numBlocks);

        void* Allocate();
        void Deallocate(void* block);

    private:
        void* m_Buffer;
        char* m_FreeList;
        size_t m_BlockSize;
        size_t m_NumBlocks;
    };
}
