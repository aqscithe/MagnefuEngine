#pragma once

#include "Magnefu/Core.h"
#include "Magnefu/Log.h"
#include "Magnefu/Renderer/VertexArray.h"

#include "GLAD/glad.h"

#include <vector>


namespace Magnefu
{
	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		~OpenGLVertexArray();

		void Bind() const override;
		void Unbind() const override;

		void AddVertexBuffer(const VertexBuffer*) override;

	private:
		uint32_t m_RendererID;
		const VertexBuffer* m_VBO;
	};
}