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
		static void DrawPlane(const Maths::vec2& size, const Maths::vec3& color);
		//static void DrawCube()
		
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:

	};
}
