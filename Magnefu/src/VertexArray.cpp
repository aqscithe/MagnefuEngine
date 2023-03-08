#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferAttribsLayout.h"
#include "Globals.h"
#include <GL/glew.h>

VertexArray::VertexArray()
{
	glGenVertexArrays(1, &m_RendererID);
	glBindVertexArray(m_RendererID);
}

VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &m_RendererID);
}

void VertexArray::Unbind() const
{
	glBindVertexArray(0);
}

void VertexArray::Bind() const
{
	glBindVertexArray(m_RendererID);
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
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, attribute.count, attribute.type, attribute.normalized, stride, (const void*)offset);

		offset += attribute.count * VertexBufferAttribute::GetSizeOfType(attribute.type);
		index += 1;
	}

}


