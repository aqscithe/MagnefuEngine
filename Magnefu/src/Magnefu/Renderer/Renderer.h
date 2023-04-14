#pragma once

#include "RenderCommand.h"
#include "Material.h"
#include "Magnefu/Tests/Test.h"
#include "Magnefu/Core/Maths/Primitive2D.h"


namespace Magnefu
{
	class VertexArray;

	class Renderer
	{

	public:

		static void BeginScene();
		static void EndScene();
		static void Submit(const Ref<VertexArray>&, const Ref<Material>& material);
		static void DrawPlane(const Plane&);
		static void DrawCube(float size, const Maths::vec3& color);
		
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:

	};
}
