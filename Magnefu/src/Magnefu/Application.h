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

		inline size_t GetTextureCount() { return sizeof(BindingTextureDescs) / sizeof(TextureDesc); }
		//inline Handle<BindGroup>& GetMaterialBindGroup() { return m_Material; }
		inline Handle<BindGroup>& GetRenderPassBindGroup() { return m_RenderPassGlobals; }
		//inline Handle<Shader>& GetGraphicsPipelineShaderHandle() { return m_GraphicsPipelineShader; }
		//inline Handle<Buffer>& GetVertexBufferHandle() { return m_VertexBuffer; }
		//inline Handle<Buffer>& GetIndexBufferHandle() { return m_IndexBuffer; }
		//inline uint32_t GetIndexCount() { return static_cast<uint32_t>(m_Indices.data.size()) / sizeof(uint32_t); }
		inline Light& GetLightData() { return m_LightData; }
		inline Material& GetMaterialData() { return m_MaterialData; }

		inline static Application& Get() { return *s_Instance; }

		inline void SetVertexBlock(DataBlock&& vertexBlock, size_t objPos) { m_SceneObjects[objPos].SetVertexBlock(std::move(vertexBlock)); }
		inline void SetIndexBlock(DataBlock&& indexBlock, size_t objPos) { m_SceneObjects[objPos].SetIndexBlock(std::move(indexBlock)); }

		inline void SetLightData(Light& lightData) { m_LightData = lightData; }
		inline void SetMaterialData(Material& materialData) { m_MaterialData = materialData; }

		inline void ResizeSceneObjects(const size_t size) { m_SceneObjects.resize(size); }
		inline std::vector<SceneObject>& GetSceneObjects() { return m_SceneObjects; }


		//inline Span<const uint8_t> GetVertices() { return m_Vertices; }

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

		bool m_Running;
		bool m_Minimized;

		Handle<BindGroup>        m_RenderPassGlobals;
		std::vector<SceneObject> m_SceneObjects;

		Light m_LightData;
		Material m_MaterialData;
	};

	// to be defined in client
	Application* CreateApplication();
}