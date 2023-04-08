#pragma once

#include "Magnefu/Core/Assertions.h"
#include "Magnefu/Core/Log.h"
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

		void AddVertexBuffer(const Ref<VertexBuffer>&) override;
		void SetIndexBuffer(const Ref<IndexBuffer>&) override;

		const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		const Ref<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }

	private:
		uint32_t m_RendererID;
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;
	};
}