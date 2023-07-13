#include "mfpch.h"
#include <iterator>

#include "VulkanContext.h"
#include "Magnefu/Application.h"
#include "Magnefu/Core/Maths/Quaternion.h"
#include "Magnefu/Renderer/RenderConstants.h"
#include "Magnefu/ResourceManagement/ResourceManager.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#include "shaderc/shaderc.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader/tiny_obj_loader.h"

#include <set>
#include <unordered_map>

namespace Magnefu
{

	static const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
	};

	static const std::vector<const char*> deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	bool operator==(const Vertex& a, const Vertex& b)
	{
		return a.pos == b.pos && a.color == b.color && a.texCoord == b.texCoord;
	}

	static const std::string MODEL_PATH = "res/meshes/corridor.obj";
	static const std::string BASE_TEXTURE_PATH = "res/textures/scificorridor/scene_1001_BaseColor.png";
	static const std::string METAL_TEXTURE_PATH = "res/textures/scificorridor/scene_1001_Metalness.png";
	static const std::string ROUGHNESS_TEXTURE_PATH = "res/textures/scificorridor/scene_1001_Roughness.png";
	static const std::string NORMAL_TEXTURE_PATH = "res/textures/scificorridor/scene_1001_Normal.png";
	static const std::string AO_TEXTURE_PATH = "res/textures/scificorridor/scene_1001_AO.png";
	static const std::string ARM_TEXTURE_PATH = "res/textures/scificorridor/scene_1001_ARM.png";


	/*static const std::string MODEL_PATH = "res/meshes/Bronze_shield.obj";
	static const std::string BASE_TEXTURE_PATH = "res/textures/Bronze_shield/Bronze_shield_BaseColor.png";
	static const std::string METAL_TEXTURE_PATH = "res/textures/Bronze_shield/Bronze_shield_Metallic.png";
	static const std::string ROUGHNESS_TEXTURE_PATH = "res/textures/Bronze_shield/Bronze_shield_Roughness.png";
	static const std::string NORMAL_TEXTURE_PATH = "res/textures/Bronze_shield/Bronze_shield_Normal.png";
	static const std::string AO_TEXTURE_PATH = "res/textures/Bronze_shield/Bronze_shield_AO.png";
	static const std::string ARM_TEXTURE_PATH = "res/textures/Bronze_shield/Bronze_shield_ARM.png";*/


	/*static const std::string MODEL_PATH = "res/meshes/Victorian_Painting.obj";
	static const std::string BASE_TEXTURE_PATH = "res/textures/Victorian_Painting/VictorianPaintings_BaseColor_Utility-sRGB-Texture.png";
	static const std::string METAL_TEXTURE_PATH = "res/textures/Victorian_Painting/VictorianPaintings_Metallic_Utility-Raw.png";
	static const std::string ROUGHNESS_TEXTURE_PATH = "res/textures/Victorian_Painting/VictorianPaintings_Roughness_Utility-Raw.png";
	static const std::string NORMAL_TEXTURE_PATH = "res/textures/Victorian_Painting/VictorianPaintings_Normal_Utility-Raw.png";
	static const std::string AO_TEXTURE_PATH = "res/textures/Victorian_Painting/VictorianPaintings_AO_1_Utility-Raw.png";
	static const std::string ARM_TEXTURE_PATH = "res/textures/Victorian_Painting/VictorianPaintings_ARM_Utility-sRGB-Texture.png";*/


	static const std::string SHADER_PATH = "res/shaders/Basic.shader";
	static const std::string PARTICLE_SHADER_PATH = "res/shaders/Particles.shader";

	static const std::array<std::string, 3> TEXTURE_PATHS { 
		BASE_TEXTURE_PATH, 
		ARM_TEXTURE_PATH,
		//METAL_TEXTURE_PATH, 
		//ROUGHNESS_TEXTURE_PATH, 
		NORMAL_TEXTURE_PATH,
		//AO_TEXTURE_PATH
	};


	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		MF_CORE_WARN("validation layer: {}",  pCallbackData->pMessage);

		return VK_FALSE;
	}

	VulkanContext* VulkanContext::s_Instance = nullptr;

	VulkanContext::VulkanContext(GLFWwindow* windowHandle) :
		m_WindowHandle(windowHandle), m_VkInstance(VkInstance()), m_VkPhysicalDevice(VK_NULL_HANDLE)
	{
		MF_CORE_ASSERT(m_WindowHandle, "Window Handle is null!");

		MF_CORE_ASSERT(!s_Instance, "VulkanContext instance already exists.");

		s_Instance = this;
	}

	VulkanContext::~VulkanContext()
	{
		CleanupSwapChain();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(m_VkDevice, m_ComputeUniformBuffers[i], nullptr);
			vkFreeMemory(m_VkDevice, m_ComputeUniformBuffersMemory[i], nullptr);
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(m_VkDevice, m_ShaderStorageBuffers[i], nullptr);
			vkFreeMemory(m_VkDevice, m_ShaderStorageBuffersMemory[i], nullptr);
		}

		vkDestroyDescriptorPool(m_VkDevice, m_ComputeDescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(m_VkDevice, m_ComputeDescriptorSetLayout, nullptr);


		vkDestroySampler(m_VkDevice, m_TextureSampler, nullptr);

		for (size_t i = 0; i < m_Textures.size(); i++)
		{
			vkDestroyImageView(m_VkDevice, m_Textures[i].ImageView, nullptr);
			vkDestroyImage(m_VkDevice, m_Textures[i].Image, nullptr);
			vkFreeMemory(m_VkDevice, m_Textures[i].Buffer, nullptr);
		}

		/*for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
		{
			vkDestroyBuffer(m_VkDevice, m_UniformBuffers[i], nullptr);
			vkFreeMemory(   m_VkDevice, m_UniformBuffersMemory[i], nullptr);
		}*/

		vkDestroyDescriptorPool(m_VkDevice, m_DescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(m_VkDevice, m_DescriptorSetLayout, nullptr);

		/*vkDestroyBuffer(m_VkDevice, m_IndexBuffer, nullptr);
		vkFreeMemory(m_VkDevice,    m_IndexBufferMemory, nullptr);

		vkDestroyBuffer(m_VkDevice, m_VertexBuffer, nullptr);
		vkFreeMemory(m_VkDevice, m_VertexBufferMemory, nullptr);*/

		vkDestroyPipeline(m_VkDevice, m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_VkDevice, m_PipelineLayout, nullptr);

		vkDestroyRenderPass(m_VkDevice, m_RenderPass, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_VkDevice, m_ImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(m_VkDevice, m_ImGuiRenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_VkDevice, m_RenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(m_VkDevice, m_InFlightFences[i], nullptr);
			vkDestroyFence(m_VkDevice, m_ImGuiInFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(m_VkDevice, m_CommandPool, nullptr);
		
		vkDestroyDevice(m_VkDevice, nullptr);

		if (m_EnableValidationLayers)
			DestroyDebugUtilsMessengerEXT(m_VkInstance, m_DebugMessenger, nullptr);

		vkDestroySurfaceKHR(m_VkInstance, m_WindowSurface, nullptr);
		
		vkDestroyInstance(m_VkInstance, nullptr);
		
	}

	void VulkanContext::Init()
	{
		MF_PROFILE_FUNCTION();

		m_FramebufferResized = false;
		m_CurrentFrame = 0;

		CreateVkInstance();
		SetupDebugMessenger();
		CreateWindowSurface();
		SelectPhysicalDevice();
		CreateLogicalDevice();
		

		// -- Everything above will remain in Init() -- //

		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateDescriptorSetLayout();
		CreateComputeDescriptorSetLayout();
		CreateGraphicsPipeline();
		CreateParticleGraphicsPipeline();
		CreateCommandPool();
		CreateShaderStorageBuffers(); // For Compute pipeline
		CreateComputePipeline();
		CreateColorResources();
		CreateDepthResources();
		CreateFrameBuffers();
		CreateTextures();
		CreateTextureSampler();
		LoadModel();
		

	}

	void VulkanContext::TempSecondaryInit()
	{
		//CreateVertexBuffer();
		//CreateIndexBuffer();
		//CreateUniformBuffers();
		CreateComputeUniformBuffers();
		CreateDescriptorPool();
		CreateComputeDescriptorPool();
		CreateDescriptorSets();
		CreateComputeDescriptorSets();
		CreateCommandBuffers();
		CreateComputeCommandBuffers();
		CreateSyncObjects();

	}

	void VulkanContext::DrawFrame()
	{
		//PerformComputeOps();
		PerformGraphicsOps();
		PresentImage();
	}

	void VulkanContext::OnImGuiRender()
	{
		
	}

	void VulkanContext::OnFinish()
	{
		vkDeviceWaitIdle(m_VkDevice);

	}

	std::any VulkanContext::GetContextInfo(const std::string& name)
	{
		if (name == "Instance") return m_VkInstance;
		else if (name == "PhysicalDevice") return m_VkPhysicalDevice;
		else if (name == "Device") return m_VkDevice;
		else if (name == "Surface") return m_WindowSurface;
		else if (name == "SurfaceFormat") return m_SurfaceFormat;
		else if (name == "ImageCount") return m_ImageCount;
		else if (name == "QueueFamily") return m_QueueFamilyIndices.GraphicsFamily.value();
		else if (name == "Queue") return m_GraphicsQueue;
		else if (name == "PresentMode") return m_PresentMode;
		else if (name == "CurrentFrame") return m_CurrentFrame;
		else if (name == "SwapChain") return m_SwapChain;
		else if (name == "CommandPool") return m_CommandPool;
		else if (name == "CommandBuffer") return m_CommandBuffers[m_CurrentFrame];
		else if (name == "ImGuiCommandBuffer") return m_ImGuiCommandBuffers[m_CurrentFrame];
		else if (name == "SwapChainImages") return m_SwapChainImages.data();
		else if (name == "SwapChainExtent") return m_SwapChainExtent;
		else if (name == "SwapChainRebuild") return m_SwapChainRebuild;
		else if (name == "InFlightFence") return m_InFlightFences[m_CurrentFrame];
		else if (name == "ImGuiInFlightFence") return m_ImGuiInFlightFences[m_CurrentFrame];
		else if (name == "ImGuiWaitSemaphores") return &m_RenderFinishedSemaphores[m_CurrentFrame];
		else if (name == "ImGuiSignalSemaphores") return &m_ImGuiRenderFinishedSemaphores[m_CurrentFrame];
	}

	void VulkanContext::CreateVkInstance()
	{
		bool allLayersAvailable = true;
		SetupValidationLayers(allLayersAvailable);

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Sandbox - Vulkan";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Magnefu Engine - Vulkan";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pApplicationInfo = &appInfo;


		auto extensions = GetRequiredExtensions();
		instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (m_EnableValidationLayers && allLayersAvailable)
		{
			instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();

			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			instanceCreateInfo.enabledLayerCount = 0;
			instanceCreateInfo.pNext = nullptr;
		}


		if (vkCreateInstance(&instanceCreateInfo, nullptr, &m_VkInstance) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to create Vulkan VkInstance");
	}

	void VulkanContext::SetupValidationLayers(bool& allLayersAvailable)
	{
#ifdef MF_DEBUG
		// Print extensions
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> vkextensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vkextensions.data());
		MF_CORE_DEBUG("Available Vulkan Extensions: ");
		for (const auto& extension : vkextensions)
		{
			MF_CORE_DEBUG("\t{}", extension.extensionName);
		}
#endif
		// Check validation layer support

		if (m_EnableValidationLayers)
		{
			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

			for (const char* layerName : validationLayers)
			{
				bool layerFound = false;

				for (const auto& layerProperties : availableLayers)
				{
					if (strcmp(layerName, layerProperties.layerName) == 0)
					{
						layerFound = true;
						break;
					}
				}

				if (!layerFound)
				{
					MF_CORE_ASSERT(layerFound, "Requested validation layer unavailable: {}", layerName);
					allLayersAvailable = false;
					break;
				}
			}
		}
	}

	void VulkanContext::SetupDebugMessenger()
	{
		if (m_EnableValidationLayers)
		{
			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
			PopulateDebugMessengerCreateInfo(debugCreateInfo);

			if (CreateDebugUtilsMessengerEXT(m_VkInstance, &debugCreateInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
				MF_CORE_ERROR("failed to set up debug messenger!");

		}
	}

	void VulkanContext::CreateWindowSurface()
	{
		if (glfwVulkanSupported() == GLFW_FALSE)
			MF_CORE_DEBUG("GLFW - Vulkan not Supported!!");

		if (glfwCreateWindowSurface(m_VkInstance, m_WindowHandle, nullptr, &m_WindowSurface) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to create a window surface!");
	}

	void VulkanContext::SelectPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, nullptr);
		MF_CORE_ASSERT(deviceCount, "Failed to find a GPU with Vulkan support");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device))
			{
				m_VkPhysicalDevice = device;
				m_MSAASamples = GetMaxUsableSampleCount();
				MF_CORE_DEBUG("MSAA x{}", m_MSAASamples);
				break;
			}
		}

		MF_CORE_ASSERT(m_VkPhysicalDevice, "Failed to find a suitable GPU!");
	}

	void VulkanContext::CreateLogicalDevice()
	{
		// Specifying queues to be created

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		queueCreateInfos.reserve(2);
		std::set<uint32_t> uniqueQueueFamilies = { m_QueueFamilyIndices.GraphicsFamily.value(), m_QueueFamilyIndices.PresentFamily.value() };

		float queuePriority = 1.f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// Specifying device features to be used
		VkPhysicalDeviceFeatures deviceFeatures{}; //empty for now
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.sampleRateShading = VK_FALSE;

		// Create the logical device

#ifdef MF_DEBUG
		// Print device extensions
		uint32_t deviceExtensionCount = 0;
		vkEnumerateDeviceExtensionProperties(m_VkPhysicalDevice, nullptr, &deviceExtensionCount, nullptr);
		std::vector<VkExtensionProperties> vkDeviceExtensions(deviceExtensionCount);
		vkEnumerateDeviceExtensionProperties(m_VkPhysicalDevice, nullptr, &deviceExtensionCount, vkDeviceExtensions.data());
		MF_CORE_DEBUG("Available Device Extensions: ");
		for (const auto& extension : vkDeviceExtensions)
		{
			MF_CORE_DEBUG("\t{}", extension.extensionName);
		}
#endif
		VkDeviceCreateInfo logicalDevCreateInfo{};
		logicalDevCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		logicalDevCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		logicalDevCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		logicalDevCreateInfo.pEnabledFeatures = &deviceFeatures;
		logicalDevCreateInfo.enabledExtensionCount = deviceExtensions.size();
		logicalDevCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (m_EnableValidationLayers)
		{
			logicalDevCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			logicalDevCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			logicalDevCreateInfo.enabledLayerCount = 0;
		}

		// Instantiate the logical device
		if (vkCreateDevice(m_VkPhysicalDevice, &logicalDevCreateInfo, nullptr, &m_VkDevice) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create logical device!");

		// Retrieving queue handles
		vkGetDeviceQueue(m_VkDevice, m_QueueFamilyIndices.GraphicsFamily.value(), 0, &m_GraphicsQueue); // that 0 is the queue family index
		vkGetDeviceQueue(m_VkDevice, m_QueueFamilyIndices.PresentFamily.value(), 0, &m_PresentQueue);
		vkGetDeviceQueue(m_VkDevice, m_QueueFamilyIndices.ComputeFamily.value(), 0, &m_ComputeQueue);

	}

	void VulkanContext::CreateSwapChain()
	{
		// -- Creating the swap chain (infrastructure for the frame buffer) -- // TODO: Move to VKFrameBuffer class

		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_VkPhysicalDevice);

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
		m_PresentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.Capabilities);

		uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
		if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.Capabilities.maxImageCount;
		}

		//--For ImGui--//
		m_SurfaceFormat = surfaceFormat;
		m_ImageCount = imageCount;
		//-------------//

		VkSwapchainCreateInfoKHR swapChainCreateInfo{};
		swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainCreateInfo.surface = m_WindowSurface;
		swapChainCreateInfo.minImageCount = imageCount;
		swapChainCreateInfo.imageFormat = surfaceFormat.format;
		swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
		swapChainCreateInfo.imageExtent = extent;
		swapChainCreateInfo.imageArrayLayers = 1;

		// NOTE Renders directly to swap chain image; Use VK_IMAGE_USAGE_TRANSFER_DST_BIT for post processing
		swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyIndices[] = { m_QueueFamilyIndices.GraphicsFamily.value(), m_QueueFamilyIndices.PresentFamily.value() };

		if (m_QueueFamilyIndices.GraphicsFamily != m_QueueFamilyIndices.PresentFamily) {
			swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapChainCreateInfo.queueFamilyIndexCount = 2;
			swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapChainCreateInfo.queueFamilyIndexCount = 0; // Optional
			swapChainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		swapChainCreateInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
		swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapChainCreateInfo.presentMode = m_PresentMode;
		swapChainCreateInfo.clipped = VK_TRUE;
		swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(m_VkDevice, &swapChainCreateInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to create swap chain!");

		vkGetSwapchainImagesKHR(m_VkDevice, m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_VkDevice, m_SwapChain, &imageCount, m_SwapChainImages.data());

		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;

		//Application::Get().GetWindow().GetSceneCamera()->SetAspectRatio((float)m_SwapChainExtent.width / (float)m_SwapChainExtent.height);
	}

	void VulkanContext::CreateImageViews()
	{
		m_SwapChainImageViews.resize(m_SwapChainImages.size());

		for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
			m_SwapChainImageViews[i] = CreateImageView(m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
	}

	void VulkanContext::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_SwapChainImageFormat;
		colorAttachment.samples = m_MSAASamples;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // flag used b/c of msaa

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = FindDepthFormat();
		depthAttachment.samples = m_MSAASamples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = m_SwapChainImageFormat;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		// Subpass Dependencies
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;		

		std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(m_VkDevice, &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create render pass!");
	}

	void VulkanContext::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.pImmutableSamplers = nullptr; // For image sampling
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding baseSamplerLayoutBinding{};
		baseSamplerLayoutBinding.binding = 1;
		baseSamplerLayoutBinding.descriptorCount = 1;
		baseSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		baseSamplerLayoutBinding.pImmutableSamplers = nullptr;
		baseSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding armSamplerLayoutBinding{};
		armSamplerLayoutBinding.binding = 2;
		armSamplerLayoutBinding.descriptorCount = 1;
		armSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		armSamplerLayoutBinding.pImmutableSamplers = nullptr;
		armSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding normalSamplerLayoutBinding{};
		normalSamplerLayoutBinding.binding = 3;
		normalSamplerLayoutBinding.descriptorCount = 1;
		normalSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		normalSamplerLayoutBinding.pImmutableSamplers = nullptr;
		normalSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		/*VkDescriptorSetLayoutBinding metalSamplerLayoutBinding{};
		metalSamplerLayoutBinding.binding = 2;
		metalSamplerLayoutBinding.descriptorCount = 1;
		metalSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		metalSamplerLayoutBinding.pImmutableSamplers = nullptr;
		metalSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding roughnessSamplerLayoutBinding{};
		roughnessSamplerLayoutBinding.binding = 3;
		roughnessSamplerLayoutBinding.descriptorCount = 1;
		roughnessSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		roughnessSamplerLayoutBinding.pImmutableSamplers = nullptr;
		roughnessSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;*/


		/*VkDescriptorSetLayoutBinding ambientOcclusionSamplerLayoutBinding{};
		ambientOcclusionSamplerLayoutBinding.binding = 5;
		ambientOcclusionSamplerLayoutBinding.descriptorCount = 1;
		ambientOcclusionSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		ambientOcclusionSamplerLayoutBinding.pImmutableSamplers = nullptr;
		ambientOcclusionSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;*/

		std::array<VkDescriptorSetLayoutBinding, 4> bindings = { 
			uboLayoutBinding, 
			baseSamplerLayoutBinding, 
			armSamplerLayoutBinding, 
			normalSamplerLayoutBinding,
		};

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(m_VkDevice, &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create descriptor set layout!");
		
	}

	void VulkanContext::CreateComputeDescriptorSetLayout()
	{
		std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings{};
		layoutBindings[0].binding = 0;
		layoutBindings[0].descriptorCount = 1;
		layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[0].pImmutableSamplers = nullptr;
		layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		layoutBindings[1].binding = 1;
		layoutBindings[1].descriptorCount = 1;
		layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layoutBindings[1].pImmutableSamplers = nullptr;
		layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		layoutBindings[2].binding = 2;
		layoutBindings[2].descriptorCount = 1;
		layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layoutBindings[2].pImmutableSamplers = nullptr;
		layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 3;
		layoutInfo.pBindings = layoutBindings.data();

		if (vkCreateDescriptorSetLayout(m_VkDevice, &layoutInfo, nullptr, &m_ComputeDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create compute descriptor set layout!");
		}
	}

	void VulkanContext::CreateGraphicsPipeline()
	{
		// Shader Modules
		ShaderList shaderList = ParseShader(SHADER_PATH);
		VkShaderModule vertShaderModule = CreateShaderModule(shaderList.Vertex);
		VkShaderModule fragShaderModule = CreateShaderModule(shaderList.Fragment);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;

		// shader entrypoint - for fragment shaders, this means it is possible
		// to include several shaders(of the same type) in a single shader module and 
		// use different entry points to differentiate between their behaviors
		vertShaderStageInfo.pName = "main";

		// delcare shader constants here. Ex: float PI = 3.1415926535897932384626;
		vertShaderStageInfo.pSpecializationInfo = nullptr;


		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";
		fragShaderStageInfo.pSpecializationInfo = nullptr;


		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// Vertex Input
		auto bindingDescription = Vertex::GetBindingDescription();
		auto attributeDescriptions = Vertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional

		// Input Assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// Viewport & Scissors
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)m_SwapChainExtent.width;
		viewport.height = (float)m_SwapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_SwapChainExtent;

		// Dynamic State
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		// Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		// MSAA
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = m_MSAASamples;
		multisampling.minSampleShading = 1.f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		// Depth & Stencil Testing
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional

		// Color blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		// Push Constants
		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // The shader stages that will use the push constants.
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstants);

		MF_CORE_DEBUG("Size of Push Constants: {}", sizeof(PushConstants));

		// Pipeline Layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;	
		pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange; // Optional
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;

		if (vkCreatePipelineLayout(m_VkDevice, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to create pipeline layout!");


		// Pipeline struct
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;

		pipelineInfo.layout = m_PipelineLayout;

		pipelineInfo.renderPass = m_RenderPass;
		pipelineInfo.subpass = 0;

		if (vkCreateGraphicsPipelines(m_VkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to create graphics pipeline");

		vkDestroyShaderModule(m_VkDevice, vertShaderModule, nullptr);
		vkDestroyShaderModule(m_VkDevice, fragShaderModule, nullptr);
	}

	void VulkanContext::CreateParticleGraphicsPipeline()
	{
		m_ParticleShaderList = ParseShader(PARTICLE_SHADER_PATH);
		VkShaderModule vertShaderModule = CreateShaderModule(m_ParticleShaderList.Vertex);
		VkShaderModule fragShaderModule = CreateShaderModule(m_ParticleShaderList.Fragment);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		auto bindingDescription = Particle::GetBindingDescription();
		auto attributeDescriptions = Particle::GetAttributeDescriptions();

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = m_MSAASamples;
		multisampling.minSampleShading = 1.f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

		// Depth & Stencil Testing
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;

		if (vkCreatePipelineLayout(m_VkDevice, &pipelineLayoutInfo, nullptr, &m_ParticleGraphicsPipelineLayout) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create particle pipeline layout!");
		

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_ParticleGraphicsPipelineLayout;
		pipelineInfo.renderPass = m_RenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(m_VkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_ParticleGraphicsPipeline) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create graphics pipeline!");
		

		vkDestroyShaderModule(m_VkDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(m_VkDevice, vertShaderModule, nullptr);
	}

	void VulkanContext::CreateShaderStorageBuffers()
	{
		m_ShaderStorageBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		m_ShaderStorageBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

		// Initialize particles
		std::default_random_engine rndEngine((unsigned)time(nullptr));
		std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);

		// Initial particle positions on a circle
		std::vector<Particle> particles(PARTICLE_COUNT);
		for (auto& particle : particles) {
			float r = 0.25f * sqrt(rndDist(rndEngine));
			float theta = rndDist(rndEngine) * 2 * 3.14159265358979323846;
			float x = r * cos(theta) * HEIGHT / WIDTH;
			float y = r * sin(theta);
			particle.position = Maths::vec2(x, y);
			particle.velocity = Maths::normalize(Maths::vec2(x, y)) * 0.00025f;
			particle.color    = Maths::vec4(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine), 1.0f);
		}

		VkDeviceSize bufferSize = sizeof(Particle) * PARTICLE_COUNT;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		void* data;
		vkMapMemory(m_VkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, particles.data(), (size_t)bufferSize);
		vkUnmapMemory(m_VkDevice, stagingBufferMemory);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			CreateBuffer(
				bufferSize,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_ShaderStorageBuffers[i],
				m_ShaderStorageBuffersMemory[i]
			);

			// Copy data from the staging buffer (host) to the shader storage buffer (GPU)
			CopyBuffer(stagingBuffer, m_ShaderStorageBuffers[i], bufferSize);
		}

		std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings{};
		layoutBindings[0].binding = 0;
		layoutBindings[0].descriptorCount = 1;
		layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[0].pImmutableSamplers = nullptr;
		layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		layoutBindings[1].binding = 1;
		layoutBindings[1].descriptorCount = 1;
		layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layoutBindings[1].pImmutableSamplers = nullptr;
		layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		layoutBindings[2].binding = 2;
		layoutBindings[2].descriptorCount = 1;
		layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layoutBindings[2].pImmutableSamplers = nullptr;
		layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 3;
		layoutInfo.pBindings = layoutBindings.data();

		if (vkCreateDescriptorSetLayout(m_VkDevice, &layoutInfo, nullptr, &m_ComputeDescriptorSetLayout) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create COMPUTE descriptor set layout!");
		

	}

	void VulkanContext::CreateComputePipeline()
	{
		VkShaderModule computeShaderModule = CreateShaderModule(m_ParticleShaderList.Compute);

		VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
		computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStageInfo.module = computeShaderModule;
		computeShaderStageInfo.pName = "main";
		computeShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_ComputeDescriptorSetLayout;

		if (vkCreatePipelineLayout(m_VkDevice, &pipelineLayoutInfo, nullptr, &m_ComputePipelineLayout) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create compute pipeline layout!");
		

		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = m_ComputePipelineLayout;
		pipelineInfo.stage = computeShaderStageInfo;

		if (vkCreateComputePipelines(m_VkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_ComputePipeline) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create compute pipeline!");
		

	}

	void VulkanContext::CreateFrameBuffers()
	{
		m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());

		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
		{
			std::array<VkImageView, 3> attachments = { m_ColorImageView, m_DepthImageView, m_SwapChainImageViews[i] };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = m_SwapChainExtent.width;
			framebufferInfo.height = m_SwapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(m_VkDevice, &framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS)
				MF_CORE_ASSERT(false, "failed to create framebuffer!");
		}
	}

	void VulkanContext::CreateCommandPool()
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = m_QueueFamilyIndices.GraphicsFamily.value();

		if (vkCreateCommandPool(m_VkDevice, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create command pool!");
	}

	void VulkanContext::CreateColorResources()
	{
		VkFormat colorFormat = m_SwapChainImageFormat;

		CreateImage(
			m_SwapChainExtent.width, 
			m_SwapChainExtent.height, 
			1, m_MSAASamples, 
			colorFormat, 
			VK_IMAGE_TYPE_2D, 
			VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			m_ColorImage, 
			m_ColorImageMemory
		);

		m_ColorImageView = CreateImageView(m_ColorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	void VulkanContext::CreateDepthResources()
	{
		VkFormat depthFormat = FindDepthFormat();

		CreateImage(
			m_SwapChainExtent.width, 
			m_SwapChainExtent.height, 
			1,
			m_MSAASamples,
			depthFormat, 
			VK_IMAGE_TYPE_2D, // ???
			VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			m_DepthImage, 
			m_DepthImageMemory
		);

		m_DepthImageView = CreateImageView(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
	}

	void VulkanContext::CreateTextures()
	{
		m_Textures.resize(TEXTURE_PATHS.size());
		//for (size_t i = 0; i < m_Textures.size(); i++)
		//{
		//	m_Textures[i].Type = static_cast<TextureType>(i);
		//}
		m_Textures[0].Type = TextureType::DIFFUSE;
		m_Textures[1].Type = TextureType::ARM;
		m_Textures[2].Type = TextureType::NORMAL;

		for (auto& texture : m_Textures)
			CreateTextureImage(texture);

		for (auto& texture : m_Textures)
			CreateTextureImageView(texture);
	}

	void VulkanContext::CreateTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		if (m_SupportedFeatures.samplerAnisotropy)
		{
			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = m_Properties.limits.maxSamplerAnisotropy;
		}
		else
		{
			samplerInfo.anisotropyEnable = VK_FALSE;
			samplerInfo.maxAnisotropy = 1.0f;
		}
		
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0.0f; // Optional
		samplerInfo.maxLod = static_cast<float>(m_Textures[0].MipLevels); // all pbr textures of the same mesh should have the same dimensions and thus equal miplevels
		samplerInfo.mipLodBias = 0.0f; // Optional

		if (vkCreateSampler(m_VkDevice, &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create texture sampler!");
	}

	void VulkanContext::LoadModel()
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str()))
		{
			MF_CORE_WARN(warn);
			MF_CORE_ERROR(err);
		}

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		std::vector<Vertex>    vertices;
		std::vector<uint32_t>  indices;

		// Note that this code(tangents and bitangents) does not handle mirrored UV sand other special cases, 
		// and it does not normalize the vectors, which you may want to do depending on how you use them.

		std::vector<std::vector<Maths::vec3>> tempTangents(attrib.vertices.size() / 3);
		std::vector<std::vector<Maths::vec3>> tempBitangents(attrib.vertices.size() / 3);

			
		for (const auto& shape : shapes) 
		{
			for (size_t i = 0; i < shape.mesh.indices.size(); i += 3)
			{
				tinyobj::index_t idx1 = shape.mesh.indices[i];
				tinyobj::index_t idx2 = shape.mesh.indices[i + 1];
				tinyobj::index_t idx3 = shape.mesh.indices[i + 2];

				// Grab the three vertices of the current face
				Maths::vec3 pos1 = Maths::vec3(&attrib.vertices[3 * idx1.vertex_index]);
				Maths::vec3 pos2 = Maths::vec3(&attrib.vertices[3 * idx2.vertex_index]);
				Maths::vec3 pos3 = Maths::vec3(&attrib.vertices[3 * idx3.vertex_index]);

				// Same for the texture coordinates
				Maths::vec2 uv1 = Maths::vec2(&attrib.texcoords[2 * idx1.texcoord_index]);
				Maths::vec2 uv2 = Maths::vec2(&attrib.texcoords[2 * idx2.texcoord_index]);
				Maths::vec2 uv3 = Maths::vec2(&attrib.texcoords[2 * idx3.texcoord_index]);

				// Edges of the triangle in position and texture space
				Maths::vec3 deltaPos1 = pos2 - pos1;
				Maths::vec3 deltaPos2 = pos3 - pos1;
				Maths::vec2 deltaUV1 = uv2 - uv1;
				Maths::vec2 deltaUV2 = uv3 - uv1;

				// This will give us a direction vector pointing in the direction of positive s for this triangle.
				// It calculates how much each vertex position needs to be adjusted in space to match how it was proportionally stretched when the texture was mapped.
				float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
				Maths::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
				Maths::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

				// Store the tangents and bitangents in the temporary vectors
				tempTangents[idx1.vertex_index].push_back(tangent);
				tempBitangents[idx1.vertex_index].push_back(bitangent);
				tempTangents[idx2.vertex_index].push_back(tangent);
				tempBitangents[idx2.vertex_index].push_back(bitangent);
				tempTangents[idx3.vertex_index].push_back(tangent);
				tempBitangents[idx3.vertex_index].push_back(bitangent);
			}

			for (const auto& index : shape.mesh.indices) 
			{

				Vertex vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.color = { 1.0f, 1.0f, 1.0f };

				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				vertex.tangent = Maths::normalize(std::accumulate(tempTangents[index.vertex_index].begin(), tempTangents[index.vertex_index].end(), Maths::vec3(0.0f)));
				vertex.bitangent = Maths::normalize(std::accumulate(tempBitangents[index.vertex_index].begin(), tempBitangents[index.vertex_index].end(), Maths::vec3(0.0f)));


				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}

		Application& app = Application::Get();

		{
			size_t bufferSize = vertices.size() * sizeof(Vertex); // Buffer size
			constexpr size_t alignment = 32; // Alignment requirement

			// Allocate extra memory to accommodate alignment
			Scope<char[]> vertexData(new char[bufferSize + alignment]);
			void* ptr = vertexData.get();
			size_t space = bufferSize + alignment;

			void* alignedPtr = std::align(alignment, bufferSize, ptr, space);


			if (alignedPtr != nullptr)
			{
				// Copy the vertex data to the aligned memory
				std::memcpy(alignedPtr, vertices.data(), bufferSize);

				// Create the Span from the aligned memory
				Span<const uint8_t> vertexDataSpan(reinterpret_cast<const uint8_t*>(alignedPtr), bufferSize);

				// Now you can set InitData in BufferDesc with vertexDataSpan
				// Note: Make sure original is not destroyed until you're done with vertexDataSpan
				app.SetVertices(std::move(vertexDataSpan));
				app.SetVertexData(std::move(vertexData));
			}
			else
			{
				MF_CORE_ASSERT(false, "Failed to properly align data.");
			}
		}

		Span<const uint8_t> indexDataSpan(reinterpret_cast<const uint8_t*>(indices.data()), indices.size() * sizeof(uint32_t));
		app.SetIndices(std::move(indexDataSpan));
		app.SetIndexData(std::move(indices));
		
		
	}

	//void VulkanContext::CreateVertexBuffer()
	//{
	//	// TODO:
	//	// The previous chapter already mentioned that you should allocate multiple resources like 
	//	// buffers from a single memory allocation, but in fact you should go a step further. Driver 
	//	// developers recommend that you also store multiple buffers, like the vertex and index 
	//	// buffer, into a single VkBuffer and use offsets in commands like vkCmdBindVertexBuffers. 
	//	// The advantage is that your data is more cache friendly in that case, because it's closer 
	//	// together. It is even possible to reuse the same chunk of memory for multiple resources 
	//	// if they are not used during the same render operations, provided that their data is 
	//	// refreshed, of course. This is known as aliasing and some Vulkan functions have explicit 
	//	 // flags to specify that you want to do this.
	//	
	//	VkDeviceSize bufferSize = static_cast<uint64_t>(sizeof(m_Vertices[0]) * m_Vertices.size());

	//	VkBuffer stagingBuffer;
	//	VkDeviceMemory stagingBufferMemory;

	//	CreateBuffer(
	//		bufferSize, 
	//		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
	//		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
	//		stagingBuffer, 
	//		stagingBufferMemory
	//	);

	//	void* data;
	//	vkMapMemory(m_VkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	//	memcpy(data, m_Vertices.data(), (size_t)bufferSize);
	//	vkUnmapMemory(m_VkDevice, stagingBufferMemory);

	//	CreateBuffer(
	//		bufferSize,
	//		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	//		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	//		m_VertexBuffer,
	//		m_VertexBufferMemory
	//	);

	//	CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

	//	vkDestroyBuffer(m_VkDevice, stagingBuffer, nullptr);
	//	vkFreeMemory(m_VkDevice, stagingBufferMemory, nullptr);
	//}

	//void VulkanContext::CreateIndexBuffer()
	//{
	//	VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

	//	VkBuffer stagingBuffer;
	//	VkDeviceMemory stagingBufferMemory;
	//	CreateBuffer(
	//		bufferSize, 
	//		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
	//		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
	//		stagingBuffer, 
	//		stagingBufferMemory
	//	);

	//	void* data;
	//	vkMapMemory(m_VkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	//	memcpy(data, m_Indices.data(), (size_t)bufferSize);
	//	vkUnmapMemory(m_VkDevice, stagingBufferMemory);

	//	CreateBuffer(
	//		bufferSize, 
	//		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
	//		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
	//		m_IndexBuffer, 
	//		m_IndexBufferMemory
	//	);

	//	CopyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

	//	vkDestroyBuffer(m_VkDevice, stagingBuffer, nullptr);
	//	vkFreeMemory(m_VkDevice, stagingBufferMemory, nullptr);
	//}

	/*void VulkanContext::CreateUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		m_UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		m_UniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		m_UniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			CreateBuffer(
				bufferSize, 
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				m_UniformBuffers[i], 
				m_UniformBuffersMemory[i]
			);

			vkMapMemory(m_VkDevice, m_UniformBuffersMemory[i], 0, bufferSize, 0, &m_UniformBuffersMapped[i]);
		}
	}*/

	void VulkanContext::CreateComputeUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(ParticleUniformBufferObject);

		m_ComputeUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		m_ComputeUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		m_ComputeUniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			CreateBuffer(
				bufferSize, 
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				m_ComputeUniformBuffers[i], 
				m_ComputeUniformBuffersMemory[i]
			);

			vkMapMemory(m_VkDevice, m_ComputeUniformBuffersMemory[i], 0, bufferSize, 0, &m_ComputeUniformBuffersMapped[i]);
		}
	}

	

	void VulkanContext::CreateDescriptorPool()
	{
		VkDescriptorPoolSize uboPoolSize{};
		uboPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboPoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolSize samplerPoolSize{};
		samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerPoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		std::array<VkDescriptorPoolSize, 2> poolSizes{ uboPoolSize, samplerPoolSize };

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolInfo.flags = 0; // VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT - determines if individual descriptor sets can be freed

		if (vkCreateDescriptorPool(m_VkDevice, &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create descriptor pool!");
	}

	void VulkanContext::CreateComputeDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};

		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 2;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(m_VkDevice, &poolInfo, nullptr, &m_ComputeDescriptorPool) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create COMPUTE descriptor pool!");
	}

	void VulkanContext::CreateDescriptorSets()
	{
		Handle<Buffer>& uniformHandle = Application::Get().GetUniformBufferHandle();
		VulkanUniformBuffer& uniformBuffer = static_cast<VulkanUniformBuffer&>(Application::Get().GetResourceManager().GetBuffer(uniformHandle));
		

		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(m_VkDevice, &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to allocate descriptor sets!");

		std::vector<VkDescriptorImageInfo> imageInfo{};
		imageInfo.resize(m_Textures.size());

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
		{
			// UBO
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffer.GetBuffers()[i];   //m_UniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);
			
			// Diffuse
			imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo[0].imageView = m_Textures[0].ImageView;
			imageInfo[0].sampler =   m_TextureSampler;

			// ARM
			imageInfo[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo[1].imageView = m_Textures[1].ImageView;
			imageInfo[1].sampler = m_TextureSampler;

			// Normal
			imageInfo[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo[2].imageView = m_Textures[2].ImageView;
			imageInfo[2].sampler = m_TextureSampler;


			std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

			// UBO
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = m_DescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
			descriptorWrites[0].pImageInfo = nullptr; // Optional
			descriptorWrites[0].pTexelBufferView = nullptr; // Optional

			// Diffuse Texture
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = m_DescriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = nullptr;
			descriptorWrites[1].pImageInfo = &imageInfo[0]; 
			descriptorWrites[1].pTexelBufferView = nullptr; // Optional

			// ARM Texture
			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = m_DescriptorSets[i];
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].pBufferInfo = nullptr;
			descriptorWrites[2].pImageInfo = &imageInfo[1]; 
			descriptorWrites[2].pTexelBufferView = nullptr; // Optional

			// Normal Texture
			descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[3].dstSet = m_DescriptorSets[i];
			descriptorWrites[3].dstBinding = 3;
			descriptorWrites[3].dstArrayElement = 0;
			descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[3].descriptorCount = 1;
			descriptorWrites[3].pBufferInfo = nullptr;
			descriptorWrites[3].pImageInfo = &imageInfo[2];
			descriptorWrites[3].pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(m_VkDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
		
	}

	void VulkanContext::CreateComputeDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_ComputeDescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_ComputeDescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		m_ComputeDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(m_VkDevice, &allocInfo, m_ComputeDescriptorSets.data()) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to allocate COMPUTE descriptor sets!");

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

			VkDescriptorBufferInfo uniformBufferInfo{};
			uniformBufferInfo.buffer = m_ComputeUniformBuffers[i];
			uniformBufferInfo.offset = 0;
			uniformBufferInfo.range = sizeof(ParticleUniformBufferObject);

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = m_ComputeDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &uniformBufferInfo;
			descriptorWrites[0].pImageInfo = nullptr; // Optional
			descriptorWrites[0].pTexelBufferView = nullptr; // Optional

			VkDescriptorBufferInfo storageBufferInfoLastFrame{};
			storageBufferInfoLastFrame.buffer = m_ShaderStorageBuffers[(i - 1) % MAX_FRAMES_IN_FLIGHT];
			storageBufferInfoLastFrame.offset = 0;
			storageBufferInfoLastFrame.range = sizeof(Particle) * PARTICLE_COUNT;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = m_ComputeDescriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &storageBufferInfoLastFrame;

			VkDescriptorBufferInfo storageBufferInfoCurrentFrame{};
			storageBufferInfoCurrentFrame.buffer = m_ShaderStorageBuffers[i];
			storageBufferInfoCurrentFrame.offset = 0;
			storageBufferInfoCurrentFrame.range = sizeof(Particle) * PARTICLE_COUNT;

			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = m_ComputeDescriptorSets[i];
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].pBufferInfo = &storageBufferInfoCurrentFrame;

			vkUpdateDescriptorSets(m_VkDevice, 3, descriptorWrites.data(), 0, nullptr);

		}
	}

	void VulkanContext::CreateCommandBuffers()
	{
		m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

		if (vkAllocateCommandBuffers(m_VkDevice, &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to allocate command buffers!");

		m_ImGuiCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_ImGuiCommandBuffers.size();

		if (vkAllocateCommandBuffers(m_VkDevice, &allocInfo, m_ImGuiCommandBuffers.data()) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to allocate command buffers!");
		
	}

	void VulkanContext::CreateComputeCommandBuffers()
	{
		m_ComputeCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_ComputeCommandBuffers.size();

		if (vkAllocateCommandBuffers(m_VkDevice, &allocInfo, m_ComputeCommandBuffers.data()) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to allocate compute command buffers!");
		
	}

	void VulkanContext::CreateSyncObjects()
	{
		
		m_ComputeFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_ComputeInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo computeSemaphoreInfo{};
		computeSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo computeFenceInfo{};
		computeFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		computeFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


		m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_ImGuiRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_ImGuiInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(m_VkDevice, &computeSemaphoreInfo, nullptr, &m_ComputeFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_VkDevice, &computeFenceInfo, nullptr, &m_ComputeInFlightFences[i]) != VK_SUCCESS)
			{
				MF_CORE_ASSERT(false, "failed to create COMPUTE semaphores and/or fences!");
			}

			if (vkCreateSemaphore(m_VkDevice, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_VkDevice, &semaphoreInfo, nullptr, &m_ImGuiRenderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_VkDevice, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_VkDevice, &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS ||
				vkCreateFence(m_VkDevice, &fenceInfo, nullptr, &m_ImGuiInFlightFences[i]) != VK_SUCCESS)
			{
				MF_CORE_ASSERT(false, "failed to create GRAPHICS semaphores and/or fences!");
			}
		}
		
	}

	std::vector<const char*> VulkanContext::GetRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (m_EnableValidationLayers)
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);


		return extensions;
	}

	void VulkanContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr; // Optional
	}

	bool VulkanContext::IsDeviceSuitable(VkPhysicalDevice device)
	{
		// TODO: allow user to choose their graphics card from list of
		// vulkan-supported options.

		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		m_QueueFamilyIndices = FindQueueFamilies(device);
		bool extensionsSupported = CheckDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) 
		{
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
		}
			
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			m_QueueFamilyIndices.IsComplete() &&
			extensionsSupported &&
			swapChainAdequate &&
			deviceFeatures.samplerAnisotropy)
		{
			m_SupportedFeatures = deviceFeatures;
			m_Properties = deviceProperties;

			MF_CORE_DEBUG("Max Push Constant Size: {}", m_Properties.limits.maxPushConstantsSize);


			//MF_CORE_DEBUG("Renderer Info: ");
			//MF_CORE_DEBUG("\tVersion: {}", m_RendererInfo.Version);
			//MF_CORE_DEBUG("\tVendor: {}", m_RendererInfo.Vendor);
			//MF_CORE_DEBUG("\tRenderer: {}", m_RendererInfo.Renderer);
			return true;
		}

		return false;
		
	}

	bool VulkanContext::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	QueueFamilyIndices VulkanContext::FindQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) 
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_WindowSurface, &presentSupport);

			// Preferring a physical device that supports drawing and presentation in the same queue
			if (presentSupport && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
			{
				indices.PresentFamily = i;
				indices.GraphicsFamily = i;
				indices.ComputeFamily = i;
				MF_CORE_DEBUG("Supported Number of Queues: {}", queueFamily.queueCount);
			}

			/*if (presentSupport)
				indices.PresentFamily = i;

			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
				indices.GraphicsFamily = i;*/

			if (indices.IsComplete())
				break;
			
			i++;
		}

		return indices;
	}

	SwapChainSupportDetails VulkanContext::QuerySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		// Formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_WindowSurface, &formatCount, nullptr);

		if (formatCount != 0) 
		{
			details.Formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_WindowSurface, &formatCount, details.Formats.data());
		}

		// Presentation Modes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_WindowSurface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.PresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_WindowSurface, &presentModeCount, details.PresentModes.data());
		}

		// Capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_WindowSurface, &details.Capabilities);

		return details;
	}

	VkSurfaceFormatKHR VulkanContext::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR VulkanContext::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanContext::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(m_WindowHandle, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	void VulkanContext::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, VulkanBuffer& vertexBuffer, VulkanBuffer& indexBuffer, uint32_t indexCount)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to begin recording command buffer!");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_RenderPass;
		renderPassInfo.framebuffer = m_SwapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapChainExtent;

		std::array<VkClearValue, 2> clearValues{};

		clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			// MODEL PIPELINE
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
			
			{
				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = static_cast<float>(m_SwapChainExtent.width);
				viewport.height = static_cast<float>(m_SwapChainExtent.height);
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

				VkRect2D scissor{};
				scissor.offset = { 0, 0 };
				scissor.extent = m_SwapChainExtent;
				vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

				VkBuffer vertexBuffers[] = { vertexBuffer.GetBuffer()};
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

				vkCmdBindIndexBuffer(commandBuffer, indexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSets[m_CurrentFrame], 0, nullptr);

				vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &m_PushConstants);

				vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
			}

			// PARTICLE PIPELINE
			/*vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ParticleGraphicsPipeline);

			{
				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = (float)m_SwapChainExtent.width;
				viewport.height = (float)m_SwapChainExtent.height;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

				VkRect2D scissor{};
				scissor.offset = { 0, 0 };
				scissor.extent = m_SwapChainExtent;
				vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_ShaderStorageBuffers[m_CurrentFrame], offsets);

				vkCmdDraw(commandBuffer, PARTICLE_COUNT, 1, 0, 0);
			}*/

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to record command buffer!");
	}


	// The disadvantage of this approach is that we need to stop all rendering before creating the new swap chain.
	// It is possible to create a new swap chain while drawing commands on an image from the old swap chain are still 
	// in - flight.You need to pass the previous swap chain to the oldSwapChain field in the VkSwapchainCreateInfoKHR 
	// struct and destroy the old swap chain as soon as you've finished using it.
	void VulkanContext::RecreateSwapChain()
	{
		m_SwapChainRebuild = true; // for ImGui

		int width = 0, height = 0;
		glfwGetFramebufferSize(m_WindowHandle, &width, &height);
		while (width == 0 || height == 0) 
		{
			glfwGetFramebufferSize(m_WindowHandle, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_VkDevice);

		CleanupSwapChain();

		CreateSwapChain();
		CreateImageViews();
		CreateColorResources();
		CreateDepthResources();
		CreateFrameBuffers();

		// Recreate ImGui Resources
		Application::Get().GetImGuiLayer()->RecreateImageResources();

	}

	void VulkanContext::CleanupSwapChain()
	{
		vkDestroyImageView(m_VkDevice, m_ColorImageView, nullptr);
		vkDestroyImage(m_VkDevice, m_ColorImage, nullptr);
		vkFreeMemory(m_VkDevice, m_ColorImageMemory, nullptr);

		vkDestroyImageView(m_VkDevice, m_DepthImageView, nullptr);
		vkDestroyImage(m_VkDevice,     m_DepthImage, nullptr);
		vkFreeMemory(m_VkDevice,       m_DepthImageMemory, nullptr);

		for (auto framebuffer : m_SwapChainFramebuffers)
			vkDestroyFramebuffer(m_VkDevice, framebuffer, nullptr);

		for (auto imageView : m_SwapChainImageViews)
			vkDestroyImageView(m_VkDevice, imageView, nullptr);

		vkDestroySwapchainKHR(m_VkDevice, m_SwapChain, nullptr);
	}

	uint32_t VulkanContext::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_VkPhysicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		MF_CORE_ASSERT(false, "failed to find suitable memory type!");

		return 0;
	}

	void VulkanContext::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		//bufferInfo.flags = 

		if (vkCreateBuffer(m_VkDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create vertex buffer!");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_VkDevice, buffer, &memRequirements);

		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.allocationSize = memRequirements.size;
		memAllocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		// TODO:
		// It should be noted that in a real world application, you're not supposed to actually call
		// vkAllocateMemory for every individual buffer. The maximum number of simultaneous memory 
		// allocations is limited by the maxMemoryAllocationCount physical device limit, which may 
		// be as low as 4096 even on high end hardware like an NVIDIA GTX 1080. The right way to 
		// allocate memory for a large number of objects at the same time is to create a custom 
		// allocator that splits up a single allocation among many different objects by using the 
		// offset parameters that we've seen in many functions.

		// You can either implement such an allocator yourself, or use the VulkanMemoryAllocator 
		// library provided by the GPUOpen initiative.However, for this tutorial it's okay to use a 
		// separate allocation for every resource, because we won't come close to hitting any of 
		// these limits for now.
		if (vkAllocateMemory(m_VkDevice, &memAllocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to allocate vertex buffer memory!");

		vkBindBufferMemory(m_VkDevice, buffer, bufferMemory, 0);
	}

	void VulkanContext::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
		
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(commandBuffer);
	}

	//void VulkanContext::UpdateUniformBuffer()
	//{
	//	/*static auto startTime = std::chrono::high_resolution_clock::now();

	//	auto currentTime = std::chrono::high_resolution_clock::now();
	//	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();*/

	//	auto& camera = Application::Get().GetWindow().GetSceneCamera();
	//	camera->SetAspectRatio((float)m_SwapChainExtent.width / (float)m_SwapChainExtent.height);
	//	
	//	UniformBufferObject ubo{};
	//	//ubo.model = Maths::Quaternion::CalculateRotationMatrix(time * 45.f, Maths::vec3(0.0f, 1.0f, 0.0f));
	//	ubo.model = Maths::Quaternion::CalculateRotationMatrix(0.f, Maths::vec3(0.0f, 1.0f, 0.0f));
	//	ubo.view = camera->CalculateView();
	//	ubo.proj = camera->CalculateProjection();

	//	ubo.proj.c[1].e[1] *= -1;

	//	memcpy(m_UniformBuffersMapped[m_CurrentFrame], &ubo, sizeof(ubo));
	//}

	void VulkanContext::UpdateComputeUniformBuffer()
	{
		//Application& app = Application::Get();
		ParticleUniformBufferObject ubo{};
		ubo.deltaTime = Application::Get().GetTimeStep().GetDeltaTime() * 1000.f * 2.0f;

		memcpy(m_ComputeUniformBuffersMapped[m_CurrentFrame], &ubo, sizeof(ubo));
	}

	void VulkanContext::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageType imageType, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		VkPhysicalDeviceImageFormatInfo2 imageFormatInfo = {};
		imageFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
		imageFormatInfo.format = format;  // replace with the format you want to query
		imageFormatInfo.type = imageType; // or whatever image type you're interested in
		imageFormatInfo.tiling = tiling;
		imageFormatInfo.usage = usage;
		imageFormatInfo.flags = 0; 

		VkImageFormatProperties2 imageFormatProperties = {};
		imageFormatProperties.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;

		if (vkGetPhysicalDeviceImageFormatProperties2(m_VkPhysicalDevice, &imageFormatInfo, &imageFormatProperties) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Image format w/ specified properties not supported!");

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = imageType;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = numSamples;
		imageInfo.flags = 0; // Optional

		if (vkCreateImage(m_VkDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to create image!");

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(m_VkDevice, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(m_VkDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to allocate image memory!");

		vkBindImageMemory(m_VkDevice, image, imageMemory, 0);
	}

	VkCommandBuffer VulkanContext::BeginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_VkDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void VulkanContext::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

		//A fence would allow you to schedule multiple transfers simultaneously
		// and wait for all of them complete, instead of executing one at a time.
		// That may give the driver more opportunities to optimize.
		// 
		//vkWaitForFences()
		vkQueueWaitIdle(m_GraphicsQueue);

		vkFreeCommandBuffers(m_VkDevice, m_CommandPool, 1, &commandBuffer);
	}



	// TODO(async):
	// All of the helper functions that submit commands so far have been set up to execute synchronously by waiting 
	// for the queue to become idle. For practical applications it is recommended to combine these operations in a 
	// single command buffer and execute them asynchronously for higher throughput, especially the transitions and 
	// copy in the createTextureImage function. Try to experiment with this by creating a setupCommandBuffer that 
	// the helper functions record commands into, and add a flushSetupCommands to execute the commands that have been 
	// recorded so far. It's best to do this after the texture mapping works to check if the texture resources are 
	// still set up correctly.

	void VulkanContext::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else 
		{
			MF_CORE_ASSERT(false, "unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		EndSingleTimeCommands(commandBuffer);
	}

	void VulkanContext::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		EndSingleTimeCommands(commandBuffer);
	}

	VkImageView VulkanContext::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // VK_IMAGE_VIEW_TYPE_CUBE -> for cube map
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;



		VkImageView imageView;
		if (vkCreateImageView(m_VkDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to create image or texture image view!");

		return imageView;
	}

	VkFormat VulkanContext::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates) 
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_VkPhysicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) 
				return format;
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) 
				return format;
		}

		MF_CORE_ASSERT(false, "Failed to find supported format");
	}

	VkFormat VulkanContext::FindDepthFormat()
	{
		return FindSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	bool VulkanContext::HasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	void VulkanContext::GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
	{
		// Check if image format supports linear blitting(linear filtering)
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(m_VkPhysicalDevice, imageFormat, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
			throw std::runtime_error("texture image format does not support linear blitting!");
		}

		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

	
		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++) 
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;
			
			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		EndSingleTimeCommands(commandBuffer);

	}


	void VulkanContext::CreateTextureImage(TextureInfo& texture)
	{
		int requestedChannels = 0;

		stbi_set_flip_vertically_on_load(0);
		texture.Tiling = VK_IMAGE_TILING_OPTIMAL;
		switch (texture.Type)
		{
			case TextureType::DIFFUSE:
			{
				texture.Format = VK_FORMAT_R8G8B8A8_SRGB;
				requestedChannels = STBI_rgb_alpha;
				break;
			}

			// TODO: Update arm and normal to just use rgb in both format and requested channels 
			case TextureType::ARM:
			{
				texture.Format = VK_FORMAT_R8G8B8A8_UNORM;
				requestedChannels = STBI_rgb_alpha;
				break;
			}

			case TextureType::NORMAL:
			{
				texture.Format = VK_FORMAT_R8G8B8A8_UNORM;
				requestedChannels = STBI_rgb_alpha;
				break;
			}

			/*case TextureType::AO || TextureType::ROUGHNESS || TextureType::METAL:
			{
				texture.Format = VK_FORMAT_R8_UNORM;
				requestedChannels = STBI_grey;
				break;
			}*/

			default:
			{
				MF_CORE_ASSERT(false, "CreateTextureImage - Unknown or unsupported texture type");
			}
			break;
		}


		int width, height, channels;
		stbi_uc* pixels = stbi_load(
			TEXTURE_PATHS[static_cast<uint32_t>(texture.Type)].c_str(), 
			&width, &height, &channels, 
			requestedChannels
		); // channels = BPP (bits per pixel)

		if (!pixels)
			MF_CORE_ASSERT(false, "failed to load texture image!");

		texture.MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

		MF_CORE_DEBUG("Mip Levels: {0} | Width: {1} | Height: {2} | Channels: {3} | Requested Channels: {4}", texture.MipLevels, width, height, channels, requestedChannels);

		VkDeviceSize imageSize = width * height * requestedChannels;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		void* data;
		vkMapMemory(m_VkDevice, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(m_VkDevice, stagingBufferMemory);

		stbi_image_free(pixels);

		CreateImage(
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height),
			texture.MipLevels,
			VK_SAMPLE_COUNT_1_BIT,
			texture.Format,
			VK_IMAGE_TYPE_2D,
			texture.Tiling,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			texture.Image,
			texture.Buffer
		);

		TransitionImageLayout(texture.Image, texture.Format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture.MipLevels);
		CopyBufferToImage(stagingBuffer, texture.Image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
		//TransitionImageLayout(m_TextureImage, texture.Format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_MipLevels);
		GenerateMipmaps(texture.Image, texture.Format, width, height, texture.MipLevels);

		vkDestroyBuffer(m_VkDevice, stagingBuffer, nullptr);
		vkFreeMemory(m_VkDevice, stagingBufferMemory, nullptr);
	}

	void VulkanContext::CreateTextureImageView(TextureInfo& texture)
	{
		texture.ImageView = CreateImageView(texture.Image, texture.Format, VK_IMAGE_ASPECT_COLOR_BIT, texture.MipLevels);
	}

	VkSampleCountFlagBits VulkanContext::GetMaxUsableSampleCount()
	{
		VkSampleCountFlags counts = m_Properties.limits.framebufferColorSampleCounts & m_Properties.limits.framebufferDepthSampleCounts;

		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}

	void VulkanContext::RecordComputeCommandBuffer(VkCommandBuffer commandBuffer)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to begin recording compute command buffer!");
		

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipelineLayout, 0, 1, &m_ComputeDescriptorSets[m_CurrentFrame], 0, nullptr);

		vkCmdDispatch(commandBuffer, PARTICLE_COUNT / 256, 1, 1);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to record compute command buffer!");
		
	}

	void VulkanContext::PerformComputeOps()
	{
		// COMPUTE SUBMISSION //

		// Wait for previous frame
		vkWaitForFences(m_VkDevice, 1, &m_ComputeInFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		UpdateComputeUniformBuffer();

		vkResetFences(m_VkDevice, 1, &m_ComputeInFlightFences[m_CurrentFrame]);

		vkResetCommandBuffer(m_ComputeCommandBuffers[m_CurrentFrame], /*VkCommandBufferResetFlagBits*/ 0);
		RecordComputeCommandBuffer(m_ComputeCommandBuffers[m_CurrentFrame]);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_ComputeCommandBuffers[m_CurrentFrame];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_ComputeFinishedSemaphores[m_CurrentFrame];

		if (vkQueueSubmit(m_ComputeQueue, 1, &submitInfo, m_ComputeInFlightFences[m_CurrentFrame]) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to submit compute command buffer!");
	}

	void VulkanContext::PerformGraphicsOps()
	{
		Application& app = Application::Get();
		ResourceManager& rm = Application::Get().GetResourceManager();
		
		VulkanUniformBuffer& uniformBuffer = static_cast<VulkanUniformBuffer&>(rm.GetBuffer(app.GetUniformBufferHandle()));
		VulkanBuffer& vertexBuffer = static_cast<VulkanBuffer&>(rm.GetBuffer(app.GetVertexBufferHandle()));
		VulkanBuffer& indexBuffer = static_cast<VulkanBuffer&>(rm.GetBuffer(app.GetIndexBufferHandle()));

		// GRAPHICS SUBMISSION //

		// Wait for previous frame
		vkWaitForFences(m_VkDevice, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		// Acquire image from swap chain
		VkResult result = vkAcquireNextImageKHR(m_VkDevice, m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &m_ImageIndex);


		m_SwapChainRebuild = false;
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
			RecreateSwapChain();
		else
			MF_CORE_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed to acquire next image for swap chain.");

		// Reset fences
		vkResetFences(m_VkDevice, 1, &m_InFlightFences[m_CurrentFrame]);

		// Reset and Record Command Buffer - Game Objects
		vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);
		RecordCommandBuffer(m_CommandBuffers[m_CurrentFrame], m_ImageIndex, vertexBuffer, indexBuffer, app.GetIndexCount());

		uniformBuffer.UpdateUniformBuffer();

		//VkSemaphore waitSemaphores[] = { m_ComputeFinishedSemaphores[m_CurrentFrame], m_ImageAvailableSemaphores[m_CurrentFrame] };
		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
		//VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		//submitInfo.waitSemaphoreCount = 2;
		submitInfo.waitSemaphoreCount = 1; 
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];

		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to submit draw command buffer!");


		// Wait for Game Object Command Execution to Finish
		vkWaitForFences(m_VkDevice, 1, &m_ImGuiInFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		// Reset fences
		vkResetFences(m_VkDevice, 1, &m_ImGuiInFlightFences[m_CurrentFrame]);

		Application::Get().GetImGuiLayer()->RecordAndSubmitCommandBuffer(m_ImageIndex);

	}

	void VulkanContext::PresentImage()
	{
		VkSemaphore signalSemaphores[] = { m_ImGuiRenderFinishedSemaphores[m_CurrentFrame] };
		//VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame]};

		// Presentation (submitting image back to swap chain)
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { m_SwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &m_ImageIndex;

		presentInfo.pResults = nullptr; // Optional

		VkResult result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized)
		{
			m_FramebufferResized = false;
			RecreateSwapChain();
		}
		else
		{
			MF_CORE_ASSERT(result == VK_SUCCESS, "Failed to submit image back to swap chain");
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	VkShaderModule VulkanContext::CreateShaderModule(const ShaderSource& source)
	{
		//String rawText = ReadFile(source.Text);

		// Compile shader to spv binary
		// Create an instance of the compiler
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		// Compile the shader code
		shaderc::SpvCompilationResult result;
		
		switch (source.Type)
		{
			case Magnefu::ShaderType::Vertex:
			{
				result = compiler.CompileGlslToSpv(source.Text, shaderc_vertex_shader, "vertex.glsl", options);
				break;
			}
			case Magnefu::ShaderType::Fragment:
			{
				result = compiler.CompileGlslToSpv(source.Text, shaderc_fragment_shader, "fragment.glsl", options);
				break;
			}
			case Magnefu::ShaderType::Compute:
			{
				result = compiler.CompileGlslToSpv(source.Text, shaderc_compute_shader, "compute.glsl", options);
				break;
			}
			default:
			{
				MF_CORE_ASSERT(false, "Unknown shader type: {}", static_cast<int>(source.Type));
				break;
			}
		}
		

		if (result.GetCompilationStatus() != shaderc_compilation_status_success) 
		{
			// Compilation failed, handle the error
			MF_CORE_ASSERT(false, "Error: Compilation failed: {} ", result.GetErrorMessage());
		}

		// The result object is an iterable object providing a begin() and end() iterator for the SPIR-V binary.
		std::vector<uint32_t> spirv_binary(result.cbegin(), result.cend());

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = spirv_binary.size() * sizeof(uint32_t);
		createInfo.pCode = spirv_binary.data();

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(m_VkDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
		{
			MF_CORE_ASSERT(false, "Failed to create a shader module");
		}

		return shaderModule;
	}


	ShaderList VulkanContext::ParseShader(const String& filepath)
	{
		std::ifstream stream(filepath);

		if (!stream.good())
			MF_CORE_ASSERT(false, "Failed to load shader contents");

		String line;
		
		std::stringstream ss[3];
		ShaderType type = ShaderType::None;
		while (std::getline(stream, line))
		{
			if (line.find("#shader") != String::npos)
			{
				if (line.find("vertex") != String::npos)
				{
					type = ShaderType::Vertex;
				}
				else if (line.find("fragment") != String::npos)
				{
					type = ShaderType::Fragment;
				}
				else if (line.find("compute") != String::npos)
				{
					type = ShaderType::Compute;
				}
			}
			else
			{
				ss[(int)type] << line << '\n';
			}
		}

		return {
			{ ss[0].str(), ShaderType::Vertex },
			{ ss[1].str(), ShaderType::Fragment },
			{ ss[2].str(), ShaderType::Compute }
		};
	}
}
