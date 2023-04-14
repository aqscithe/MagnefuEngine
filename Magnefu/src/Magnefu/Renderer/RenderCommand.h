#pragma once

#include "RendererAPI.h"


namespace Magnefu
{
	class RenderCommand
	{
	public:
		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		inline static void ClearColor(const Maths::vec4& clearColor)
		{
			s_RendererAPI->ClearColor(clearColor);
		}

		inline static void ClearColor(float r, float g, float b, float a)
		{
			s_RendererAPI->ClearColor(r, g, b, a);
		}

		inline static void EnableBlending()
		{
			s_RendererAPI->EnableBlending();
		}

		inline static void DisableBlending()
		{
			s_RendererAPI->DisableBlending();
		}

		inline static void EnableDepthTest()
		{
			s_RendererAPI->EnableDepthTest();
		}

		inline static void DisableDepthTest()
		{
			s_RendererAPI->DisableDepthTest();
		}

		inline static void SetWindowSize(int width, int height)
		{
			s_RendererAPI->SetWindowSize(width, height);
		}

		inline static void DrawIndexed(const Ref<VertexArray>& va)
		{
			va->Bind();
			s_RendererAPI->DrawIndexed(va);
		}

	private:
		static RendererAPI* s_RendererAPI;

	};
}