#include "mfpch.h"

#include "Renderer.h"
#include "Buffer.h"
#include "VertexArray.h"

#include <GLAD/glad.h>


namespace Magnefu
{
    void Renderer::BeginScene()
    {
        // shader & texture bindings
        // Get shader uniforms
        // MVP matrix calculations
        // & more
    }

    void Renderer::EndScene()
    {
    }

    void Renderer::Submit(const Ref<VertexArray>& va)
    {
        RenderCommand::DrawIndexed(va);
    }

}


