// -- PCH -- //
#include "mfpch.h"

// -- .h -- //
#include "PoolAllocator.h"

namespace Magnefu
{
	PoolAllocator::PoolAllocator(size_t blockSizeBytes, size_t numBlocks)
        : m_BlockSize(blockSizeBytes), m_NumBlocks(numBlocks), m_Buffer(nullptr), m_FreeList(nullptr)
    {
        // Allocate the buffer for the pool
        m_Buffer = malloc(m_BlockSize * m_NumBlocks);

        // Create the free list by linking all the blocks together
        char* p = reinterpret_cast<char*>(m_Buffer);
        for (size_t i = 0; i < m_NumBlocks - 1; i++)
        {
            char* next = p + m_BlockSize;
            *reinterpret_cast<char**>(p) = next;
            p = next;
        }
        *reinterpret_cast<char**>(p) = nullptr;
        m_FreeList = (char*)m_Buffer;
    }

    PoolAllocator::~PoolAllocator()
    {
        std::free(m_Buffer);
    }

    void PoolAllocator::Init(size_t blockSizeBytes, size_t numBlocks)
    {
        m_BlockSize = blockSizeBytes;
        m_NumBlocks = numBlocks;

        // Allocate the buffer for the pool
        m_Buffer = malloc(m_BlockSize * m_NumBlocks);

        // Create the free list by linking all the blocks together
        char* p = reinterpret_cast<char*>(m_Buffer);
        for (size_t i = 0; i < m_NumBlocks - 1; i++)
        {
            char* next = p + m_BlockSize;
            *reinterpret_cast<char**>(p) = next;
            p = next;
        }
        *reinterpret_cast<char**>(p) = nullptr;
        m_FreeList = (char*)m_Buffer;
    }

    void* PoolAllocator::Allocate()
    {
        // Return nullptr if the pool is full
        if (!m_FreeList)
        {
            return nullptr;
        }

        // Get the next free block from the free list
        void* block = m_FreeList;
        m_FreeList = *reinterpret_cast<char**>(block);

        return block;
    }

    void PoolAllocator::Deallocate(void* block)
    {
        *reinterpret_cast<char**>(block) = m_FreeList;
        m_FreeList = reinterpret_cast<char*>(block);
    }
}