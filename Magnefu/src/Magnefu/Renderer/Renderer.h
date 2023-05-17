#pragma once

#include "RenderCommand.h"
#include "Material.h"
#include "Magnefu/Core/Maths/PrimitiveCommon.h"


namespace Magnefu
{
	class VertexArray;

	class Renderer
	{

	public:

		static void Init();

		static void BeginScene();
		static void EndScene();
		static void Submit(const Ref<VertexArray>&, const Ref<Material>& material);
		static void DrawPlane(const PrimitiveData& data = { {20.f, 20.f, 0.1f}, {0.4f, 0.4f, 0.4f}, {-1.f, 0.f, 0.f}, 90.f});
		static void DrawCube(const PrimitiveData& data = { {1.f, 1.f, 1.f}, {1.f, 0.5f, 0.85f}, {0.f, 0.f, 0.f}, 90.f });
		static void DrawRectangularPrism(const PrimitiveData& data = { {1.f, 1.f, 1.f}, {1.f, 0.5f, 0.85f}, {0.f, 0.f, 0.f}, 90.f });
		static void DrawSphere(const SphereData& data = { {0.1f, 0.73f, 0.4f}, {0.f, 0.f, 0.f}, 0.f, 1.f, 32, 13 });
		static void DrawIcoSphere(const IcoSphereData& data = {0.5f, 1, {0.1f, 0.73f, 0.4f}, {0.f, 0.f, 0.f}, 0.f });
		static void DrawCubeSphere();
		static void DrawTriangularPyramid();
		static void DrawSkybox();

		static void OnImGuiRender();
		
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:

	};
}
