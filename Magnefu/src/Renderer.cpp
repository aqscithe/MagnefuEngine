#include "mfpch.h"

#include "Renderer.h"
#include <GLAD/glad.h>


void Renderer::Draw(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader) const
{
    shader.Bind();

    vao.Bind();
    ibo.Bind();

    glDrawElements(GL_TRIANGLES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr);
}


void Renderer::DrawCube(const VertexArray& vao, const Shader& shader) const
{
    shader.Bind();
    vao.Bind();

    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void Renderer::Clear() const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::ClearColor(Maths::vec4& clearColor) const
{
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
}

void Renderer::EnableDepthTest() const
{
    MF_CORE_INFO("RENDERER -- Enabling Depth Test");
    glEnable(GL_DEPTH_TEST);
}

void Renderer::DisableDepthTest() const
{
    MF_CORE_INFO("RENDERER -- Disabling Depth Test");
    glDisable(GL_DEPTH_TEST);
}
