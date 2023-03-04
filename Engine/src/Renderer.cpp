#include "Renderer.h"
#include "Globals.h"
#include <GL/glew.h>

void Renderer::Draw(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader) const
{
    shader.Bind();

    vao.Bind();
    ibo.Bind();

    GLCall(glDrawElements(GL_TRIANGLES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr));
}


void Renderer::DrawCube(const VertexArray& vao, const Shader& shader) const
{
    shader.Bind();
    vao.Bind();

    GLCall(glDrawArrays(GL_TRIANGLES, 0, 36));
}

void Renderer::Clear() const
{
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Renderer::ClearColor(Maths::vec4& clearColor) const
{
    GLCall(glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a));
}

void Renderer::EnableDepthTest() const
{
    GLCall(glEnable(GL_DEPTH_TEST));
}

void Renderer::DisableDepthTest() const
{
    GLCall(glDisable(GL_DEPTH_TEST));
}
