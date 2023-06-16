#include "mfpch.h"
#include "Renderer.h"

namespace Magnefu
{
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::NONE:
			MF_CORE_ASSERT(false, "RendererAPI::API::NONE VertexBuffer not supported");
			return nullptr;
		}

		MF_CORE_ASSERT(false, "Unknown Renderer API - VertexBuffer");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size, float* data)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::NONE:
			MF_CORE_ASSERT(false, "RendererAPI::API::NONE VertexBuffer not supported");
			return nullptr;
		}

		MF_CORE_ASSERT(false, "Unknown Renderer API - VertexBuffer");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t count, uint32_t* data)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::NONE:
			MF_CORE_ASSERT(false, "RendererAPI::API::NONE IndexBuffer not supported");
			return nullptr;
		}

		MF_CORE_ASSERT(false, "Unknown Renderer API - IndexBuffer");
		return nullptr;
	}
}