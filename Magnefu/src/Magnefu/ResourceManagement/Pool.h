#pragma once
#include "Magnefu/Renderer/Buffer.h"
#include "Magnefu/Renderer/Texture.h"
#include "Magnefu/Renderer/BindGroup.h"


namespace Magnefu
{
    template <typename T>
    struct Handle
    {
        bool IsValid() const;

        uint32_t Index;  // Index into the resources vector in the Pool
        uint32_t Generation;  // Generation counter for the resource
    };


    template <typename T, typename...Args>
    class Pool
    {
    public:
        Handle<Texture> Create(const TextureDesc& desc)
        {
            // Check if there are any free indices
            if (m_FreeList.empty())
            {
                // Resize the resource and generation vectors
                m_Resources.resize(m_Resources.size() + 1);
                m_Generations.resize(m_Generations.size() + 1, 0);

                // Construct a new object in the resource vector
                m_Resources.back() = TextureFactory::CreateTexture(desc);

                // Return a handle to the new object
                return Handle<Texture>{ static_cast<uint32_t>(m_Resources.size() - 1), static_cast<uint32_t>(m_Generations.back()) };
            }
            else
            {
                // Reuse an index from the free list
                int index = m_FreeList.back();
                m_FreeList.pop_back();

                // Construct a new object in the resource vector
                m_Resources[index] = TextureFactory::CreateTexture(desc);

                // Increase the generation counter for this index
                m_Generations[index]++;

                // Return a handle to the new object
                return Handle<Texture>{ static_cast<uint32_t>(index), static_cast<uint32_t>(m_Generations[index]) };
            }
        }

        Handle<Buffer> Create(const BufferDesc& desc)
        {
            // Check if there are any free indices
            if (m_FreeList.empty())
            {
                // Resize the resource and generation vectors
                m_Resources.resize(m_Resources.size() + 1);
                m_Generations.resize(m_Generations.size() + 1, 0);

                // Construct a new object in the resource vector
                m_Resources.back() = BufferFactory::CreateBuffer(desc);

                // Return a handle to the new object
                return Handle<Buffer>{ static_cast<uint32_t>(m_Resources.size() - 1), static_cast<uint32_t>(m_Generations.back()) };
            }
            else
            {
                // Reuse an index from the free list
                int index = m_FreeList.back();
                m_FreeList.pop_back();

                // Construct a new object in the resource vector
                m_Resources[index] = BufferFactory::CreateBuffer(desc);

                // Increase the generation counter for this index
                m_Generations[index]++;

                // Return a handle to the new object
                return Handle<Buffer>{ static_cast<uint32_t>(index), static_cast<uint32_t>(m_Generations[index]) };
            }
        }

        Handle<BindGroup> Create(const BindGroupDesc& desc)
        {
            // Check if there are any free indices
            if (m_FreeList.empty())
            {
                // Resize the resource and generation vectors
                m_Resources.resize(m_Resources.size() + 1);
                m_Generations.resize(m_Generations.size() + 1, 0);

                // Construct a new object in the resource vector
                m_Resources.back() = BindGroupFactory::CreateBindGroup(desc);

                // Return a handle to the new object
                return Handle<BindGroup>{ static_cast<uint32_t>(m_Resources.size() - 1), static_cast<uint32_t>(m_Generations.back()) };
            }
            else
            {
                // Reuse an index from the free list
                int index = m_FreeList.back();
                m_FreeList.pop_back();

                // Construct a new object in the resource vector
                m_Resources[index] = BindGroupFactory::CreateBindGroup(desc);

                // Increase the generation counter for this index
                m_Generations[index]++;

                // Return a handle to the new object
                return Handle<BindGroup>{ static_cast<uint32_t>(index), static_cast<uint32_t>(m_Generations[index]) };
            }
        }

        T& Get(Handle<T> handle)
        {
            uint32_t handleIndex = handle.Index;
            uint32_t resourcesSize = static_cast<uint32_t>(m_Resources.size());
            uint32_t handleGeneration = static_cast<uint32_t>(handle.Generation);
            uint32_t generationOfIndex = static_cast<uint32_t>(m_Generations[handle.Index]);

            MF_CORE_ASSERT((handleIndex < resourcesSize)  &&  (handleGeneration == generationOfIndex), "Invalid Handle");
            return *m_Resources[handle.Index];
        }

        void Destroy(Handle<T> handle)
        {
            MF_CORE_ASSERT(handle.Index < m_Resources.size() && handle.Generation == m_Generations[handle.Index], "Invalid Handle");
            m_FreeList.push_back(handle.Index);
            m_Generations[handle.Index]++;
        }

    private:
        std::vector<T*>  m_Resources;
        std::vector<int> m_FreeList;  // Stores indices of free slots in the resources vector
        std::vector<int> m_Generations;  // Stores generation counters for each slot in the resources vector
    };
}

