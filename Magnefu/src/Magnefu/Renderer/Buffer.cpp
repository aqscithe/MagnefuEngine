#include "mfpch.h"
//#include "Magnefu/Core.h"
#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Renderer.h"

namespace Magnefu
{
	VertexBuffer* VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::NONE:
			MF_CORE_ASSERT(false, "RendererAPI::NONE VertexBuffer not supported");
			return nullptr;
		case RendererAPI::OPENGL:
			return new OpenGLVertexBuffer(size);
		}

		MF_CORE_ASSERT(false, "Unknown VertexBuffer type.");
		return nullptr;
	}

	VertexBuffer* VertexBuffer::Create(uint32_t size, float* data)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::NONE:
			MF_CORE_ASSERT(false, "RendererAPI::NONE VertexBuffer not supported");
			return nullptr;
		case RendererAPI::OPENGL:
			return new OpenGLVertexBuffer(size, data);
		}

		MF_CORE_ASSERT(false, "Unknown VertexBuffer type.");
		return nullptr;
	}

	IndexBuffer* Magnefu::IndexBuffer::Create(uint32_t count, uint32_t* data)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::NONE:
			MF_CORE_ASSERT(false, "RendererAPI::NONE IndexBuffer not supported");
			return nullptr;
		case RendererAPI::OPENGL:
			return new OpenGLIndexBuffer(count, data);
		}

		MF_CORE_ASSERT(false, "Unknown IndexBuffer type.");
		return nullptr;
	}
}