#include "mfpch.h"
#include <iterator>

#include "VulkanContext.h"
#include "VulkanTexture.h"
#include "VulkanBindGroup.h"
#include "VulkanShader.h"
#include "Magnefu/Application.h"
#include "Magnefu/Core/Maths/Quaternion.h"
#include "Magnefu/Renderer/RenderConstants.h"
#include "Magnefu/Renderer/LTCMatrix.h"
#include "Magnefu/ResourceManagement/ResourceManager.h"
#include "Magnefu/Scene/Scene.h"
#include "Magnefu/Scene/Entity.h"
#include "Magnefu/Scene/Components.h"



#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//#include "shaderc/shaderc.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader/tiny_obj_loader.h"

//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image/stb_image.h"

#include "SOIL2/SOIL2.h"

#include <set>
#include <unordered_map>
#include <thread>



namespace Magnefu
{
	static void* const CUSTOM_CPU_ALLOCATION_CALLBACK_USER_DATA = (void*)(intptr_t)43564544;
	static const bool USE_CUSTOM_CPU_ALLOCATION_CALLBACKS = true;

	static std::atomic_uint32_t g_CpuAllocCount;

	static const VkAllocationCallbacks* s_Allocs;

	static void* CustomCpuAllocation(
		void* pUserData, size_t size, size_t alignment,
		VkSystemAllocationScope allocationScope)
	{
		assert(pUserData == CUSTOM_CPU_ALLOCATION_CALLBACK_USER_DATA);
		void* const result = _aligned_malloc(size, alignment);
		if (result)
		{
			++g_CpuAllocCount;
		}
		return result;
	}

	static void* CustomCpuReallocation(
		void* pUserData, void* pOriginal, size_t size, size_t alignment,
		VkSystemAllocationScope allocationScope)
	{
		assert(pUserData == CUSTOM_CPU_ALLOCATION_CALLBACK_USER_DATA);
		void* const result = _aligned_realloc(pOriginal, size, alignment);
		if (pOriginal && !result)
		{
			--g_CpuAllocCount;
		}
		else if (!pOriginal && result)
		{
			++g_CpuAllocCount;
		}
		return result;
	}

	static void CustomCpuFree(void* pUserData, void* pMemory)
	{
		assert(pUserData == CUSTOM_CPU_ALLOCATION_CALLBACK_USER_DATA);
		if (pMemory)
		{
			const uint32_t oldAllocCount = g_CpuAllocCount.fetch_sub(1);
			MF_CORE_ASSERT(oldAllocCount > 0, "old allocation not greater than 0");
			_aligned_free(pMemory);
		}
	}

	static const VkAllocationCallbacks g_CpuAllocationCallbacks = {
		CUSTOM_CPU_ALLOCATION_CALLBACK_USER_DATA, // pUserData
		&CustomCpuAllocation, // pfnAllocation
		&CustomCpuReallocation, // pfnReallocation
		&CustomCpuFree // pfnFree
	};

	

	bool VK_KHR_get_memory_requirements2_enabled        = false;
	bool VK_KHR_get_physical_device_properties2_enabled = false;
	bool VK_KHR_dedicated_allocation_enabled            = false;
	bool VK_KHR_bind_memory2_enabled                    = false;
	bool VK_AMD_device_coherent_memory_enabled          = false;
	bool VK_KHR_buffer_device_address_enabled           = false;
	bool VK_EXT_debug_utils_enabled                     = false;

	bool VK_EXT_memory_budget_enabled   = true;
	bool VK_EXT_memory_priority_enabled = true;



