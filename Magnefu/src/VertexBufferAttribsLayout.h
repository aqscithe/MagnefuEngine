#pragma once

#include <GLAD/glad.h>

#include <vector>
#include "Globals.h"

struct VertexBufferAttribute
{
	unsigned int type;
	unsigned int count;
	unsigned int normalized;

	static unsigned int GetSizeOfType(unsigned int type)
	{
		switch (type)
		{
			case GL_FLOAT: return 4;
			case GL_UNSIGNED_INT: return 4;
			case GL_UNSIGNED_BYTE: return 1;
		}
		ASSERT(false);
		return 0;
	}

};

class VertexBufferAttribsLayout
{
private:
	std::vector<VertexBufferAttribute> m_Attributes;
	unsigned int m_Stride;

public:
	VertexBufferAttribsLayout()
		: m_Stride(0) {}
	

	template<typename T>
	void Push(unsigned int count)
	{
		static_assert(true);
	}

	template<>
	void Push<float>(unsigned int count)
	{
		m_Attributes.push_back( { GL_FLOAT, count, GL_FALSE } );
		m_Stride += VertexBufferAttribute::GetSizeOfType(GL_FLOAT) * count;
	}

	template<>
	void Push<unsigned int>(unsigned int count)
	{
		m_Attributes.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
		m_Stride += VertexBufferAttribute::GetSizeOfType(GL_UNSIGNED_INT) * count;
	}

	template<>
	void Push<unsigned char>(unsigned int count)
	{
		m_Attributes.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
		m_Stride += VertexBufferAttribute::GetSizeOfType(GL_UNSIGNED_BYTE) * count;
	}


	inline const std::vector<VertexBufferAttribute> GetAttributes() const& { return m_Attributes; }
	inline unsigned int GetStride() const { return m_Stride; }

};