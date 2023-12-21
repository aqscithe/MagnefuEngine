
// -- PCH -- //
#include "mfpch.h"

// -- .h -- //
#include "Application.h"

// -- Application Includes ---------//


// -- Graphics Includes --------------------- //
#include "Magnefu/Graphics/GraphicsContext.h"

// -- Core Includes ---------------------------------- //




//TEMP
#include "imgui/imgui.h"


namespace Magnefu
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

    //static const int THREAD_POOL_WORKERS = 2;

    Application* Application::s_Instance = nullptr;

    


	Application::Application()
	{
        MF_PROFILE_FUNCTION();


        // Set Application Instance
        MF_CORE_ASSERT(!s_Instance, "Application instance already exists.");
        s_Instance = this;


        // -- Create Managers ------------------------------------------- //

        // Create Resource Manager

        // Create Scene Manager
        m_SceneManager = Scope<SceneManager>(SceneManager::Create());

        {
            MF_PROFILE_SCOPE("Window Creation");
            m_Window = Scope<Window>(Window::Create());
            m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

        }


        PushOverlay(m_ImGuiLayer);

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


	void Application::Run()
	{        
        MF_PROFILE_FUNCTION();

        GraphicsContext* GraphicsContext = m_Window->GetGraphicsContext();


        // TODO: Create setting to switch between locked and unlocked frame rate
        //m_TimeStep.Init();

        auto lastTime = Magnefu::time_now();


       MF_CORE_INFO("STARTING FIRST LOOP");

        while (m_Running)
        {
            MF_PROFILE_SCOPE("Run Loop");

            //m_TimeStep.CalculateDeltaTime();
            auto currentTime = Magnefu::time_now();
            auto deltaTime = Magnefu::time_delta_milliseconds(lastTime, currentTime);
            lastTime = currentTime;

            // -- Poll and Handle Events -- //
            m_Window->OnUpdate();


            // -- Start ImGui Frame -- //
            if (m_ImGuiLayer)
                m_ImGuiLayer->BeginFrame();


            // -- Game Logic Here -- //
            for (Layer* layer : m_LayerStack)
                layer->OnUpdate(deltaTime);


            // -- ImGui Widget Updates -- //
            for (Layer* layer : m_LayerStack)
                layer->OnGUIRender();


            // -- End ImGui Frame -- //
            // This constructs the draw data for the ImGui frame, 
            // but doesn't actually submit any draw commands. 
            if (m_ImGuiLayer)
                m_ImGuiLayer->EndFrame();


            // -- Start Vulkan command buffer recording -- //
            // -- Issue draw calls for game objects and imgui widgets-- //
            GraphicsContext->DrawFrame();

        }   
        m_Window->OnFinish();

        delete GraphicsContext;
    }

      
}