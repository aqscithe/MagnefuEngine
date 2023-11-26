#pragma once

#include "Magnefu/Core/Assertions.h"
#include "Window.h"
#include "Magnefu/LayerStack.h"
#include "Magnefu/ImGui/ImGuiLayer.h"
#include "Magnefu/Core/Events/ApplicationEvent.h"
#include "Magnefu/Core/Events/MouseEvent.h"
#include "Magnefu/Core/Events/KeyEvent.h"
#include "Magnefu/ResourceManagement/ResourceManager.h"
#include "Magnefu/Core/MemoryAllocation/StackAllocator.h"
#include "Magnefu/Core/MemoryAllocation/LinkedListAlloc.h"
#include "Magnefu/Renderer/Light.h"
#include "Magnefu/Renderer/Material.h"
#include "Magnefu/Renderer/SceneObject.h"
#include "Magnefu/Core/TimeStep.h"
#include "Magnefu/ResourceManagement/ResourcePaths.h"
#include "Magnefu/Scene/SceneManager.h"



namespace Magnefu
{
	struct Draw
	{
		Handle<Shader> Shader;
		Handle<BindGroup> BindGroups[3];
		Handle<Buffer> IndexBuffer;
		Handle<Buffer> VertexBuffer;

		// For when I have 1 large buffer for each buffer type:
		//uint32_t IndexOffset;
		//uint32_t VertexOffset;
		
		
		// For handling multiple instances of the same object(i think)
		//uint32_t InstanceOffset = 0;
		//uint32_t InstanceCount = 1;
	};

	class  Application
	{
	public:
		Application();
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		virtual ~Application();

		virtual void Run();

		void OnEvent(Event& event);
		void OnGUIRender();
		void OnUpdate(float deltaTime);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline Window& GetWindow() { return *m_Window; }
		inline TimeStep& GetTimeStep() { return m_TimeStep; }
		inline ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		inline ResourceManager& GetResourceManager() { return *m_RM; }

		inline SceneManager& GetSceneManager() { return *m_SceneManager; }

		inline Handle<BindGroup>& GetRenderPassBindGroup() { return m_RenderPassGlobals; }

		// -- AREA LIGHT INFO -- //
		inline const std::array<AreaLight, MAX_AREA_LIGHTS>& GetAreaLightData() const { return m_AreaLights; }
		inline const Maths::mat4& GetAreaLightVertices() const { return m_AreaLightVertices; }
		inline int GetAreaLightCount() const { return m_AreaLightCount; }

		inline void SetAreaLightData(const std::array<AreaLight, MAX_AREA_LIGHTS>& areaLightData) { m_AreaLights = areaLightData; }
		inline void SetAreaLightVertices(const Maths::mat4& vertices) { m_AreaLightVertices = vertices; }
		inline void SetAreaLightCount(int count) { m_AreaLightCount = count; }
		
		inline std::thread& GetBufferThread() { return m_BufferResourceThread; }
		inline std::thread& GetImageThread() { return m_ImageResourceThread; }

		// -- SCENES -- //
		

		inline void ResizeSceneObjects(const size_t size) { m_SceneObjects.resize(size); }

		inline std::vector<SceneObject>& GetSceneObjects() { return m_SceneObjects; }


		inline static Application& Get() { return *s_Instance; }

		void SetVertexBlock(DataBlock&& vertexBlock, size_t objPos, ModelType modelType);
		void SetIndexBlock(DataBlock&& indexBlock, size_t objPos, ModelType modelType);
		

		


	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	

	private:
		static Application* s_Instance;

		TimeStep m_TimeStep;
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;
		Scope<Window> m_Window;
		Scope<ResourceManager> m_RM;

		Scope<SceneManager> m_SceneManager;

		bool m_Running;
		bool m_Minimized;

		Handle<BindGroup>        m_RenderPassGlobals;

		// Phasing out SceneObject in favor or scene
		// objects will be represented as entities using
		// ECS architecture
		std::vector<SceneObject> m_SceneObjects;

		


		// -- Area Lights -- //
		std::array<AreaLight, MAX_AREA_LIGHTS> m_AreaLights;
		Maths::mat4 m_AreaLightVertices;
		int m_AreaLightCount;

		//std::vector<std::thread> m_ThreadPool;
		std::thread m_BufferResourceThread;
		std::thread m_ImageResourceThread;


		
	};

	// to be defined in client
	Application* CreateApplication();
}