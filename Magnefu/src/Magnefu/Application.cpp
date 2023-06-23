#include "mfpch.h"

#include "Application.h"
#include "Renderer/GraphicsContext.h"


//TEMP
#include "imgui/imgui.h"


namespace Magnefu
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

    Application* Application::s_Instance = nullptr;

	Application::Application()
	{
        MF_PROFILE_FUNCTION();
        MF_CORE_ASSERT(!s_Instance, "Application instance already exists.");
        s_Instance = this;

        {
            MF_PROFILE_SCOPE("Window Creation");
            m_Window = Scope<Window>(Window::Create());
            m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
        }
        m_Minimized = false;

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);

        m_ResourceCache = Scope<ResourceCache>(ResourceCache::Create());

        {
            MF_PROFILE_SCOPE("Renderer Init");
            Renderer::Init();
        }
	}

	Application::~Application()
	{
        
	}

    bool Application::OnWindowClose(WindowCloseEvent& e)
    {
        m_Running = false;
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& e)
    {
        if (e.GetWidth() == 0 || e.GetHeight() == 0)
        {
            m_Minimized = true;
            return false;
        }

        m_Minimized = false;
        return false;
    }

    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer* overlay)
    {
        m_LayerStack.PushOverlay(overlay);
        overlay->OnAttach();
    }


    void Application::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

        for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
        {
            (*--it)->OnEvent(event);
            if (event.IsHandled())
                break;
        }
    }

    void Application::OnUpdate(float deltaTime)
    {

    }

    // TODO: Make graphics context a child of application, not window
	void Application::Run()
	{        
        MF_PROFILE_FUNCTION();
        Scope<StackAllocator>& SingleFrameAllocator = StackAllocator::Get();

        // Maybe Do a unique pointer(Scope) as there should only be one
        GraphicsContext* GraphicsContext = m_Window->GetGraphicsContext();

        // TODO: Create setting to switch between locked and unlocked frame rate
        m_TimeStep.Init();

        while (m_Running)
        {
            //SingleFrameAllocator->Clear();
            MF_PROFILE_SCOPE("Run Loop");

                m_TimeStep.CalculateDeltaTime();

                // -- Poll and Handle Events -- //
                m_Window->OnUpdate();
                // -- ---------------------------------------- -- //



                // -- Start ImGui Frame -- //
                if (m_ImGuiLayer)
                    m_ImGuiLayer->BeginFrame();
                // -- ---------------------------------------- -- //



                // -- Game Logic And ImGui Widget Updates Here -- //
                for (Layer* layer : m_LayerStack)
                    layer->OnUpdate(m_TimeStep.GetDeltaTime());


                /*for (Layer* layer : m_LayerStack)
                {
                    layer->OnGUIRender();
                }*/

                OnGUIRender();


                // -- This constructs the draw data for the ImGui frame, but doesn't actually submit any draw commands. -- //
                if (m_ImGuiLayer)
                    m_ImGuiLayer->EndFrame();


                // -- Start Vulkan command buffer recording -- //
                // -- Issue draw calls for game objects and imgui widgets-- //
                GraphicsContext->DrawFrame();



        }   
        m_Window->OnFinish();
    }

    void Application::OnGUIRender()
    {
        MF_PROFILE_FUNCTION();
        ImGui::Begin("Application");
        //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        // Shouldn't do this calculation every frame
        //ImGui::Text("Frame time: %.3f ms/frame | Frame Rate: %.1f FPS", deltaTime * 1000.f, 1.f / deltaTime);
        if (ImGui::CollapsingHeader("CONTROLS", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("APP CONTROLS");
            ImGui::Bullet(); ImGui::Text("Close App:      ESC  |");
            ImGui::Separator();
            ImGui::Text("CAMERA CONTROLS");
            ImGui::Bullet(); ImGui::Text("Camera Left:    A    | Left Arrow");
            ImGui::Bullet(); ImGui::Text("Camera Right:   D    | Right Arrow");
            ImGui::Bullet(); ImGui::Text("Camera Forward: W    | Up Arrow");
            ImGui::Bullet(); ImGui::Text("Camera Back:    S    | Down Arrow");
            ImGui::Bullet(); ImGui::Text("Camera Up:      E    |");
            ImGui::Bullet(); ImGui::Text("Camera Down:    Q    |");
            ImGui::Bullet(); ImGui::Text("Camera Rotate:  Right-Click + Move Mouse");
            ImGui::Bullet(); ImGui::Text("Adjust FOV:     Rotate Middle Mouse Button");
        }
        ImGui::End();

       // m_Window->OnImGuiRender();
        //Renderer::OnImGuiRender();

#ifdef MF_DEBUG
        ImGui::Begin("Stats");

        if (ImGui::BeginTabBar("Stats", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("MEMORY"))
            {
                if (ImGui::CollapsingHeader("Stack Allocator", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    //ImGui::Text("Stack Allocator Capacity: %d", 2048);
                    //ImGui::Text("Avg Alloc Count(per frame, per min): %d", m_MemAllocData->AvgAllocsPerFramePerMin);
                    //ImGui::Text("Avg Alloc Size(per frame, per min): %d", m_MemAllocData->AvgBytePerFramePerMin);
                }
                if (ImGui::CollapsingHeader("Pool Allocator"))
                {

                }

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("TIMERS"))
            {
                
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
#endif
    }

      
}