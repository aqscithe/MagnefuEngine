#pragma once

#include "RenderCommand.h"
#include "Texture.h"
#include "Shader.h"
#include "tests/Test.h"


namespace Magnefu
{
	class VertexArray;

	class Renderer
	{

	public:

		static void BeginScene(SceneData* data);
		static void EndScene();
		static void Submit(const Ref<VertexArray>&, const Ref<Shader>, Texture*);
		
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:

		static SceneData* m_SceneData;

	};
}
