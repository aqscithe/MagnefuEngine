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
		void EnableBlending() override;
		void DisableBlending() override;
		void SetWindowSize(int width, int height) override;
		void DrawIndexed(const Ref<VertexArray>&) override;
	};
}