#pragma once

#include "Magnefu/Renderer/GraphicsContext.h"
#include "vulkan/vulkan.h"

struct GLFWwindow;

namespace Magnefu
{
	using String = std::string;

	enum class ShaderType
	{
		None = -1,
		Vertex,
		Fragment
	};

	struct ShaderSource
	{
		String Text;
		ShaderType Type;
	};

	struct ShaderList
	{
		ShaderSource Vertex;
		ShaderSource Fragment;
	};

	struct QueueFamilyIndices 
	{
		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> PresentFamily;

		bool IsComplete() {
			return (GraphicsFamily.has_value() && PresentFamily.has_value());
		}
	};

	struct SwapChainSupportDetails 
	{
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR>   PresentModes;
		VkSurfaceCapabilitiesKHR        Capabilities;
	};

	struct GraphicsPipelines 
	{
		VkPipeline Primitive3D;
		VkPipeline Skybox;
	};

	struct Vertex 
	{
		Maths::vec2 pos;
		Maths::vec3 color;

		static VkVertexInputBindingDescription GetBindingDescription() 
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);


			return attributeDescriptions;
		}
	};

	class VKContext : public GraphicsContext
	{
	public:
		VKContext(GLFWwindow* windowHandle);
		~VKContext();

		void Init() override;
		void SwapBuffers() override;
		void DrawFrame() override;
		void OnImGuiRender() override;
		void OnFinish() override; // main loop completed
		void GetImGuiInitData() override { int x = 1; }
		void SetFramebufferResized(bool framebufferResized) override { m_FramebufferResized = framebufferResized; }

	private:
		void CreateVkInstance();
		void SetupValidationLayers(bool& allLayersAvailable);
		void SetupDebugMessenger();
		void CreateWindowSurface();
		void SelectPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateGraphicsPipeline();
		void CreateFrameBuffers();
		void CreateVertexBuffer();
		void CreateCommandBuffers();
		void CreateSyncObjects();

		std::vector<const char*> GetRequiredExtensions();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		bool IsDeviceSuitable(VkPhysicalDevice);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void RecreateSwapChain();
		void CleanupSwapChain();
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		// Place in VKShader
		ShaderList ParseShader(const String& filepath);
		VkShaderModule CreateShaderModule(const ShaderSource& source);

	private:
		GLFWwindow* m_WindowHandle;
		RendererInfo                 m_RendererInfo;
		VkInstance                   m_VkInstance;
		VkDebugUtilsMessengerEXT     m_DebugMessenger;
		VkPhysicalDevice             m_VkPhysicalDevice;
		VkDevice                     m_VkDevice;
		VkQueue                      m_GraphicsQueue;
		VkSurfaceKHR                 m_WindowSurface;
		VkQueue			             m_PresentQueue;
		QueueFamilyIndices           m_QueueFamilyIndices;
		VkSwapchainKHR               m_SwapChain;
		std::vector<VkImage>         m_SwapChainImages;
		VkFormat                     m_SwapChainImageFormat;
		VkExtent2D                   m_SwapChainExtent;
		std::vector<VkImageView>     m_SwapChainImageViews;
		VkRenderPass                 m_RenderPass;
		VkPipelineLayout             m_PipelineLayout;
		//GraphicsPipelines            m_GraphicsPipelines;
		VkPipeline                   m_GraphicsPipeline;
		std::vector<VkFramebuffer>   m_SwapChainFramebuffers;
		VkCommandPool                m_CommandPool;
		std::vector<VkCommandBuffer> m_CommandBuffers;
		std::vector<VkSemaphore>     m_ImageAvailableSemaphores;
		std::vector<VkSemaphore>     m_RenderFinishedSemaphores;
		std::vector<VkFence>         m_InFlightFences;
		uint32_t                     m_CurrentFrame;
		bool						 m_FramebufferResized;
		VkBuffer                     m_VertexBuffer;
		VkDeviceMemory               m_VertexBufferMemory;

#ifdef MF_DEBUG
		const bool                   m_EnableValidationLayers = true;
#else			                   
		const bool                   m_EnableValidationLayers = false;
#endif
	};
}