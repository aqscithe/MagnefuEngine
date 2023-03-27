#include "mfpch.h"

#include "VertexArray.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
#include "Renderer.h"


namespace Magnefu
{
	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::NONE:
			MF_CORE_ASSERT(false, "RendererAPI::NONE VertexArray not supported");
			return nullptr;
		case RendererAPI::OPENGL:
			return new OpenGLVertexArray();
		}

		MF_CORE_ASSERT(false, "Unknown Renderer API - VertexArray");
		return nullptr;
	}

}




