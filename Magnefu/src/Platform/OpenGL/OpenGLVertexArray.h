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

		void AddVertexBuffer(const std::shared_ptr<VertexBuffer>&) override;
		void SetIndexBuffer(const std::shared_ptr<IndexBuffer>&) override;

	private:
		uint32_t m_RendererID;
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
	};
}