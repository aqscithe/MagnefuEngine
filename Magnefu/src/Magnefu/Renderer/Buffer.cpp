#include "mfpch.h"
#include "Platform/OpenGL/OpenGLBuffer.h"
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
		case RendererAPI::API::OPENGL:
			return CreateRef<OpenGLVertexBuffer>(size);
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
		case RendererAPI::API::OPENGL:
			return CreateRef< OpenGLVertexBuffer>(size, data);
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
		case RendererAPI::API::OPENGL:
			return CreateRef<OpenGLIndexBuffer>(count, data);
		}

		MF_CORE_ASSERT(false, "Unknown Renderer API - IndexBuffer");
		return nullptr;
	}
}