	static const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
	};

	static const std::vector<const char*> deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
		VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME
		//VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME
	};

	bool operator==(const VulkanVertex& a, const VulkanVertex& b)
	{
		return a.pos == b.pos && a.color == b.color && a.texCoord == b.texCoord;
	}

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

		MF_CORE_DEBUG("Size of LTC1: {0}  | Size of LTC2 : {1}", sizeof(LTC1_Matrix), sizeof(LTC2_Matrix));
	}

	VulkanContext::~VulkanContext()
	{
		CleanupSwapChain();

		// -- COMPUTE SHADER CLEANUP -- //

		/*for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(m_VkDevice, m_ComputeUniformBuffers[i], s_Allocs);
			vkFreeMemory(m_VkDevice, m_ComputeUniformBuffersMemory[i], s_Allocs);
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(m_VkDevice, m_ShaderStorageBuffers[i], s_Allocs);
			vkFreeMemory(m_VkDevice, m_ShaderStorageBuffersMemory[i], s_Allocs);
		}

		vkDestroyDescriptorPool(m_VkDevice, m_ComputeDescriptorPool, s_Allocs);
		vkDestroyDescriptorSetLayout(m_VkDevice, m_ComputeDescriptorSetLayout, s_Allocs);*/

		// ----------------------------- //

		vkDestroyRenderPass(m_VkDevice, m_RenderPass, s_Allocs);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_VkDevice, m_ImageAvailableSemaphores[i], s_Allocs);
			vkDestroySemaphore(m_VkDevice, m_ImGuiRenderFinishedSemaphores[i], s_Allocs);
			vkDestroySemaphore(m_VkDevice, m_RenderFinishedSemaphores[i], s_Allocs);
			vkDestroyFence(m_VkDevice, m_InFlightFences[i], s_Allocs);
			vkDestroyFence(m_VkDevice, m_ImGuiInFlightFences[i], s_Allocs);
		}

		vkDestroyCommandPool(m_VkDevice, m_CommandPool, s_Allocs);


		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			vmaDestroyBuffer(m_VmaAllocator, m_VulkanMemory.UniformBuffers[i], m_VulkanMemory.UniformAllocations[i]);

		vmaDestroyBuffer(m_VmaAllocator, m_VulkanMemory.VBuffer, m_VulkanMemory.VBufferAllocation);
		vmaDestroyBuffer(m_VmaAllocator, m_VulkanMemory.IBuffer, m_VulkanMemory.IBufferAllocation);

		vmaDestroyAllocator(m_VmaAllocator);
		
		vkDestroyDevice(m_VkDevice, s_Allocs);

		if (m_EnableValidationLayers)
			DestroyDebugUtilsMessengerEXT(m_VkInstance, m_DebugMessenger, s_Allocs);

		vkDestroySurfaceKHR(m_VkInstance, m_WindowSurface, s_Allocs);
		
		vkDestroyInstance(m_VkInstance, s_Allocs);
		
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
		CreateSwapChain();
		CreateCommandPool(); 
		LoadModels();
		LoadTextures();
		CreateRenderPass();

		// The Application should tell Context how much memory is needed � la m_GraphicsContext->AllocateResourceMemory(BytesStruct)
		// For now, I will explicitly state within the function what is needed.
		CreateVmaAllocator();

		std::thread& bufferThread = Application::Get().GetBufferThread();
		bufferThread = std::thread(&VulkanContext::AllocateBufferMemory, this);

	}

	void VulkanContext::TempSecondaryInit()
	{
		// -- FrameBuffer Class -- //
		CreateImageViews();
		CreateColorResources();
		CreateDepthResources();
		CreateFrameBuffers();
		// -- FrameBuffer Class -- //

		CreateCommandBuffers();
		CreateSyncObjects();

	}

	void VulkanContext::DrawFrame()
	{
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

	void VulkanContext::CalculateMemoryStats()
	{
		//vmaCalculateStatistics(m_VmaAllocator, &m_VulkanMemory.TotalStats);
		vmaGetHeapBudgets(m_VmaAllocator, &m_VulkanMemory.Budgets);
	}

	MemoryStats VulkanContext::GetMemoryStats()
	{
		CalculateMemoryStats();

		auto& vulkanStats = m_VulkanMemory.Budgets.statistics;
		auto& vulkanBudget = m_VulkanMemory.Budgets;
		MemoryStats stats;

		stats.blockCount = vulkanStats.blockCount;
		stats.blockBytes = (uint64_t)vulkanStats.blockBytes;
		stats.allocationCount = vulkanStats.allocationCount;
		stats.allocationBytes = (uint64_t)vulkanStats.allocationBytes;
		stats.usage = vulkanBudget.usage;
		stats.budget = vulkanBudget.budget;

		return stats;
	}

	void VulkanContext::CreateVkInstance()
	{
		bool allLayersAvailable = true;
		SetupValidationLayers(allLayersAvailable);

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Magnefu Editor - Vulkan";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Magnefu Engine - Vulkan";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = m_APIVersion;

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


		if (vkCreateInstance(&instanceCreateInfo, s_Allocs, &m_VkInstance) != VK_SUCCESS)
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

			if (CreateDebugUtilsMessengerEXT(m_VkInstance, &debugCreateInfo, s_Allocs, &m_DebugMessenger) != VK_SUCCESS)
				MF_CORE_ERROR("failed to set up debug messenger!");

		}
	}

	void VulkanContext::CreateWindowSurface()
	{
		if (glfwVulkanSupported() == GLFW_FALSE)
			MF_CORE_DEBUG("GLFW - Vulkan not Supported!!");

		if (glfwCreateWindowSurface(m_VkInstance, m_WindowHandle, s_Allocs, &m_WindowSurface) != VK_SUCCESS)
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
		deviceFeatures.sparseBinding = VK_FALSE;

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

		m_EnabledFeatures = deviceFeatures;

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
		if (vkCreateDevice(m_VkPhysicalDevice, &logicalDevCreateInfo, s_Allocs, &m_VkDevice) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create logical device!");

		// Retrieving queue handles
		vkGetDeviceQueue(m_VkDevice, m_QueueFamilyIndices.GraphicsFamily.value(), 0, &m_GraphicsQueue); // that 0 is the queue family index
		vkGetDeviceQueue(m_VkDevice, m_QueueFamilyIndices.PresentFamily.value(), 0, &m_PresentQueue);
		vkGetDeviceQueue(m_VkDevice, m_QueueFamilyIndices.ComputeFamily.value(), 0, &m_ComputeQueue);

	}

	void VulkanContext::CreateVmaAllocator()
	{
		if (USE_CUSTOM_CPU_ALLOCATION_CALLBACKS)
		{
			s_Allocs = &g_CpuAllocationCallbacks;
		}

		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.device = m_VkDevice;
		allocatorInfo.physicalDevice = m_VkPhysicalDevice;
		allocatorInfo.instance = m_VkInstance;
		allocatorInfo.vulkanApiVersion = m_APIVersion;


		if (VK_KHR_dedicated_allocation_enabled)
		{
			allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
		}
		if (VK_KHR_bind_memory2_enabled)
		{
			allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;
		}
#if !defined(VMA_MEMORY_BUDGET) || VMA_MEMORY_BUDGET == 1
		if (VK_EXT_memory_budget_enabled && (
			m_APIVersion >= VK_API_VERSION_1_1 || VK_KHR_get_physical_device_properties2_enabled))
		{
			allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
		}
#endif
		if (VK_AMD_device_coherent_memory_enabled)
		{
			allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT;
		}
		if (VK_KHR_buffer_device_address_enabled)
		{
			allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		}
#if !defined(VMA_MEMORY_PRIORITY) || VMA_MEMORY_PRIORITY == 1
		if (VK_EXT_memory_priority_enabled)
		{
			allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
		}
#endif

		if (USE_CUSTOM_CPU_ALLOCATION_CALLBACKS)
		{
			allocatorInfo.pAllocationCallbacks = &g_CpuAllocationCallbacks;
		}

#if VMA_DYNAMIC_VULKAN_FUNCTIONS
		static VmaVulkanFunctions vulkanFunctions = {};
		vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
		allocatorInfo.pVulkanFunctions = &vulkanFunctions;
#endif*/

		// Uncomment to enable recording to CSV file.
		/*
		static VmaRecordSettings recordSettings = {};
		recordSettings.pFilePath = "VulkanSample.csv";
		outInfo.pRecordSettings = &recordSettings;
		*/

		// Uncomment to enable HeapSizeLimit.
		/*
		static std::array<VkDeviceSize, VK_MAX_MEMORY_HEAPS> heapSizeLimit;
		std::fill(heapSizeLimit.begin(), heapSizeLimit.end(), VK_WHOLE_SIZE);
		heapSizeLimit[0] = 512ull * 1024 * 1024;
		outInfo.pHeapSizeLimit = heapSizeLimit.data();
		*/
		if (vmaCreateAllocator(&allocatorInfo, &m_VmaAllocator) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to create the VMA Allocator.");

	}

	// Create Main Buffers that will be suballocated
	void VulkanContext::AllocateBufferMemory()
	{
		Application& app = Application::Get();
		auto sceneObjs = app.GetSceneObjects();
		uint32_t sceneObjCount = sceneObjs.size();

		VkCommandPool commandPool;

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		poolInfo.queueFamilyIndex = m_QueueFamilyIndices.GraphicsFamily.value();

		if (vkCreateCommandPool(m_VkDevice, &poolInfo, s_Allocs, &commandPool) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create command pool!");


		AllocateUniformBuffers(sceneObjCount);

		// TODO: Can get pointers for mesh data in Mesh components here
		AllocateVertexBuffers(sceneObjCount, sceneObjs, commandPool);
		AllocateIndexBuffers(sceneObjCount, sceneObjs, commandPool);
	}

	void VulkanContext::AllocateIndexBuffers(const uint32_t& sceneObjCount, std::vector<Magnefu::SceneObject>& sceneObjs, VkCommandPool commandPool)
	{
		
		VkDeviceSize totalSize = 0;
		VkDeviceSize size = 0;
		VkDeviceSize offset = 0;

		m_VulkanMemory.IBufferOffsets.resize(sceneObjCount);

		for (size_t i = 0; i < sceneObjCount; i++)
		{
			size = sceneObjs[i].GetIndicesSize();
			offset = (totalSize + ALIGNMENT_INDEX_BUFFER - 1) & ~(ALIGNMENT_INDEX_BUFFER - 1);
			totalSize = offset + size;
			m_VulkanMemory.IBufferOffsets[i] = offset;
		}

		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;
		VmaAllocationInfo stagingAllocInfo;

		VulkanCommon::CreateBuffer(
			totalSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			stagingBuffer,
			VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
			stagingAllocation,
			stagingAllocInfo
		);

		void* data;
		vmaMapMemory(m_VmaAllocator, stagingAllocation, &data);

		for (size_t i = 0; i < sceneObjCount; i++)
		{
			memcpy(static_cast<char*>(data) + m_VulkanMemory.IBufferOffsets[i], sceneObjs[i].GetIndicesData(), sceneObjs[i].GetIndicesSize());
			sceneObjs[i].ClearIndexDataBlock();
		}
			

		vmaUnmapMemory(m_VmaAllocator, stagingAllocation);

		VulkanCommon::CreateBuffer(
			totalSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			m_VulkanMemory.IBuffer,
			VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
			0,
			m_VulkanMemory.IBufferAllocation,
			m_VulkanMemory.IBufferAllocInfo
		);

		VulkanCommon::CopyBuffer(stagingBuffer, m_VulkanMemory.IBuffer, totalSize, commandPool);

		vmaDestroyBuffer(m_VmaAllocator, stagingBuffer, stagingAllocation);
	}

	void VulkanContext::AllocateVertexBuffers(const uint32_t& sceneObjCount, std::vector<Magnefu::SceneObject>& sceneObjs, VkCommandPool commandPool)
	{

		VkDeviceSize totalSize = 0;
		VkDeviceSize size = 0;
		VkDeviceSize offset = 0;

		m_VulkanMemory.VBufferOffsets.resize(sceneObjCount);

		for (size_t i = 0; i < sceneObjCount; i++)
		{
			size = sceneObjs[i].GetVerticesSize();
			offset = (totalSize + ALIGNMENT_VERTEX_BUFFER - 1) & ~(ALIGNMENT_VERTEX_BUFFER - 1);
			totalSize = offset + size;
			m_VulkanMemory.VBufferOffsets[i] = offset;
		}

		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;
		VmaAllocationInfo stagingAllocInfo;

		VulkanCommon::CreateBuffer(
			totalSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			stagingBuffer,
			VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
			stagingAllocation,
			stagingAllocInfo
		);

		void* data;
		vmaMapMemory(m_VmaAllocator, stagingAllocation, &data);

		for (size_t i = 0; i < sceneObjCount; i++)
		{
			memcpy(static_cast<char*>(data) + m_VulkanMemory.VBufferOffsets[i], sceneObjs[i].GetVerticesData(), sceneObjs[i].GetVerticesSize());
			sceneObjs[i].ClearVertexDataBlock();
		}
			

		vmaUnmapMemory(m_VmaAllocator, stagingAllocation);

		VulkanCommon::CreateBuffer(
			totalSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			m_VulkanMemory.VBuffer,
			VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
			0,
			m_VulkanMemory.VBufferAllocation,
			m_VulkanMemory.VBufferAllocInfo
		);

		VulkanCommon::CopyBuffer(stagingBuffer, m_VulkanMemory.VBuffer, totalSize, commandPool);

		vmaDestroyBuffer(m_VmaAllocator, stagingBuffer, stagingAllocation);
	}

	void VulkanContext::AllocateUniformBuffers(const uint32_t& sceneObjCount)
	{
		VkDeviceSize totalSize = 0;
		VkDeviceSize size = sizeof(RenderPassUniformBufferObject);
		VkDeviceSize offset = (totalSize + m_VulkanMemory.UniformAlignment - 1) & ~(m_VulkanMemory.UniformAlignment - 1);

		totalSize = offset + size;

		
		for (int index = 0; index < sceneObjCount; index++)
		{
			ResourceInfo resourceInfo = RESOURCE_PATHS[index];

			// If the sceneobject is instanced, the size of its uniform buffer
			// will be different
			if (resourceInfo.IsInstanced)
			{
				size = sizeof(MaterialUniformBufferObjectInstanced);
			}
			else
			{
				size = sizeof(MaterialUniformBufferObject);
			}
			
			offset = (totalSize + m_VulkanMemory.UniformAlignment - 1) & ~(m_VulkanMemory.UniformAlignment - 1);
			totalSize = offset + size;
		}

		m_VulkanMemory.UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		m_VulkanMemory.UniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
		m_VulkanMemory.UniformAllocations.resize(MAX_FRAMES_IN_FLIGHT);
		m_VulkanMemory.UniformAllocInfo.resize(MAX_FRAMES_IN_FLIGHT);
		m_VulkanMemory.UniformOffset = 0;


		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VulkanCommon::CreateBuffer(
				totalSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				m_VulkanMemory.UniformBuffers[i],
				VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
				VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
				m_VulkanMemory.UniformAllocations[i],
				m_VulkanMemory.UniformAllocInfo[i]
			);

			vmaMapMemory(m_VmaAllocator, m_VulkanMemory.UniformAllocations[i], &m_VulkanMemory.UniformBuffersMapped[i]);
		}
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

	}

	void VulkanContext::CreateImageViews()
	{
		m_SwapChainImageViews.resize(m_SwapChainImages.size());

		for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
			m_SwapChainImageViews[i] = VulkanCommon::CreateImageView(m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
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

		if (vkCreateRenderPass(m_VkDevice, &renderPassInfo, s_Allocs, &m_RenderPass) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create render pass!");
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

		if (vkCreateCommandPool(m_VkDevice, &poolInfo, s_Allocs, &m_CommandPool) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create command pool!");
	}

	void VulkanContext::CreateColorResources()
	{
		VkFormat colorFormat = m_SwapChainImageFormat;

		VulkanCommon::CreateImage(
			m_SwapChainExtent.width,
			m_SwapChainExtent.height,
			1,
			m_MSAASamples,
			colorFormat,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			m_VulkanMemory.ColorImage,
			VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
			0,
			m_VulkanMemory.ColorResAllocation,
			m_VulkanMemory.ColorResAllocInfo
		);

		m_ColorImageView = VulkanCommon::CreateImageView(m_VulkanMemory.ColorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	void VulkanContext::CreateDepthResources()
	{
		VkFormat depthFormat = FindDepthFormat();

		VulkanCommon::CreateImage(
			m_SwapChainExtent.width,
			m_SwapChainExtent.height,
			1,
			m_MSAASamples,
			depthFormat,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			m_VulkanMemory.DepthImage,
			VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
			0,
			m_VulkanMemory.DepthResAllocation,
			m_VulkanMemory.DepthResAllocInfo
		);

		m_DepthImageView = VulkanCommon::CreateImageView(m_VulkanMemory.DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
	}

	void VulkanContext::LoadModels()
	{
		Application::Get().ResizeSceneObjects(RESOURCE_PATHS.size());

		std::vector<std::thread> threads;
		threads.resize(RESOURCE_PATHS.size());  // TODO: I need to do some testing to programmatically determine max number of threads

		for (size_t i = 0; i < RESOURCE_PATHS.size(); i++)
			threads[i] = std::thread(&VulkanContext::LoadSingleModel, this, RESOURCE_PATHS[i], i, ModelType::MODEL_DEFAULT);

		// Join all threads
		for (auto& t : threads)
			t.join();
		
	}

	void VulkanContext::LoadTextures()
	{
		MF_CORE_ASSERT(
			Application::Get().GetSceneObjects().size() == TEXTURE_PATHS.size(),
			"Texture path count does not match number of scene objects"
		); // Will only be relevant as long as each object has a different set of textures.

		std::vector<std::thread> threads;
		threads.resize(TEXTURE_PATHS.size() * 3); // TODO: Hard coding this is REEEAALLY dangerous. Update ASAP!!

		for (int i = 0; i < TEXTURE_PATHS.size(); i++)  //
		{
			//auto& sceneObj = sceneObjs[i];
			for (int j = 0; j < 3; j++)
			{
				threads[i * 3 + j] = std::thread(&VulkanContext::LoadSingleTexture, this, TEXTURE_PATHS[i].ModelIndex, TEXTURE_PATHS[i].Paths[j], j);
			}
			
		}

		// Join all threads
		for (auto& t : threads)
			t.join();
	}

	void VulkanContext::LoadSingleModel(const ResourceInfo& resourceInfo, size_t objIndex, ModelType modelType)
	{
		Application& app = Application::Get();
		

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, resourceInfo.ModelPath))
		{
			MF_CORE_WARN(warn);
			MF_CORE_ERROR(err);
		}

		MF_CORE_DEBUG(
			"Loaded model: Vertices {0} | Normals {1} | TexCoords {2} | Shapes {3}", 
			attrib.vertices.size(), 
			attrib.normals.size(), 
			attrib.texcoords.size(), 
			shapes.size()
		);

		std::unordered_map<VulkanVertex, uint32_t> uniqueVertices{};

		std::vector<VulkanVertex>    vertices;
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

				VulkanVertex vertex{};

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

		

		{
			size_t bufferSize = vertices.size() * sizeof(VulkanVertex); // Buffer size
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
				DataBlock vertexBlock(reinterpret_cast<const uint8_t*>(alignedPtr), bufferSize);
				app.SetVertexBlock(std::move(vertexBlock), objIndex, modelType);
				
				
			}
			else
			{
				MF_CORE_ASSERT(false, "Failed to properly align data.");
			}
		}

		DataBlock indexBlock(reinterpret_cast<const uint8_t*>(indices.data()), indices.size() * sizeof(uint32_t));
		app.SetIndexBlock(std::move(indexBlock), objIndex, modelType);

		
		
	}


	void VulkanContext::LoadSingleTexture(int sceneObjIndex, const char* texturePath, int textureType)
	{
		int width, height, channels;

		unsigned char* pixels = SOIL_load_image(
			texturePath, 
			&width, &height, &channels, 
			TextureChannels::CHANNELS_RGB_ALPHA
		);

		if (!pixels)
			MF_CORE_ASSERT(false, "failed to load texture image!");

		MF_CORE_DEBUG("Width: {0} | Height: {1} | Channels: {2}", width, height, channels);

		DataBlock textureBlock(reinterpret_cast<const uint8_t*>(pixels), width * height * TextureChannels::CHANNELS_RGB_ALPHA);
		Application::Get().GetSceneObjects()[sceneObjIndex].SetTextureBlock(static_cast<TextureType>(textureType), std::move(textureBlock), width, height, channels);
		SOIL_free_image_data(pixels);
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
			m_VulkanMemory.UniformAlignment = m_Properties.limits.minUniformBufferOffsetAlignment;

			MF_CORE_DEBUG("Max Push Constant Size: {}", m_Properties.limits.maxPushConstantsSize);
			MF_CORE_DEBUG("Min Uniform Buffer Offset Alignment: {}", m_Properties.limits.minUniformBufferOffsetAlignment);
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

		// add necessary device extensions here:
		if (m_APIVersion == VK_API_VERSION_1_0)
		{
			requiredExtensions.insert(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
			requiredExtensions.insert(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
			requiredExtensions.insert(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
		}

		if (m_APIVersion < VK_API_VERSION_1_2)
			requiredExtensions.insert(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
			



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

	void VulkanContext::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		Application& app = Application::Get();
		ResourceManager& rm = Application::Get().GetResourceManager();

		VulkanBindGroup& renderpassGlobals = static_cast<VulkanBindGroup&>(rm.GetBindGroup(app.GetRenderPassBindGroup()));
		VulkanUniformBuffer& renderpassUniformBuffer = static_cast<VulkanUniformBuffer&>(rm.GetBuffer(renderpassGlobals.GetUniformsHandle()));


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

		// TODO: Order draw calls by shader to limit pipeline switching

		auto& sceneObjects = app.GetSceneObjects();		

		for (auto& sceneObject : sceneObjects)
		{
			VulkanShader& shader = static_cast<VulkanShader&>(rm.GetShader(sceneObject.GetGraphicsPipelineShaderHandle()));
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.GetPipeline());

			VulkanBindGroup& material = static_cast<VulkanBindGroup&>(rm.GetBindGroup(sceneObject.GetMaterialBindGroup()));

			std::array<VkDescriptorSet, 2> descriptorSets = {
				renderpassGlobals.GetFrameDescriptorSet(m_CurrentFrame),
				material.GetFrameDescriptorSet(m_CurrentFrame)
			};
			
			VulkanBuffer& vertexBuffer = static_cast<VulkanBuffer&>(rm.GetBuffer(sceneObject.GetVertexBufferHandle()));
			VulkanBuffer& indexBuffer = static_cast<VulkanBuffer&>(rm.GetBuffer(sceneObject.GetIndexBufferHandle()));

			VkBuffer vertexBuffers[] = { vertexBuffer.GetBuffer() };
			VkDeviceSize offsets[] = { vertexBuffer.GetOffset() };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(commandBuffer, indexBuffer.GetBuffer(), indexBuffer.GetOffset(), VK_INDEX_TYPE_UINT32);


			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.GetPipelineLayout(), 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

			vkCmdPushConstants(commandBuffer, shader.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &m_PushConstants);

			if (sceneObject.IsInstanced())
			{
					vkCmdDrawIndexed(commandBuffer, sceneObject.GetIndexCount(), sceneObject.GetInstanceCount(), 0, 0, 0);
			}
			else
			{
				vkCmdDrawIndexed(commandBuffer, sceneObject.GetIndexCount(), 1, 0, 0, 0);
			}
			
			
		}
			

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to record command buffer!");

		// -- Update Uniform Buffers -- //

		renderpassUniformBuffer.UpdateUniformBuffer(Material{});

		for (auto& sceneObject : sceneObjects)
		{
			VulkanBindGroup& material = static_cast<VulkanBindGroup&>(rm.GetBindGroup(sceneObject.GetMaterialBindGroup()));
			VulkanUniformBuffer& materialUniformBuffer = static_cast<VulkanUniformBuffer&>(rm.GetBuffer(material.GetUniformsHandle()));

			if (sceneObject.IsInstanced())
			{
				materialUniformBuffer.UpdateUniformBuffer(sceneObject.GetMaterialDataInstanced(), sceneObject.GetInstanceCount());
			}
			else
			{
				materialUniformBuffer.UpdateUniformBuffer(sceneObject.GetMaterialData());
			}
		}
		
	}

	void VulkanContext::CreateSyncObjects()
	{


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

			if (vkCreateSemaphore(m_VkDevice, &semaphoreInfo, s_Allocs, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_VkDevice, &semaphoreInfo, s_Allocs, &m_ImGuiRenderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_VkDevice, &semaphoreInfo, s_Allocs, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_VkDevice, &fenceInfo, s_Allocs, &m_InFlightFences[i]) != VK_SUCCESS ||
				vkCreateFence(m_VkDevice, &fenceInfo, s_Allocs, &m_ImGuiInFlightFences[i]) != VK_SUCCESS)
			{
				MF_CORE_ASSERT(false, "failed to create GRAPHICS semaphores and/or fences!");
			}
		}

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
		vmaDestroyImage(m_VmaAllocator, m_VulkanMemory.ColorImage, m_VulkanMemory.ColorResAllocation);

		vkDestroyImageView(m_VkDevice, m_DepthImageView, nullptr);
		vmaDestroyImage(m_VmaAllocator, m_VulkanMemory.DepthImage, m_VulkanMemory.DepthResAllocation);

		for (auto framebuffer : m_SwapChainFramebuffers)
			vkDestroyFramebuffer(m_VkDevice, framebuffer, nullptr);

		for (auto imageView : m_SwapChainImageViews)
			vkDestroyImageView(m_VkDevice, imageView, nullptr);

		vkDestroySwapchainKHR(m_VkDevice, m_SwapChain, nullptr);
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



	void VulkanContext::PerformGraphicsOps()
	{
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
		RecordCommandBuffer(m_CommandBuffers[m_CurrentFrame], m_ImageIndex);


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
}