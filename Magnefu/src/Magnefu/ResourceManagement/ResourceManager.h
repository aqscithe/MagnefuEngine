#pragma once

#include "Magnefu/Renderer/Buffer.h"
#include "Magnefu/Renderer/Texture.h"
#include "Magnefu/Renderer/BindGroup.h"
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


		Handle<Texture> CreateTexture(const TextureDesc&);
		Handle<Buffer> CreateBuffer(const BufferDesc&);
		Handle<BindGroup> CreateBindGroup(const BindGroupDesc&);

		Texture& GetTexture(Handle<Texture>& handle);
		Buffer& GetBuffer(Handle<Buffer>& handle);
		BindGroup& GetBindGroup(Handle<BindGroup>& handle);

		void DestroyTexture(Handle<Texture> handle);
		void DestroyBuffer(Handle<Buffer> handle);
		void DestroyBindGroup(Handle<BindGroup> handle);
		//void destroyShader(Handle<Shader> handle);
		

	private:
		Pool<Texture> m_TexturePool;
		Pool<Buffer> m_BufferPool;
		Pool<BindGroup> m_BindGroupPool;
		//Pool<Shader> shaderPool;
		
	};
}