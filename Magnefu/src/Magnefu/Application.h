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

		//inline std::array<Handle<Texture>, 3>& GetTextureHandles() { return m_Textures; }
		inline size_t GetTextureCount() { return sizeof(BindingTextureDescs) / sizeof(TextureDesc); }
		inline Handle<BindGroup>& GetMaterialBindGroup() { return m_Material; }
		inline Handle<BindGroup>& GetRenderPassBindGroup() { return m_RenderPassGlobals; }
		inline Handle<Shader>& GetGraphicsPipelineShaderHandle() { return m_GraphicsPipelineShader; }

		inline Handle<Buffer>& GetVertexBufferHandle() { return m_VertexBuffer; }
		inline Handle<Buffer>& GetIndexBufferHandle() { return m_IndexBuffer; }
		inline uint32_t GetIndexCount() { return static_cast<uint32_t>(m_Indices.data.size()) / sizeof(uint32_t); }
		inline Light& GetLightData() { return m_LightData; }
		inline Material& GetMaterialData() { return m_MaterialData; }

		inline static Application& Get() { return *s_Instance; }

		inline void SetVertices(DataBlock&& vertexBlock) { m_Vertices = std::move(vertexBlock); }
		inline void SetIndices(DataBlock&& indexBlock) { m_Indices = std::move(indexBlock); }

		inline void SetLightData(Light& lightData) { m_LightData = lightData; }
		inline void SetMaterialData(Material& materialData) { m_MaterialData = materialData; }


		//inline Span<const uint8_t> GetVertices() { return m_Vertices; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	public:

		Handle<BindGroup>     m_RenderPassGlobals;
		Handle<BindGroup>     m_Material;
		Handle<Buffer>        m_VertexBuffer;
		Handle<Buffer>        m_IndexBuffer;
		Handle<Shader>        m_GraphicsPipelineShader;
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

		Light m_LightData;
		Material m_MaterialData;
	};

	// to be defined in client
	Application* CreateApplication();
}