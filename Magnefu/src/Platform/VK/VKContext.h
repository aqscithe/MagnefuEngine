#pragma once

#include "Magnefu/Renderer/GraphicsContext.h"
#include "vulkan/vulkan.h"

struct GLFWwindow;

namespace Magnefu
{
	using String = std::string;

	struct ShaderProgramSource
	{
		String VertexSource;
		String FragmentSource;
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
		void OnImGuiRender() override;
		void GetImGuiInitData() override { int x = 1; }

	private:
		bool IsDeviceSuitable(VkPhysicalDevice);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		// Place in VKShader
		ShaderProgramSource ParseShader(const String& filepath);
		//VkShaderModule CreateShaderModule(const std::vector<char>& code);
		VkShaderModule CreateShaderModule(const String& code);

	private:
		GLFWwindow*              m_WindowHandle;
		RendererInfo             m_RendererInfo;
		VkInstance               m_VkInstance;
		VkPhysicalDevice         m_VkPhysicalDevice;
		VkDevice                 m_VkDevice;
		VkQueue                  m_GraphicsQueue;
		VkSurfaceKHR             m_WindowSurface;
		VkQueue			         m_PresentQueue;
		QueueFamilyIndices       m_QueueFamilyIndices;
		VkSwapchainKHR           m_SwapChain;
		std::vector<VkImage>     m_SwapChainImages;
		VkFormat                 m_SwapChainImageFormat;
		VkExtent2D               m_SwapChainExtent;
		std::vector<VkImageView> m_SwapChainImageViews;
		GraphicsPipelines        m_GraphicsPipelines;
	};
}