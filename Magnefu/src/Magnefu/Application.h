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

#include "Magnefu/Core/TimeStep.h"


namespace Magnefu
{
	class  Application
	{
	public:
		Application();
		Application(const Application&) = delete;
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

		inline std::array<Handle<Texture>, 3>& GetTextureHandles() { return m_Textures; }
		inline size_t GetTextureCount() { return m_Textures.size(); }

		inline Handle<Buffer>& GetUniformBufferHandle() { return m_Uniforms; }
		inline Handle<Buffer>& GetVertexBufferHandle() { return m_VertexBuffer; }
		inline Handle<Buffer>& GetIndexBufferHandle() { return m_IndexBuffer; }
		inline uint32_t GetIndexCount() { return static_cast<uint32_t>(m_Indices.data.size()) / sizeof(uint32_t); }

		inline static Application& Get() { return *s_Instance; }

		inline void SetVertices(DataBlock&& vertexBlock) { m_Vertices = std::move(vertexBlock); }
		inline void SetIndices(DataBlock&& indexBlock) { m_Indices = std::move(indexBlock); }


		//inline Span<const uint8_t> GetVertices() { return m_Vertices; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	public:

		std::array<Handle<Texture>, 3> m_Textures;

		Handle<Buffer>        m_Uniforms;
		Handle<Buffer>        m_VertexBuffer;
		Handle<Buffer>        m_IndexBuffer;
		DataBlock   m_Vertices;
		DataBlock   m_Indices;

	private:
		static Application* s_Instance;

		TimeStep m_TimeStep;
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;
		Scope<Window> m_Window;
		Scope<ResourceManager> m_RM;

		bool m_Running;
		bool m_Minimized;
	};

	// to be defined in client
	Application* CreateApplication();
}