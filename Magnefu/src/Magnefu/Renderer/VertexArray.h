#pragma once

namespace Magnefu
{
	class VertexBuffer;

	class VertexArray
	{
	public:
		virtual ~VertexArray() = default;

		virtual void Unbind() const = 0;
		virtual void Bind()   const = 0;

		virtual void AddVertexBuffer(const VertexBuffer*) = 0;

		static VertexArray* Create();

	};
}