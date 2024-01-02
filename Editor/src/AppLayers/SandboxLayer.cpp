// -- HEADER -- //
#include "SandboxLayer.hpp"

// -- Application ------------- //

// -- Graphics ---------------- //



// -- Core -------------- //



SandboxLayer::SandboxLayer() : Layer("Sandbox")
{


}


// Do I really need an on attach and ondetach?
// Why not just use constructor and destructor?
void SandboxLayer::OnAttach()
{
    using namespace Magnefu;

    
}

void SandboxLayer::OnDetach()
{
}

void SandboxLayer::OnEvent(Magnefu::Event& e)
{
}

void SandboxLayer::Update(float deltatime)
{
}

void SandboxLayer::DrawGUI()
{
}
