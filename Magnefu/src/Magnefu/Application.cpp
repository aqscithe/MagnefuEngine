#include "mfpch.h"

#include "Application.h"
#include "Magnefu/Core/TimeStep.h"

//TEMP
#include "imgui/imgui.h"


namespace Magnefu
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

    Application* Application::s_Instance = nullptr;

	Application::Application()
	{
        MF_CORE_INFO("HELLO FROM APPLICATION");
        MF_CORE_ASSERT(!s_Instance, "Application instance already exists.");
        s_Instance = this;
        m_Window = Scope<Window>(Window::Create());
        m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);

        m_MemAllocData = StackAllocator::Get()->GetMemAllocData().get();
        m_ResourceCache = Scope<ResourceCache>(ResourceCache::Create());
        m_Minimized= false;
	}

	Application::~Application()
	{
        if (m_ImGuiLayer)
            delete m_ImGuiLayer;
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
#ifdef MF_DEBUG
        m_MemAllocData->Timer -= deltaTime;
        m_MemAllocData->FrameCounter++;

        if (m_MemAllocData->Timer <= 0)
        {
            m_MemAllocData->Timer = 30000.f;
            m_MemAllocData->AvgAllocsPerFramePerMin = m_MemAllocData->AllocationCounter / m_MemAllocData->FrameCounter;
            m_MemAllocData->AvgBytePerFramePerMin = m_MemAllocData->TotalBytesAllocated / m_MemAllocData->FrameCounter;
            m_MemAllocData->TotalBytesAllocated = 0;
            m_MemAllocData->AllocationCounter = 0;
            m_MemAllocData->FrameCounter = 0;
        }
#endif
    }


	void Application::Run()
	{        
        Scope<StackAllocator>& SingleFrameAllocator = StackAllocator::Get();
        TimeStep ts;

        while (m_Running)
        {
            SingleFrameAllocator->Clear();

            ts.CalculateDeltaTime();
            while (ts.TimeLeftInFrame())
            {
                m_Window->OnUpdate();

                for (Layer* layer : m_LayerStack)
                    layer->OnUpdate(ts.GetDeltaTime());

                ts.DecrementTime();
                // integration formulas to consult when you start physics simulation
                // https://gafferongames.com/post/fix_your_timestep/
            }
            
            OnUpdate(ts.CalculateInterpolationCoeff());
            
            if (m_ImGuiLayer)
                m_ImGuiLayer->BeginFrame();


            for (Layer* layer : m_LayerStack)
                layer->OnRender(ts.CalculateInterpolationCoeff());
            

            OnImGuiRender();

            if (m_ImGuiLayer)
                m_ImGuiLayer->EndFrame();

        }   
    }

    void Application::OnImGuiRender()
    {
        ImGui::Begin("Application");
        //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        // Shouldn't do this calculation every frame
        //ImGui::Text("Frame time: %.3f ms/frame | Frame Rate: %.1f FPS", deltaTime * 1000.f, 1.f / deltaTime);
        ImGui::Separator();
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

#ifdef MF_DEBUG
        ImGui::Begin("Stats");

        if (ImGui::BeginTabBar("Stats", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("MEMORY"))
            {
                if (ImGui::CollapsingHeader("Stack Allocator", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Text("Stack Allocator Capacity: %d", 2048);
                    ImGui::Text("Avg Alloc Count(per frame, per min): %d", m_MemAllocData->AvgAllocsPerFramePerMin);
                    ImGui::Text("Avg Alloc Size(per frame, per min): %d", m_MemAllocData->AvgBytePerFramePerMin);
                }
                if (ImGui::CollapsingHeader("Pool Allocator"))
                {

                }

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
#endif
    }

      
}