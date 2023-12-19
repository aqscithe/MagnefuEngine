#pragma once

#include "Pool.h"


namespace Magnefu
{
	class ResourceManager
	{
	public:
		ResourceManager() = default;
		~ResourceManager() = default;

		ResourceManager(const ResourceManager&) = delete;
		ResourceManager& operator=(const ResourceManager&) = delete;

		ResourceManager(ResourceManager&&) = delete;
		ResourceManager& operator=(ResourceManager&&) = delete;

		static ResourceManager* Create();

		Handle<Texture>   CreateTexture(const TextureDesc&);
		Handle<Buffer>    CreateBuffer(const BufferDesc&);
		Handle<BindGroup> CreateBindGroup(const BindGroupDesc&);
		Handle<Shader>    CreateShader(const ShaderDesc&);

		Texture&   GetTexture(const Handle<Texture>& handle);
		Buffer&    GetBuffer(const Handle<Buffer>& handle);
		BindGroup& GetBindGroup(const Handle<BindGroup>& handle);
		Shader&    GetShader(const Handle<Shader>& handle);

		uint32_t GetTextureCount() { return m_TexturePool.GetResourceCount(); }
		uint32_t GetBufferCount() { return m_BufferPool.GetResourceCount(); }
		uint32_t GetBindGroupCount() { return m_BindGroupPool.GetResourceCount(); }
		uint32_t GetShaderCount() { return m_ShaderPool.GetResourceCount(); }

		void DestroyTexture(Handle<Texture> handle);
		void DestroyBuffer(Handle<Buffer> handle);
		void DestroyBindGroup(Handle<BindGroup> handle);
		void DestroyShader(Handle<Shader> handle);
		

	private:
		Pool<Texture>   m_TexturePool;
		Pool<Buffer>    m_BufferPool;
		Pool<BindGroup> m_BindGroupPool;
		Pool<Shader>    m_ShaderPool;
		
	};
}