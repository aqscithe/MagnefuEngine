#include "mfpch.h"

#include "Application.h"
#include "Magnefu/Input.h"

#include "Globals.h"

#include "tests/Test3DRender.h"
#include "tests/Test2DTexture.h"
#include "tests/TestBatchRendering.h"
#include "tests/TestLighting.h"
#include "tests/TestModelLoading.h"

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
        Globals& global = Globals::Get();
        
        Magnefu::Test* activeTest = nullptr;
        Magnefu::TestMenu* testMenu = new Magnefu::TestMenu(activeTest);

        activeTest = testMenu;

        testMenu->RegisterTest<Magnefu::Test2DTexture>("2D Texture");
        testMenu->RegisterTest<Magnefu::Test3DRender>("Cube Render");
        testMenu->RegisterTest<Magnefu::TestBatchRendering>("Batching");
        testMenu->RegisterTest<Magnefu::TestLighting>("Lighting");
        testMenu->RegisterTest <Magnefu::TestModelLoading>("3D Models");


        auto lastTime = std::chrono::high_resolution_clock::now();
        while (m_Running)
        {
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto elapsedTime = currentTime - lastTime;
            lastTime = currentTime;

            m_Window->OnUpdate();

            if (m_ImGuiLayer)
                m_ImGuiLayer->BeginFrame();

            for (Layer* layer : m_LayerStack)
                layer->OnUpdate();



            ImGui::Begin("Application");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            //ImGui::Text("MousePos: %.1f %.1f", , mouseY);
            ImGui::Text("Yaw: %.2f, | Pitch: %.2f", global.yaw, global.pitch);
            ImGui::Separator();
            if (ImGui::TreeNode("CONTROLS"))
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

            if (activeTest)
            {
                activeTest->OnUpdate(std::chrono::duration<float>(elapsedTime).count());
                activeTest->OnRender();
                ImGui::Begin("Tests");
                if (activeTest != testMenu && ImGui::Button("<-"))
                {
                    delete activeTest;
                    activeTest = testMenu;
                }
                activeTest->OnImGUIRender();
                ImGui::End();
            }

            if (m_ImGuiLayer)
                m_ImGuiLayer->EndFrame();

        }

        if (activeTest != testMenu)
            delete testMenu;
        delete activeTest;
           
    }

      
}