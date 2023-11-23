#include "mfpch.h"

#include "Application.h"
#include "Renderer/GraphicsContext.h"
#include "Magnefu/Core/MemoryAllocation/OffsetAllocator.h"

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
        MF_CORE_ASSERT(!s_Instance, "Application instance already exists.");
        s_Instance = this;

        // Number to be determined by saved scene files
        // Default reservation of 1
        m_Scenes.reserve(SCENE_COUNT);

        {
            MF_PROFILE_SCOPE("Window Creation");
            m_Window = Scope<Window>(Window::Create());
            m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

        }
        m_Minimized = false;

        m_RM = Scope<ResourceManager>(ResourceManager::Create());


        m_BufferResourceThread.join();

        // -- Global RenderPass -- //
        m_RenderPassGlobals = m_RM->CreateBindGroup({
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
        });

        // -- Scene Objects -- //

        

        // need to get info on whether the object is textured from
        // MODEL_PATHS

        for (size_t i = 0; i < m_SceneObjects.size(); i++)
            m_SceneObjects[i].Init(i);


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
        
	}

    Scene* Application::CreateScene()
    {
        std::string name = "Scene" + std::to_string(m_Scenes.size());
        //TODO: ERROR HANDLING to ensure name is not in use
        
        return m_Scenes.emplace_back(Scene::Create(name)).get();
    }

    void Application::SetVertexBlock(DataBlock&& vertexBlock, size_t objPos, ModelType modelType)
    {
        switch (modelType)
        {
            case Magnefu::MODEL_DEFAULT:
            {
                m_SceneObjects[objPos].SetVertexBlock(std::move(vertexBlock));
                break;
            }
            default:
            {
                MF_CORE_ASSERT(false, "Invalid Model Type -- Application::SetVertexBlock");
                break;
            }
        }
        
    }

    void Application::SetIndexBlock(DataBlock&& indexBlock, size_t objPos, ModelType modelType)
    {
        switch (modelType)
        {
            case Magnefu::MODEL_DEFAULT:
            {
                m_SceneObjects[objPos].SetIndexBlock(std::move(indexBlock));
                break;
            }
            default:
            {
                MF_CORE_ASSERT(false, "Invalid Model Type -- Application::SetIndexBlock");
                break;
            }
        }
        
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
        //Scope<StackAllocator>& SingleFrameAllocator = StackAllocator::Get();
        //Scope<OffsetAllocator::Allocator>& OffsetAllocator = OffsetAllocator::Allocator::Get();

        // Maybe Do a unique pointer(Scope) as there should only be one
        GraphicsContext* GraphicsContext = m_Window->GetGraphicsContext();


        // TODO: Create setting to switch between locked and unlocked frame rate
        m_TimeStep.Init();

       MF_CORE_INFO("STARTING FIRST LOOP");

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


                for (Layer* layer : m_LayerStack)
                    layer->OnGUIRender();

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

        /*GraphicsContext* GraphicsContext = m_Window->GetGraphicsContext();
        ImGui::Begin("Renderer");
        ImGui::Text("Renderer: %s", GraphicsContext->GetRendererInfo().Renderer);
        ImGui::Text("Version: %s", GraphicsContext->GetRendererInfo().Version);
        ImGui::Text("Vendor: %s", GraphicsContext->GetRendererInfo().Vendor);
        ImGui::End();*/

        CameraData& data = GetWindow().GetSceneCamera()->GetData();
     
        ImGui::Begin("Camera");
        //ImGui::Text("Type: %s", data.Type);
        ImGui::Text("Aspect Ration: %f", data.AspectRatio);
        ImGui::SliderFloat("FOV", &data.FOV, 45.f, 100.f);
        ImGui::SliderFloat("Near", &data.Near, 0.01f, 10.f);
        ImGui::SliderFloat("Far", &data.Far, 700.f, 1000.f);
        ImGui::SliderFloat("Speed", &data.Speed, 15.f, 100.f);
        ImGui::SliderFloat3("Position", data.Position.e, -500.f, 500.f);
        ImGui::SliderFloat("Pitch", &data.Pitch, -360.f, 360.f);
        ImGui::SliderFloat("Yaw", &data.Yaw, -360.f, 360.f);
        ImGui::End();



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