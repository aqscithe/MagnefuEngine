#pragma once

#include "Magnefu/Renderer/Buffer.h"


namespace Magnefu
{
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(uint32_t size, float* data = nullptr);
		~OpenGLVertexBuffer();

		void Bind() const override;
		void Unbind() const override;

		inline void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
		inline const BufferLayout& GetLayout() const override { return m_Layout; }

	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32_t count, uint32_t* data);
		~OpenGLIndexBuffer();

		void Bind() const override;
		void Unbind() const override;

		inline uint32_t GetCount() const override { return m_Count; }

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;

	};
}