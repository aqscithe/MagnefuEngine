#pragma once

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "Texture.h"
#include "Shader.h"

class Renderer
{
private:

public:

	void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
	void DrawQuads(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
	void DrawCube(const VertexArray& vao, const Shader& shader) const;
	void Clear() const;
	void ClearColor(const Maths::vec4& clearColor) const;
	void ClearColor(float r, float g, float b, float a) const;
	void EnableDepthTest() const;
	void DisableDepthTest() const;
};