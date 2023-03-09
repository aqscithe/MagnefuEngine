#include "Application.h"
#include "Log.h"
#include "Globals.h"
#include "Renderer.h"
#include "Vectors.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "tests/TestClearColor.h"
#include "tests/Test3DRender.h"
#include "tests/Test2DTexture.h"
#include "tests/TestBatchRendering.h"
#include "tests/TestLighting.h"
#include "tests/TestModelLoading.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <chrono>
#include <iostream>


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Globals& global = Globals::Get();

    global.fovY -= (float)yoffset;
    if (global.fovY < 1.0f)
        global.fovY = 1.0f;
    if (global.fovY > 100.f)
        global.fovY = 100.f;
}

void processInput(GLFWwindow* window, bool& flightMode)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    flightMode = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_REPEAT);
    if (flightMode)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void MouseUpdates(GLFWwindow* window, float& mouseDeltaX, double& mouseX, float& mouseDeltaY, double& mouseY, float sensitivity, const bool& flightMode)
{
    Globals& global = Globals::Get();

    double newMouseX, newMouseY;
    glfwGetCursorPos(window, &newMouseX, &newMouseY);
    mouseDeltaX = (float)(newMouseX - mouseX);
    mouseDeltaY = (float)(newMouseY - mouseY);
    mouseX = newMouseX;
    mouseY = newMouseY;

    mouseDeltaX *= sensitivity;
    mouseDeltaY *= sensitivity;


    if (!flightMode) return;

    global.yaw += mouseDeltaX;
    global.pitch += mouseDeltaY;

    if (global.pitch > Maths::toRadians(89.0f))
        global.pitch = Maths::toRadians(89.0f);
    if (global.pitch < Maths::toRadians(-89.0f))
        global.pitch = Maths::toRadians(-89.0f);
}

namespace Magnefu
{
	Application::Application()
	{
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
        Globals& global = Globals::Get();

		GLFWwindow* window;

		/* Initialize the library */
		if (!glfwInit())
		{
			MF_ERROR("GLFW Failed to Initialize. Closing...");
			exit(-1);
		}

        const char* glsl_version = "#version 460";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif


        /* Create a windowed mode window and its OpenGL context */
        window = glfwCreateWindow(global.WIDTH, global.HEIGHT, "Rasterizer", NULL, NULL);
        if (!window)
        {
            MF_ERROR("GLFW window failed to create. Terminating GLFW..");
            glfwTerminate();
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        // Must be initialized after contex has been created
        if (glewInit() != GLEW_OK)
            MF_ERROR("GLEW initialization failed");
        else
            MF_INFO("GLEW initialization succeeded");

        std::cout << glGetString(GL_VERSION) << std::endl;
        std::cout << glGetString(GL_VENDOR) << std::endl;
        std::cout << glGetString(GL_RENDERER) << std::endl;

        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetScrollCallback(window, scroll_callback);

        {
            Renderer renderer;

            Maths::vec4 clearColor = { 0.f, 0.f, 0.f, 1.f };

            int displayWidth, displayHeight;
            float lastX = global.WIDTH / 2.f;
            float lastY = global.HEIGHT / 2.f;
            float sensitivity = 0.001f;
            float mouseDeltaX;
            float mouseDeltaY;
            double	mouseX = 0.0;
            double	mouseY = 0.0;
            bool flightMode = true;

           Magnefu::Test* activeTest = nullptr;
           Magnefu::TestMenu* testMenu = new Magnefu::TestMenu(activeTest);

            activeTest = testMenu;

            testMenu->RegisterTest<Magnefu::TestClearColor>("Clear Color");
            testMenu->RegisterTest<Magnefu::Test2DTexture>("2D Texture");
            testMenu->RegisterTest<Magnefu::Test3DRender>("Cube Render");
            testMenu->RegisterTest<Magnefu::TestBatchRendering>("Batching");
            testMenu->RegisterTest<Magnefu::TestLighting>("Lighting");
            testMenu->RegisterTest <Magnefu::TestModelLoading>("3D Models");

            auto lastTime = std::chrono::high_resolution_clock::now();
            while (!glfwWindowShouldClose(window))
            {
                auto currentTime = std::chrono::high_resolution_clock::now();
                auto elapsedTime = currentTime - lastTime;
                lastTime = currentTime;

                renderer.Clear();
                renderer.ClearColor(clearColor);

                glfwPollEvents();
                MouseUpdates(window, mouseDeltaX, mouseX, mouseDeltaY, mouseY, sensitivity, flightMode);
                processInput(window, flightMode);

                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                ImGui::Begin("Application");
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::Text("MousePos: %.1f %.1f", mouseX, mouseY);
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
                    activeTest->OnUpdate(window, std::chrono::duration<float>(elapsedTime).count());
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


                glfwGetFramebufferSize(window, &displayWidth, &displayHeight);
                glViewport(0, 0, displayWidth, displayHeight);

                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                glfwSwapBuffers(window);
            }

           if (activeTest != testMenu)
               delete testMenu;
           delete activeTest;
        }
        
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
	}
}