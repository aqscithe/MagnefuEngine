#include "mfpch.h"
#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Renderer.h"

namespace Magnefu
{
	std::shared_ptr<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::NONE:
			MF_CORE_ASSERT(false, "RendererAPI::NONE VertexBuffer not supported");
			return nullptr;
		case RendererAPI::OPENGL:
			return std::make_shared<OpenGLVertexBuffer>(size);
		}

		MF_CORE_ASSERT(false, "Unknown Renderer API - VertexBuffer");
		return nullptr;
	}

	std::shared_ptr<VertexBuffer> VertexBuffer::Create(uint32_t size, float* data)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::NONE:
			MF_CORE_ASSERT(false, "RendererAPI::NONE VertexBuffer not supported");
			return nullptr;
		case RendererAPI::OPENGL:
			return std::make_shared< OpenGLVertexBuffer>(size, data);
		}

		MF_CORE_ASSERT(false, "Unknown Renderer API - VertexBuffer");
		return nullptr;
	}

	std::shared_ptr<IndexBuffer> IndexBuffer::Create(uint32_t count, uint32_t* data)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::NONE:
			MF_CORE_ASSERT(false, "RendererAPI::NONE IndexBuffer not supported");
			return nullptr;
		case RendererAPI::OPENGL:
			return std::make_shared<OpenGLIndexBuffer>(count, data);
		}

		MF_CORE_ASSERT(false, "Unknown Renderer API - IndexBuffer");
		return nullptr;
	}
}