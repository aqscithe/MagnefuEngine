#include "mfpch.h"

#include "Application.h"
#include "Renderer.h"

//#include "tests/TestClearColor.h"
//#include "tests/Test3DRender.h"
//#include "tests/Test2DTexture.h"
//#include "tests/TestBatchRendering.h"
//#include "tests/TestLighting.h"
//#include "tests/TestModelLoading.h"
//
//#include "imgui/imgui.h"
//#include "imgui/imgui_impl_glfw.h"
//#include "imgui/imgui_impl_opengl3.h"


namespace Magnefu
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application::Application()
	{
        m_Window = std::unique_ptr<Window>(Window::Create());
        m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
	}

	Application::~Application()
	{
	}

    void Application::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));

        MF_CORE_TRACE("{0}", event);

        for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
        {
            (*--it)->OnEvent(event);
            if (event.IsHandled())
                break;
        }
    }


	void Application::Run()
	{
        //Globals& global = Globals::Get();

        //// Setup Dear ImGui context
        //IMGUI_CHECKVERSION();
        //ImGui::CreateContext();
        //ImGuiIO& io = ImGui::GetIO(); (void)io;

        //// Setup Dear ImGui style
        //ImGui::StyleColorsDark();

        //// Setup Platform/Renderer backends
        //const char* glsl_version = "#version 460";
        //ImGui_ImplGlfw_InitForOpenGL(, true);
        //ImGui_ImplOpenGL3_Init(glsl_version);


        // Must be initialized after contex has been created

        


        
            Renderer renderer;

            Maths::vec4 clearColor = { 0.4f, 0.f, 0.8f, 1.f };

           /*Magnefu::Test* activeTest = nullptr;
           Magnefu::TestMenu* testMenu = new Magnefu::TestMenu(activeTest);

            activeTest = testMenu;

            testMenu->RegisterTest<Magnefu::TestClearColor>("Clear Color");
            testMenu->RegisterTest<Magnefu::Test2DTexture>("2D Texture");
            testMenu->RegisterTest<Magnefu::Test3DRender>("Cube Render");
            testMenu->RegisterTest<Magnefu::TestBatchRendering>("Batching");
            testMenu->RegisterTest<Magnefu::TestLighting>("Lighting");
            testMenu->RegisterTest <Magnefu::TestModelLoading>("3D Models");*/

            auto lastTime = std::chrono::high_resolution_clock::now();
            while (m_Running)
            {
                auto currentTime = std::chrono::high_resolution_clock::now();
                auto elapsedTime = currentTime - lastTime;
                lastTime = currentTime;

                renderer.ClearColor(clearColor);
                renderer.Clear();

                /*ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();*/

                for (Layer* layer : m_LayerStack)
                    layer->OnUpdate();

                m_Window->OnUpdate();

                //ImGui::Begin("Application");
                //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ////ImGui::Text("MousePos: %.1f %.1f", mouseX, mouseY);
                //ImGui::Text("Yaw: %.2f, | Pitch: %.2f", global.yaw, global.pitch);
                //ImGui::Separator();
                //if (ImGui::TreeNode("CONTROLS"))
                //{
                //    ImGui::Text("APP CONTROLS");
                //    ImGui::Bullet(); ImGui::Text("Close App:      ESC  |");
                //    ImGui::Text("CAMERA CONTROLS");
                //    ImGui::Bullet(); ImGui::Text("Camera Left:    A    | Left Arrow");
                //    ImGui::Bullet(); ImGui::Text("Camera Right:   D    | Right Arrow");
                //    ImGui::Bullet(); ImGui::Text("Camera Forward: W    | Up Arrow");
                //    ImGui::Bullet(); ImGui::Text("Camera Back:    S    | Down Arrow");
                //    ImGui::Bullet(); ImGui::Text("Camera Up:      E    |");
                //    ImGui::Bullet(); ImGui::Text("Camera Down:    Q    |");
                //    ImGui::Bullet(); ImGui::Text("Camera Rotate:  Right-Click + Move Mouse");
                //    ImGui::Bullet(); ImGui::Text("Adjust FOV:     Rotate Middle Mouse Button");
                //    ImGui::Separator();
                //    ImGui::TreePop();
                //}
                //ImGui::End();

                /*if (activeTest)
                {
                    activeTest->OnUpdate(m_Window->GetWindow(), std::chrono::duration<float>(elapsedTime).count());
                    activeTest->OnRender();
                    ImGui::Begin("Tests");
                    if (activeTest != testMenu && ImGui::Button("<-"))
                    {
                        delete activeTest;
                        activeTest = testMenu;
                    }
                    activeTest->OnImGUIRender();
                    ImGui::End();
                }*/


                

                /*ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());*/

            }

           /*if (activeTest != testMenu)
               delete testMenu;
           delete activeTest;*/
    }
        
        /*ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();*/

        
	

    bool  Application::OnWindowClose(WindowCloseEvent& event)
    {
        m_Running = false;
        return true;
    }

    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
    }

    void Application::PushOverlay(Layer* overlay)
    {
        m_LayerStack.PushOverlay(overlay);
    }
}