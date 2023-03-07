#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferAttribsLayout.h"
#include "Globals.h"
#include <GL/glew.h>

VertexArray::VertexArray()
{
	GLCall(glGenVertexArrays(1, &m_RendererID));
	GLCall(glBindVertexArray(m_RendererID));
}

VertexArray::~VertexArray()
{
	GLCall(glDeleteVertexArrays(1, &m_RendererID));
}

void VertexArray::Unbind() const
{
	GLCall(glBindVertexArray(0));
}

void VertexArray::Bind() const
{
	GLCall(glBindVertexArray(m_RendererID));
}

void VertexArray::AddBuffer(const VertexBuffer& vbo, const VertexBufferAttribsLayout& layout)
{
	Bind();
	vbo.Bind();

	const auto& attributes = layout.GetAttributes();
	auto stride = layout.GetStride();

	unsigned int offset = 0;

	int index = 0;
	for (const auto& attribute : attributes)
	{
		GLCall(glEnableVertexAttribArray(index));
		GLCall(glVertexAttribPointer(index, attribute.count, attribute.type, attribute.normalized, stride, (const void*)offset));

		offset += attribute.count * VertexBufferAttribute::GetSizeOfType(attribute.type);
		index += 1;
	}

}


