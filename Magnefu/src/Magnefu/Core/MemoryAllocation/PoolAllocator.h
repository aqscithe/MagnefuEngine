#pragma once
#include <stdint.h>
#include <cstdlib>

namespace Magnefu
{
    class PoolAllocator
    {
    public:
        PoolAllocator(size_t blockSizeBytes, size_t numBlocks);
        ~PoolAllocator();

        void* Allocate();
        void Deallocate(void* block);

    private:
        void* m_Buffer;
        char* m_FreeList;
        size_t m_BlockSize;
        size_t m_NumBlocks;
    };
}
