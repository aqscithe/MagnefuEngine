#pragma once
#include "Magnefu/Renderer/Buffer.h"
#include "Magnefu/Renderer/Texture.h"
#include "Magnefu/Renderer/BindGroup.h"
#include "Magnefu/Renderer/Shader.h"
#include "Magnefu/ResourceManagement/Handle.h"


namespace Magnefu
{
    template <class T>
    inline void hash_combine(std::size_t& seed, const T& v) {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

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
                return Handle<Texture>{ static_cast<uint32_t>(m_Resources.size() - 1), static_cast<uint32_t>(m_Generations.back()), 0 };
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
                return Handle<Texture>{ static_cast<uint32_t>(index), static_cast<uint32_t>(m_Generations[index]), 0 };
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
                return Handle<Buffer>{ static_cast<uint32_t>(m_Resources.size() - 1), static_cast<uint32_t>(m_Generations.back()), 0 };
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
                return Handle<Buffer>{ static_cast<uint32_t>(index), static_cast<uint32_t>(m_Generations[index]), 0 };
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
                return Handle<BindGroup>{ static_cast<uint32_t>(m_Resources.size() - 1), static_cast<uint32_t>(m_Generations.back()), 0 };
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
                return Handle<BindGroup>{ static_cast<uint32_t>(index), static_cast<uint32_t>(m_Generations[index]), 0 };
            }
        }

        Handle<Shader> Create(const ShaderDesc& desc)
        {
            // Create a key from the description.
            std::size_t key = CreateKey(desc);

            // Check if a shader with this configuration already exists.
            auto it = m_ResourceMap.find(key);
            if (it != m_ResourceMap.end()) {
                // Shader already exists. Return its handle.
                return it->second;
            }


            // Check if there are any free indices
            if (m_FreeList.empty())
            {
                // Resize the resource and generation vectors
                m_Resources.resize(m_Resources.size() + 1);
                m_Generations.resize(m_Generations.size() + 1, 0);

                // Construct a new object in the resource vector
                m_Resources.back() = ShaderFactory::CreateShader(desc);

                // Add new shader to the map
                uint32_t index = static_cast<uint32_t>(m_Resources.size() - 1);
                uint32_t generation = static_cast<uint32_t>(m_Generations.back());
                m_ResourceMap[key] = Handle<Shader>{ index, generation, static_cast<uint32_t>(key) };

                // Return a handle to the new object
                return Handle<Shader>{ index, generation, static_cast<uint32_t>(key) };
            }
            else
            {
                // Reuse an index from the free list
                int index = m_FreeList.back();
                m_FreeList.pop_back();

                // Construct a new object in the resource vector
                m_Resources[index] = ShaderFactory::CreateShader(desc);

                // Increase the generation counter for this index
                m_Generations[index]++;

                // Return a handle to the new object
                return Handle<Shader>{ static_cast<uint32_t>(index), static_cast<uint32_t>(m_Generations[index]), static_cast<uint32_t>(key) };
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

        uint32_t GetResourceCount() { return m_Resources.size(); }

        void Destroy(Handle<T> handle)
        {
            MF_CORE_ASSERT(handle.Index < m_Resources.size() && handle.Generation == m_Generations[handle.Index], "Invalid Handle");
            m_FreeList.push_back(handle.Index);
            m_Generations[handle.Index]++;

            auto it = m_ResourceMap.find(handle.Hash);
            if (it != m_ResourceMap.end())
                m_ResourceMap.erase(handle.Hash);
        }

    private:
        std::size_t CreateKey(const ShaderDesc& desc) {
            // Hash each field of desc individually.
            std::hash<std::string> stringHasher;
            std::hash<uint32_t> uint32Hasher;
            std::hash<int> intHasher;
            std::hash<float> floatHasher;
            std::hash<bool> boolHasher;

            // For now, I don't care about hashing the bind groups. All of objects will be using the same
            // render pass global layout and descriptor set layout.
            // I am also not hashing the DebugName, as it does not contribute to the configuration of the shader
            // or pipeline.

            size_t result = stringHasher(desc.Path);
            hash_combine(result, stringHasher(desc.StageDescriptions.VS.EntryPoint));
            hash_combine(result, stringHasher(desc.StageDescriptions.FS.EntryPoint));
            hash_combine(result, uint32Hasher(static_cast<uint32_t>(desc.StageDescriptions.VS.Stage)));
            hash_combine(result, uint32Hasher(static_cast<uint32_t>(desc.StageDescriptions.FS.Stage)));
            hash_combine(result, uint32Hasher(static_cast<uint32_t>(desc.GraphicsPipeline.DynamicStates[0])));
            hash_combine(result, uint32Hasher(static_cast<uint32_t>(desc.GraphicsPipeline.DynamicStates[1])));
            hash_combine(result, uint32Hasher(desc.GraphicsPipeline.ViewportInfo.ViewportCount));
            hash_combine(result, uint32Hasher(desc.GraphicsPipeline.ViewportInfo.ScissorCount));
            hash_combine(result, uint32Hasher(static_cast<uint32_t>(desc.GraphicsPipeline.RasterizerInfo.CullMode)));
            hash_combine(result, uint32Hasher(static_cast<uint32_t>(desc.GraphicsPipeline.RasterizerInfo.PolygonMode)));
            hash_combine(result, floatHasher(desc.GraphicsPipeline.RasterizerInfo.LineWidth));
            hash_combine(result, boolHasher(desc.GraphicsPipeline.MSAAInfo.EnableSampleShading));
            hash_combine(result, floatHasher(desc.GraphicsPipeline.MSAAInfo.MinSampleShading));
            hash_combine(result, uint32Hasher(static_cast<uint32_t>(desc.GraphicsPipeline.DepthAndStencilInfo.CompareOp)));
            hash_combine(result, boolHasher(desc.GraphicsPipeline.DepthAndStencilInfo.DepthBoundsTestEnable));
            hash_combine(result, boolHasher(desc.GraphicsPipeline.DepthAndStencilInfo.DepthTestEnable));
            hash_combine(result, boolHasher(desc.GraphicsPipeline.DepthAndStencilInfo.DepthWriteEnable));
            hash_combine(result, boolHasher(desc.GraphicsPipeline.DepthAndStencilInfo.StencilTestEnable));
            hash_combine(result, boolHasher(desc.GraphicsPipeline.PushConstantInfo.Enabled));
            hash_combine(result, uint32Hasher(desc.GraphicsPipeline.PushConstantInfo.ByteSize));
            hash_combine(result, uint32Hasher(desc.GraphicsPipeline.PushConstantInfo.Offset));
            hash_combine(result, uint32Hasher(static_cast<uint32_t>(desc.GraphicsPipeline.PushConstantInfo.Stages)));

            return result;
        }

    private:
        std::vector<T*>  m_Resources;
        std::vector<int> m_FreeList;  // Stores indices of free slots in the resources vector
        std::vector<int> m_Generations;  // Stores generation counters for each slot in the resources vector

        std::unordered_map<std::size_t, Handle<Shader>> m_ResourceMap;
    };
}

