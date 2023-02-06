#pragma once

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "Shader.h"
#include "Vectors.h"

class Renderer
{
private:

public:

	void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
	void DrawCube(const VertexArray& vao, const Shader& shader) const;
	void Clear() const;
	void ClearColor(Maths::vec4& clearColor) const;
};