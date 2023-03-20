#include "mfpch.h"
#include "ResourceCache.h"



namespace Magnefu
{

	ResourceCache* ResourceCache::Create()
	{
		MF_CORE_TRACE("Initializing Resource Cache.");
		return new ResourceCache();
	}
}