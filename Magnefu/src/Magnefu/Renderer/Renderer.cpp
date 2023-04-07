#include "mfpch.h"

#include "Renderer.h"
#include "Buffer.h"
#include "VertexArray.h"


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

}


