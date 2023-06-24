#include "mfpch.h"

#include "VertexArray.h"
#include "Buffer.h"
#include "Renderer.h"


namespace Magnefu
{
	std::shared_ptr<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::NONE:
			MF_CORE_ASSERT(false, "RendererAPI::NONE VertexArray not supported");
			return nullptr;
		}

		MF_CORE_ASSERT(false, "Unknown Renderer API - VertexArray");
		return nullptr;
	}

}




