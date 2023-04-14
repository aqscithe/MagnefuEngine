#pragma once

#include "VertexArray.h"

namespace Magnefu
{
	class RendererAPI
	{
	public:
		enum class API
		{
			NONE = 0,
			OPENGL = 1
		};

	public:
		virtual void Clear() const = 0;
		virtual void ClearColor(const Maths::vec4& clearColor) const = 0;
		virtual void ClearColor(float r, float g, float b, float a) const = 0;
		virtual void EnableBlending() = 0;
		virtual void DisableBlending() = 0;
		virtual void SetWindowSize(int width, int height) = 0;
		virtual void DrawIndexed(const Ref<VertexArray>&) = 0;

		inline static API GetAPI() { return s_API; }


	private:
		static API s_API;
	};
}