#include "mfpch.h"

#include "Application.h"
#include "Renderer/GraphicsContext.h"



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

        // -- Init Services ------------------------------------------- //
        
        // Start Memory Service
        MemoryService::instance()->init(nullptr);


        // -- Create Managers ------------------------------------------- //

        // Create Resource Manager
        m_RM = Scope<ResourceManager>(ResourceManager::Create());

        // Create Scene Manager
        m_SceneManager = Scope<SceneManager>(SceneManager::Create());

        {
            MF_PROFILE_SCOPE("Window Creation");
            m_Window = Scope<Window>(Window::Create());
            m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

        }
        m_Minimized = false;

        


        m_BufferResourceThread.join();

        // -- Global RenderPass -- //
        /*m_RenderPassGlobals = m_RM->CreateBindGroup({
            .DebugName  = "Render Pass Globals",
            .LayoutType = BindingLayoutType::LAYOUT_RENDERPASS,
            .Layout     = DEFAULT_RENDERPASS_BINDING_LAYOUT,
            .Textures   = {
                .LTC1 = {
                    "LTC1Texture",
                    0,
                    TextureType::LTC1,
                    TextureTiling::IMAGE_TILING_OPTIMAL,
                    TextureFormat::FORMAT_R32G32B32A32_SFLOAT,
                },
                .LTC2 = {
                    "LTC2Texture",
                    0,
                    TextureType::LTC2,
                    TextureTiling::IMAGE_TILING_OPTIMAL,
                    TextureFormat::FORMAT_R32G32B32A32_SFLOAT,
                }},
            .Buffers    = RenderPassUniformBufferDesc
        });*/

        // -- Scene Objects -- //

        

        // need to get info on whether the object is textured from
        // MODEL_PATHS



        

        // NON AREA LIGHTS
        /*for (auto& light : m_PointLights)
        {
            light.LightEnabled = 1;
            light.MaxLightDist = 200.f;
            light.RadiantFlux = 10.f;
            light.LightPos = { 235.f, 65.f, 20.f };
            light.LightColor = Maths::vec3(1.0f);
        }*/


        m_Window->GetGraphicsContext()->TempSecondaryInit();

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);

	}

	Application::~Application()
	{
        MemoryService::instance()->shutdown();
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
        m_TimeStep.Init();

       MF_CORE_INFO("STARTING FIRST LOOP");

        while (m_Running)
        {
            MF_PROFILE_SCOPE("Run Loop");

            m_TimeStep.CalculateDeltaTime();

            // -- Poll and Handle Events -- //
            m_Window->OnUpdate();


            // -- Start ImGui Frame -- //
            if (m_ImGuiLayer)
                m_ImGuiLayer->BeginFrame();


            // -- Game Logic Here -- //
            for (Layer* layer : m_LayerStack)
                layer->OnUpdate(m_TimeStep.GetDeltaTime());


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