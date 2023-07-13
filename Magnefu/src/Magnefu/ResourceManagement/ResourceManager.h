#pragma once

#include "Magnefu/Renderer/Buffer.h"
#include "Magnefu/Renderer/Texture.h"
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


		Handle<Buffer> CreateBuffer(const BufferDesc&);

		Buffer& GetBuffer(Handle<Buffer>& handle);

		void DestroyBuffer(Handle<Buffer> handle);
		//void destroyShader(Handle<Shader> handle);
		//void destroyTexture(Handle<Texture> handle);
		//void destroyBindGroup(Handle<BindGroup> handle);

	private:
		Pool<Buffer> m_BufferPool;
		//Pool<Shader> shaderPool;
		//Pool<Texture> texturePool;
		//Pool<BindGroup> bindGroupPool;
	};
}