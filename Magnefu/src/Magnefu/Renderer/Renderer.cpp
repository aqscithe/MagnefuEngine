#include "mfpch.h"

#include "Renderer.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

#include <GLAD/glad.h>


namespace Magnefu
{
    SceneData* Renderer::m_SceneData = nullptr;

    void Renderer::BeginScene(SceneData* data)
    {
        m_SceneData = data;
    }

    void Renderer::EndScene()
    {
    }

    void Renderer::Submit(const Ref<VertexArray>& va, const Ref<Shader> shader, Texture* texture )
    {
        shader->Bind();
        shader->SetUniformMatrix4fv("u_MVP", m_SceneData->MVP);
        texture->Bind();
        RenderCommand::DrawIndexed(va);
    }

}


