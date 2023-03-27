#pragma once

namespace Magnefu
{
	class VertexBuffer;
	class IndexBuffer;

	class VertexArray
	{
	public:
		virtual ~VertexArray() = default;

		virtual void Unbind() const = 0;
		virtual void Bind()   const = 0;

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>&) = 0;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>&) = 0;

		static std::shared_ptr<VertexArray> Create();

	};
}