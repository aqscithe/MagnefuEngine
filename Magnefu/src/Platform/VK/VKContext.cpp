#include "mfpch.h"
#include <iterator>

#include "VKContext.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "shaderc/shaderc.hpp"

#include <set>

namespace Magnefu
{

	static const int MAX_FRAMES_IN_FLIGHT = 2;

	static const std::vector<const char*> deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
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

	VKContext::VKContext(GLFWwindow* windowHandle) :
		m_WindowHandle(windowHandle), m_VkInstance(VkInstance()), m_VkPhysicalDevice(VK_NULL_HANDLE)
	{
		MF_CORE_ASSERT(m_WindowHandle, "Window Handle is null!");
	}

	VKContext::~VKContext()
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_VkDevice, m_ImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(m_VkDevice, m_RenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(m_VkDevice, m_InFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(m_VkDevice, m_CommandPool, nullptr);

		vkDestroyPipeline(m_VkDevice, m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_VkDevice, m_PipelineLayout, nullptr);

		for (auto framebuffer : m_SwapChainFramebuffers) 
			vkDestroyFramebuffer(m_VkDevice, framebuffer, nullptr);

		vkDestroyRenderPass(m_VkDevice, m_RenderPass, nullptr);

		for (auto imageView : m_SwapChainImageViews) 
			vkDestroyImageView(m_VkDevice, imageView, nullptr);

		vkDestroySwapchainKHR(m_VkDevice, m_SwapChain, nullptr);
		vkDestroySurfaceKHR(m_VkInstance, m_WindowSurface, nullptr);

		if (m_EnableValidationLayers)
			DestroyDebugUtilsMessengerEXT(m_VkInstance, m_DebugMessenger, nullptr);
		
		vkDestroyInstance(m_VkInstance, nullptr);
		vkDestroyDevice(m_VkDevice, nullptr);
	}

	void VKContext::Init()
	{
		MF_PROFILE_FUNCTION();

		// -- Creating a VkInstance -- //

		// -- Validation Layers

		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

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
		bool allLayersAvailable = true;
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
		

		

		// Creating the VkInstance 

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

		// ---------------------------------------- //

		// -- Setting Up Debug Messenger -- //



		if (m_EnableValidationLayers)
		{
			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
			PopulateDebugMessengerCreateInfo(debugCreateInfo);

			if (CreateDebugUtilsMessengerEXT(m_VkInstance, &debugCreateInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
				MF_CORE_ERROR("failed to set up debug messenger!");

		}

		// -------------------------------- //


		// -- Window Surface -- //

		if (glfwVulkanSupported() == GLFW_FALSE) 
			MF_CORE_DEBUG("Vulkan not Supported!!");
		
		if (glfwCreateWindowSurface(m_VkInstance, m_WindowHandle, nullptr, &m_WindowSurface) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to create a window surface!");

		// -------------------------------- //
		


		// -- Selecting the physical device(GPU) -- //

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
				break;
			}
		}

		MF_CORE_ASSERT(m_VkPhysicalDevice, "Failed to find a suitable GPU!");


		// ---------------------------------------- //



		// -- Creating logical device(s) -- //

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
																						

		/*VkPhysicalDeviceIDProperties vkPhysicalDeviceIDProperties = {};
		vkPhysicalDeviceIDProperties.sType =
			VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
		vkPhysicalDeviceIDProperties.pNext = NULL;

		VkPhysicalDeviceProperties2 vkPhysicalDeviceProperties2 = {};
		vkPhysicalDeviceProperties2.sType =
			VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		vkPhysicalDeviceProperties2.pNext = &vkPhysicalDeviceIDProperties;

		PFN_vkGetPhysicalDeviceProperties2 fpGetPhysicalDeviceProperties2;
		fpGetPhysicalDeviceProperties2 =
			(PFN_vkGetPhysicalDeviceProperties2)vkGetInstanceProcAddr(
				m_instance, "vkGetPhysicalDeviceProperties2");
		if (fpGetPhysicalDeviceProperties2 == NULL) {
			throw std::runtime_error(
				"Vulkan: Proc address for \"vkGetPhysicalDeviceProperties2KHR\" not "
				"found.\n");
		}

		fpGetPhysicalDeviceProperties2(m_physicalDevice,
			&vkPhysicalDeviceProperties2);

		memcpy(m_vkDeviceUUID, vkPhysicalDeviceIDProperties.deviceUUID, VK_UUID_SIZE);*/

		// ---------------------------------------- //


		// -- Creating the swap chain (infrastructure for the frame buffer) -- // TODO: Move to VKFrameBuffer class

		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_VkPhysicalDevice);

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.Capabilities);

		uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
		if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount) 
		{
			imageCount = swapChainSupport.Capabilities.maxImageCount;
		}

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
		swapChainCreateInfo.presentMode = presentMode;
		swapChainCreateInfo.clipped = VK_TRUE;
		swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(m_VkDevice, &swapChainCreateInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to create swap chain!");

		vkGetSwapchainImagesKHR(m_VkDevice, m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_VkDevice, m_SwapChain, &imageCount, m_SwapChainImages.data());

		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;

		// ---------------------------------------- //

		// -- Creating Image Views -- //

		m_SwapChainImageViews.resize(m_SwapChainImages.size());

		for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
			VkImageViewCreateInfo imageViewCreateInfo{};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.image = m_SwapChainImages[i];
			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // VK_IMAGE_VIEW_TYPE_CUBE -> for cube map
			imageViewCreateInfo.format = m_SwapChainImageFormat;

			imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			imageViewCreateInfo.subresourceRange.levelCount = 1;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(m_VkDevice, &imageViewCreateInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS) 
				MF_CORE_ASSERT(false, "Failed to assert image view {}", i);
		}

