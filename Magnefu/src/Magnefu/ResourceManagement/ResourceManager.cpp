#include "mfpch.h"
#include "ResourceManager.h"

namespace Magnefu
{
	ResourceManager* ResourceManager::Create()
	{
		return new ResourceManager();
	}

	Handle<Buffer> ResourceManager::CreateBuffer(const BufferDesc& desc)
	{
		return m_BufferPool.Create(desc);
	}

	void ResourceManager::DestroyBuffer(Handle<Buffer> handle)
	{
		m_BufferPool.Destroy(handle);
	}
}