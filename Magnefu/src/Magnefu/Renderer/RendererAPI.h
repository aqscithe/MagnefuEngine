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
			OPENGL = 1,
			VULKAN = 2,
		};

	public:
		virtual void Clear() const = 0;
		virtual void ClearColor(const Maths::vec4& clearColor) const = 0;
		virtual void ClearColor(float r, float g, float b, float a) const = 0;
		virtual void EnableFaceCulling() = 0;
		virtual void DisableFaceCulling() = 0;
		virtual void EnableSeamlessCubeMap() = 0;
		virtual void DisableSeamlessCubeMap() = 0;
		virtual void EnableBlending() = 0;
		virtual void DisableBlending() = 0;
		virtual void EnableDepthTest() = 0;
		virtual void DisableDepthTest() = 0;
		virtual void EnableDepthMask() = 0;
		virtual void DisableDepthMask() = 0;
		virtual void DepthFuncLEqual() = 0;
		virtual void DepthFuncLess() = 0;
		virtual void FrontFaceCW() = 0;
		virtual void FrontFaceCCW() = 0;
		virtual void SetWindowSize(int width, int height) = 0;
		virtual void DrawIndexed(const Ref<VertexArray>&) = 0;

		inline static API GetAPI() { return s_API; }


	private:
		static API s_API;
	};
}