		// ---------------------------------------- //

		// ------- Creating Render Pass ------- //

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_SwapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		//subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		if (vkCreateRenderPass(m_VkDevice, &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create render pass!");
		

		// ------- -------------------------- ------- //

		// ------- Creating Graphics Pipeline ------- //

		// Shader Modules
		std::string shaderFilepath = "res/shaders/Basic.shader";
		ShaderList list = ParseShader(shaderFilepath);
		VkShaderModule vertShaderModule = CreateShaderModule(list.Vertex);
		VkShaderModule fragShaderModule = CreateShaderModule(list.Fragment);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

		vertShaderStageInfo.module = vertShaderModule;

		// shader entrypoint - for fragment shaders, this means it is possible
		// to include several fragment shaders in a single shader module and 
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
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

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
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; //VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		// MSAA
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		// Depth & Stencil Testing
		/*VkPipelineDepthStencilStateCreateInfo depthAndStencil{};
		depthAndStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthAndStencil.flags = 
		depthAndStencil.depthTestEnable = */

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

		// Pipeline Layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional	
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

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
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;

		pipelineInfo.layout = m_PipelineLayout;

		pipelineInfo.renderPass = m_RenderPass;
		pipelineInfo.subpass = 0;

		if (vkCreateGraphicsPipelines(m_VkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to create graphics pipeline");

		vkDestroyShaderModule(m_VkDevice, vertShaderModule, nullptr);
		vkDestroyShaderModule(m_VkDevice, fragShaderModule, nullptr);

		// ---------------------------------------- //


		// ------- Creating Framebuffers ------- //

		m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());

		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++) 
		{
			VkImageView attachments = m_SwapChainImageViews[i];

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = &attachments;
			framebufferInfo.width = m_SwapChainExtent.width;
			framebufferInfo.height = m_SwapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(m_VkDevice, &framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS) 
				MF_CORE_ASSERT(false, "failed to create framebuffer!");
		}

		// ---------------------------------------- //


		// ------- Creating Command Buffers ------- //

		m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = m_QueueFamilyIndices.GraphicsFamily.value();

