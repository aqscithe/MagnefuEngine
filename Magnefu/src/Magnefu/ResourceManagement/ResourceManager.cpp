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

	Texture& ResourceManager::GetTexture(Handle<Texture>& handle)
	{
		return m_TexturePool.Get(handle);
	}

	Buffer& ResourceManager::GetBuffer(Handle<Buffer>& handle)
	{
		return m_BufferPool.Get(handle);
	}

	void ResourceManager::DestroyTexture(Handle<Texture> handle)
	{
		m_TexturePool.Destroy(handle);
	}

	void ResourceManager::DestroyBuffer(Handle<Buffer> handle)
	{
		m_BufferPool.Destroy(handle);
	}

	
}