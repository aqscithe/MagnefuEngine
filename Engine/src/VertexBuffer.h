#pragma once

class VertexBuffer
{
private:
	unsigned int m_RendererID;

public:
	VertexBuffer(unsigned int size, const void* data = nullptr);

	~VertexBuffer();

	void Bind() const;
	void Unbind() const;
};