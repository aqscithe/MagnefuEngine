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

	static const std::vector<const char*> deviceExtensions = 
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
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

	class VKContext : public GraphicsContext
	{
	public:
		VKContext(GLFWwindow* windowHandle);
		~VKContext();

		void Init() override;
		void SwapBuffers() override;
		void DrawFrame() override;
		void OnImGuiRender() override;
		void GetImGuiInitData() override { int x = 1; }

	private:
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

		// Place in VKShader
		ShaderList ParseShader(const String& filepath);
		VkShaderModule CreateShaderModule(const ShaderSource& source);

	private:
		GLFWwindow*                m_WindowHandle;
		RendererInfo               m_RendererInfo;
		VkInstance                 m_VkInstance;
		VkDebugUtilsMessengerEXT   m_DebugMessenger;
		VkPhysicalDevice           m_VkPhysicalDevice;
		VkDevice                   m_VkDevice;
		VkQueue                    m_GraphicsQueue;
		VkSurfaceKHR               m_WindowSurface;
		VkQueue			           m_PresentQueue;
		QueueFamilyIndices         m_QueueFamilyIndices;
		VkSwapchainKHR             m_SwapChain;
		std::vector<VkImage>       m_SwapChainImages;
		VkFormat                   m_SwapChainImageFormat;
		VkExtent2D                 m_SwapChainExtent;
		std::vector<VkImageView>   m_SwapChainImageViews;
		VkRenderPass               m_RenderPass;
		VkPipelineLayout           m_PipelineLayout;
		//GraphicsPipelines          m_GraphicsPipelines;
		VkPipeline                 m_GraphicsPipeline;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;
		VkCommandPool              m_CommandPool;
		VkCommandBuffer            m_CommandBuffer;
		VkSemaphore                m_ImageAvailableSemaphore;
		VkSemaphore                m_RenderFinishedSemaphore;
		VkFence                    m_InFlightFence;
#ifdef MF_DEBUG
		const bool                 m_EnableValidationLayers = true;
#else			                   
		const bool                 m_EnableValidationLayers = false;
#endif
	};
}