#pragma once

#include "RenderCommand.h"
#include "Material.h"
#include "Magnefu/Tests/Test.h"
#include "Magnefu/Core/Maths/PrimitiveCommon.h"


namespace Magnefu
{
	class VertexArray;

	class Renderer
	{

	public:

		static void BeginScene();
		static void EndScene();
		static void Submit(const Ref<VertexArray>&, const Ref<Material>& material);
		static void DrawPlane(const PrimitiveData& data = { {10.f, 10.f, 0.1f}, {0.4f, 0.4f, 0.4f}, {-1.f, 0.f, 0.f}, 90.f});
		static void DrawCube(const PrimitiveData& data = { {1.f, 1.f, 1.f}, {1.f, 0.5f, 0.85f}, {0.f, 0.f, 0.f}, 90.f });
		static void DrawSphere(const SphereData& data = { {0.1f, 0.73f, 0.4f}, {0.f, 0.f, 0.f}, 0.f, 1.f, 32, 13 });
		
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:

	};
}
