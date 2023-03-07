#pragma once

class VertexBuffer;
class VertexBufferAttribsLayout;

class VertexArray
{
private:
	unsigned int m_RendererID;

public:
	VertexArray();
	~VertexArray();

	void Unbind() const;
	void Bind()   const;

	void AddBuffer(const VertexBuffer& vb, const VertexBufferAttribsLayout& layout );
};