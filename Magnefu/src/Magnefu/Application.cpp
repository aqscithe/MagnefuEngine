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
        m_Window = std::unique_ptr<Window>(Window::Create());
        m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);

        m_ResourceCache = std::unique_ptr<ResourceCache>(ResourceCache::Create());
	}

	Application::~Application()
	{
        if (m_ImGuiLayer)
            delete m_ImGuiLayer;
	}

    bool  Application::OnWindowClose(WindowCloseEvent& event)
    {
        m_Running = false;
        return true;
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

        for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
        {
            (*--it)->OnEvent(event);
            if (event.IsHandled())
                break;
        }
    }


	void Application::Run()
	{        
        TimeStep ts;
            
        while (m_Running)
        {
            ts.CalculateDeltaTime();

            if (m_ImGuiLayer)
                m_ImGuiLayer->BeginFrame();

            while (ts.TimeLeftInFrame())
            {
                m_Window->OnUpdate();

                for (Layer* layer : m_LayerStack)
                    layer->OnUpdate(ts.GetDeltaTime());

                ts.DecrementTime();
                // integration formulas to consult when you start physics simulation
                // https://gafferongames.com/post/fix_your_timestep/
            }
            
            const float alpha = ts.CalculateInterpolationCoeff();

            m_RenderState.MVP = (m_CurrState.MVP * alpha) +
                (m_PrevState.MVP * (1.0f - alpha));

            for (Layer* layer : m_LayerStack)
                layer->OnRender();
            
            OnImGuiRender();

            if (m_ImGuiLayer)
                m_ImGuiLayer->EndFrame();

        }   
    }

    void Application::OnImGuiRender()
    {
        ImGui::Begin("Application");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        // Shouldn't do this calculation every frame
        //ImGui::Text("Frame time: %.3f ms/frame | Frame Rate: %.1f FPS", deltaTime * 1000.f, 1.f / deltaTime);
        ImGui::Separator();
        if (ImGui::TreeNodeEx("CONTROLS", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("APP CONTROLS");
            ImGui::Bullet(); ImGui::Text("Close App:      ESC  |");
            ImGui::Text("CAMERA CONTROLS");
            ImGui::Bullet(); ImGui::Text("Camera Left:    A    | Left Arrow");
            ImGui::Bullet(); ImGui::Text("Camera Right:   D    | Right Arrow");
            ImGui::Bullet(); ImGui::Text("Camera Forward: W    | Up Arrow");
            ImGui::Bullet(); ImGui::Text("Camera Back:    S    | Down Arrow");
            ImGui::Bullet(); ImGui::Text("Camera Up:      E    |");
            ImGui::Bullet(); ImGui::Text("Camera Down:    Q    |");
            ImGui::Bullet(); ImGui::Text("Camera Rotate:  Right-Click + Move Mouse");
            ImGui::Bullet(); ImGui::Text("Adjust FOV:     Rotate Middle Mouse Button");
            ImGui::Separator();
            ImGui::TreePop();
        }
        ImGui::End();
    }

      
}