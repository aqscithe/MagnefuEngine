#include "mfpch.h"

#include "Renderer.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Magnefu/Core/Maths/Quaternion.h"
#include "Magnefu/Application.h"


namespace Magnefu
{
    void Renderer::BeginScene()
    {
    }

    void Renderer::EndScene()
    {
    }

    void Renderer::Submit(const Ref<VertexArray>& va, const Ref<Material>& material )
    {
        // When a renderer binds a material, all the necessary uniforms from the renderer and the material
        // in question and upload the values in one contiguous buffer.
        material->Bind();
        RenderCommand::DrawIndexed(va);
    }

    void Renderer::DrawPlane(const Maths::vec2& size, const Maths::vec3& color)
    {
        float vertices[40] = {
            //  position    normals          color                         texture coords
            -0.5f, -0.5f,   0.f, 0.f, 1.f,   color.r, color.g, color.b,    0.f,  0.f,       // 0  BL
             0.5f, -0.5f,   0.f, 0.f, 1.f,   color.r, color.g, color.b,    1.f,  0.f,       // 1  BR
             0.5f,  0.5f,   0.f, 0.f, 1.f,   color.r, color.g, color.b,    1.f,  1.f,       // 2  TR
            -0.5f,  0.5f,   0.f, 0.f, 1.f,   color.r, color.g, color.b,    0.f,  1.f,       // 3  TL
        };

        uint32_t indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        Ref<VertexBuffer> vbo = VertexBuffer::Create(sizeof(vertices), vertices);

        BufferLayout layout = {
            {ShaderDataType::Float2, "aPosition"},
            {ShaderDataType::Float3, "aNormal"},
            {ShaderDataType::Float3, "aColor"},
            {ShaderDataType::Float2, "aTexCoords"}
        };

        vbo->SetLayout(layout);

        Ref<IndexBuffer> ibo = IndexBuffer::Create(sizeof(indices) / sizeof(uint32_t), indices);

        Ref<VertexArray> vao = VertexArray::Create();
        vao->AddVertexBuffer(vbo);
        vao->SetIndexBuffer(ibo);

        Ref<Shader> shader = Shader::Create("res/shaders/Plane.shader");

        Maths::mat4 MVP = 
            Application::Get().GetWindow().GetSceneCamera()->CalculateVP() * 
            Maths::translate({0.f, -1.f, 0.f}) * 
            Maths::Quaternion::GetRotationMatrix(90.f, {-1.f, 0.f, 0.f}) * 
            Maths::scale({size.x, size.y, 0.1f});
        

        shader->Bind();
        shader->SetUniformMatrix4fv("u_MVP", MVP);

        RenderCommand::DrawIndexed(vao);
    }

}


