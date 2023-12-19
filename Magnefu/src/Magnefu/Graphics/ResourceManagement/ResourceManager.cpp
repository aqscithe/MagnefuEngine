#include "mfpch.h"
#include "ResourceManager.h"

namespace Magnefu
{
	ResourceManager* ResourceManager::Create()
	{
		return new ResourceManager();
	}

	Handle<Texture> ResourceManager::CreateTexture(const TextureDesc& desc)
	{
		return m_TexturePool.Create(desc);
	}

	Handle<Buffer> ResourceManager::CreateBuffer(const BufferDesc& desc)
	{
		return m_BufferPool.Create(desc);
	}

	Handle<BindGroup> ResourceManager::CreateBindGroup(const BindGroupDesc& desc)
	{
		return m_BindGroupPool.Create(desc);
	}

	Handle<Shader> ResourceManager::CreateShader(const ShaderDesc& desc)
	{
		return m_ShaderPool.Create(desc);
	}

	Texture& ResourceManager::GetTexture(const Handle<Texture>& handle)
	{
		return m_TexturePool.Get(handle);
	}

	Buffer& ResourceManager::GetBuffer(const Handle<Buffer>& handle)
	{
		return m_BufferPool.Get(handle);
	}

	BindGroup& ResourceManager::GetBindGroup(const Handle<BindGroup>& handle)
	{
		return m_BindGroupPool.Get(handle);
	}

	Shader& ResourceManager::GetShader(const Handle<Shader>& handle)
	{
		return m_ShaderPool.Get(handle);
	}

	void ResourceManager::DestroyTexture(Handle<Texture> handle)
	{
		m_TexturePool.Destroy(handle);
	}

	void ResourceManager::DestroyBuffer(Handle<Buffer> handle)
	{
		m_BufferPool.Destroy(handle);
	}

	void ResourceManager::DestroyBindGroup(Handle<BindGroup> handle)
	{
		m_BindGroupPool.Destroy(handle);
	}

	void ResourceManager::DestroyShader(Handle<Shader> handle)
	{
		m_ShaderPool.Destroy(handle);
	}

	
}