		if (vkCreateCommandPool(m_VkDevice, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create command pool!");

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t) m_CommandBuffers.size();

		if (vkAllocateCommandBuffers(m_VkDevice, &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to allocate command buffers!");
		
		// ---------------------------------------- //


		// ------- Create Sync Objects ------- //

		m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(m_VkDevice, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_VkDevice, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_VkDevice, &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
			{
				MF_CORE_ASSERT(false, "failed to create semaphores!");
			}
		}
		

		// ---------------------------------------- //


		/*MF_CORE_DEBUG("Renderer Info: ");
		MF_CORE_DEBUG("\tVersion: {}", m_RendererInfo.Version);
		MF_CORE_DEBUG("\tVendor: {}", m_RendererInfo.Vendor);
		MF_CORE_DEBUG("\tRenderer: {}", m_RendererInfo.Renderer);*/

	}

	void VKContext::SwapBuffers()
	{
		//vkQueuePresentKHR(m_PresentQueue,)
	}

	void VKContext::DrawFrame()
	{
		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

		// Wait for previous frame
		vkWaitForFences(m_VkDevice, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		// Reset fences
		vkResetFences(m_VkDevice, 1, &m_InFlightFences[m_CurrentFrame]);

		// Acquire image from swap chain
		uint32_t imageIndex;
		vkAcquireNextImageKHR(m_VkDevice, m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

		// Reset and Record Command Buffer
		vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);
		RecordCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
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
		
		// Subpass Dependencies
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;

		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;

		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		// Presentation (submitting image back to swap chain)
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { m_SwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		presentInfo.pResults = nullptr; // Optional

		vkQueuePresentKHR(m_PresentQueue, &presentInfo);
	}

	void VKContext::OnImGuiRender()
	{

	}

	void VKContext::OnFinish()
	{
		vkDeviceWaitIdle(m_VkDevice);
	}

	std::vector<const char*> VKContext::GetRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (m_EnableValidationLayers)
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);


		return extensions;
	}

	void VKContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr; // Optional
	}

	bool VKContext::IsDeviceSuitable(VkPhysicalDevice device)
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
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
		}
			
		return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && 
			m_QueueFamilyIndices.IsComplete() && 
			extensionsSupported &&
			swapChainAdequate;
		
	}

	bool VKContext::CheckDeviceExtensionSupport(VkPhysicalDevice device)
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

	QueueFamilyIndices VKContext::FindQueueFamilies(VkPhysicalDevice device)
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
			if (presentSupport && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.PresentFamily = i;
				indices.GraphicsFamily = i;
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

	SwapChainSupportDetails VKContext::QuerySwapChainSupport(VkPhysicalDevice device)
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

	VkSurfaceFormatKHR VKContext::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR VKContext::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VKContext::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
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

	void VKContext::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
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

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

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

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to record command buffer!");
	}

	VkShaderModule VKContext::CreateShaderModule(const ShaderSource& source)
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

	ShaderList VKContext::ParseShader(const String& filepath)
	{
		std::ifstream stream(filepath);

		if (!stream.good())
			MF_CORE_ASSERT(false, "Failed to load shader contents");

		String line;
		std::stringstream ss[2];
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
			}
			else
			{
				ss[(int)type] << line << '\n';
			}
		}

		return {
			{ ss[0].str(), ShaderType::Vertex },
			{ ss[1].str(), ShaderType::Fragment }
		};
	}
}





//switch (deviceProperties.vendorID) {
//case 0x10DE:
//{
//	m_RendererInfo.Vendor = "NVIDIA";
//	break;
//}
//case 0x1002:
//{
//	m_RendererInfo.Vendor = "AMD";
//	m_RendererInfo.Version = "INSERT GPU VERSION HERE";
//	break;
//}
//case 0x8086:
//{
//	m_RendererInfo.Vendor = "Intel";
//	m_RendererInfo.Version = "INSERT GPU VERSION HERE";
//	break;
//}
//case 0x13B5:
//{
//	m_RendererInfo.Vendor = "ARM";
//	m_RendererInfo.Version = "INSERT GPU VERSION HERE";
//	break;
//}
//case 0x1010:
//{
//	m_RendererInfo.Vendor = "Imagination Technologies";
//	m_RendererInfo.Version = "INSERT GPU VERSION HERE";
//	break;
//}
//case 0x5143:
//{
//	m_RendererInfo.Vendor = "Qualcomm";
//	m_RendererInfo.Version = "INSERT GPU VERSION HERE";
//	break;
//}
//default:
//{
//	m_RendererInfo.Vendor = "UNKNOWN";
//	m_RendererInfo.Version = "UNKNOWN";
//}
//}
//
//
//
//uint32_t driverVersion = deviceProperties.driverVersion;
//std::string major = std::to_string(VK_VERSION_MAJOR(driverVersion));
//std::string minor = std::to_string(VK_VERSION_MINOR(driverVersion));
//std::string patch = std::to_string(VK_VERSION_PATCH(driverVersion));
//
//m_RendererInfo.Version = major + "." + minor + "." + patch;
//m_RendererInfo.Renderer = deviceProperties.deviceName;