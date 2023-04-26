#pragma once

#include "Magnefu/Renderer/RendererAPI.h"


namespace Magnefu
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		void Clear() const override;
		void ClearColor(const Maths::vec4& clearColor) const override;
		void ClearColor(float r, float g, float b, float a) const override;
		void EnableFaceCulling() override;
		void DisableFaceCulling() override;
		void EnableSeamlessCubeMap() override;
		void DisableSeamlessCubeMap() override;
		void EnableBlending() override;
		void DisableBlending() override;
		void EnableDepthTest() override;
		void DisableDepthTest() override;
		void EnableDepthMask() override;
		void DisableDepthMask() override;
		void DepthFuncLEqual() override;
		void DepthFuncLess() override;
		void FrontFaceCCW() override;
		void FrontFaceCW() override;
		void SetWindowSize(int width, int height) override;
		void DrawIndexed(const Ref<VertexArray>&) override;
	};
}