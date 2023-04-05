#include "mfpch.h"
#include "OpenGLRendererAPI.h"
#include "Magnefu/Renderer/Buffer.h"

#include "GLAD/glad.h"

namespace Magnefu
{
	void OpenGLRendererAPI::Clear() const
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::ClearColor(const Maths::vec4& color) const
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::ClearColor(float r, float g, float b, float a) const
	{
		glClearColor(r, g, b, a);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& va)
	{
		glDrawElements(GL_TRIANGLES, va->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}
}