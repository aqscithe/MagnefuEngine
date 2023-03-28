#pragma once

#include "RenderCommand.h"
#include "Texture.h"
#include "Shader.h"


namespace Magnefu
{
	class VertexArray;

	class Renderer
	{

	public:

		static void BeginScene();
		static void EndScene();
		static void Submit(const Ref<VertexArray>&);
		
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:

	};
}
