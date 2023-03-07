#pragma once

class IndexBuffer
{
private:
	unsigned int m_RendererID;
	unsigned int m_Count;

public:
	IndexBuffer(unsigned int count, const unsigned int* data = nullptr);

	~IndexBuffer();

	void Bind() const;
	void Unbind() const;

	void InvalidateBuffer() const;
	
	inline unsigned int GetCount() const { return m_Count; }
};