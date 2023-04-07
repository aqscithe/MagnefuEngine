#pragma once

#include "RenderCommand.h"
#include "Material.h"
#include "Magnefu/Tests/Test.h"


namespace Magnefu
{
	class VertexArray;

	class Renderer
	{

	public:

		static void BeginScene();
		static void EndScene();
		static void Submit(const Ref<VertexArray>&, const Ref<Material>& material);
		
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:

	};
}
