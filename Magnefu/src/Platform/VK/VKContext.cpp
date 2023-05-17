#include "mfpch.h"

#include "VKContext.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace Magnefu
{
	

	VKContext::VKContext(GLFWwindow* windowHandle) :
		m_WindowHandle(windowHandle), m_VkInstance(VkInstance()), m_VkPhysicalDevice(VK_NULL_HANDLE)
	{
		MF_CORE_ASSERT(m_WindowHandle, "Window Handle is null!");
	}

	VKContext::~VKContext()
	{
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
		const bool enableValidationLayers = true;

		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
		MF_CORE_DEBUG("Available Vulkan Extensions: ");
		for (const auto& extension : extensions)
		{
			MF_CORE_DEBUG("\t{}", extension.extensionName);
		}
#else
		const bool enableValidationLayers = false;
#endif
		// Check validation layer support
		bool allLayersAvailable = true;
		if (enableValidationLayers)
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

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		instanceCreateInfo.enabledExtensionCount = glfwExtensionCount;
		instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;
		if (enableValidationLayers && allLayersAvailable)
		{
			instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			instanceCreateInfo.enabledLayerCount = 0;
		}
		

		if (vkCreateInstance(&instanceCreateInfo, nullptr, &m_VkInstance) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to create Vulkan VkInstance");

		// ---------------------------------------- //


		// -- Setting Up Debug Messenger -- //
		
		
		// -------------------------------- //


		// -- Window Surface -- //

		if (glfwVulkanSupported() == GLFW_FALSE) 
		{
			MF_CORE_DEBUG("Vulkan not Supported!!");
		}
		
		glfwCreateWindowSurface(m_VkInstance, m_WindowHandle, nullptr, &m_WindowSurface);

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
		QueueFamilyIndices indices = FindQueueFamilies(m_VkPhysicalDevice);

		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.GraphicsFamily.value();
		queueCreateInfo.queueCount = 1;

		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		// Specifying device features to be used
		VkPhysicalDeviceFeatures deviceFeatures{}; //empty for now


		// Create the logical device
		VkDeviceCreateInfo logicalDevCreateInfo{};
		logicalDevCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;


		logicalDevCreateInfo.pQueueCreateInfos = &queueCreateInfo;
		logicalDevCreateInfo.queueCreateInfoCount = 1;

		logicalDevCreateInfo.pEnabledFeatures = &deviceFeatures;

		// Instantiate the logical device
		if (vkCreateDevice(m_VkPhysicalDevice, &logicalDevCreateInfo, nullptr, &m_VkDevice) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "failed to create logical device!");
		
		// Retrieving queue handles
		vkGetDeviceQueue(m_VkDevice, indices.GraphicsFamily.value(), 0, &m_GraphicsQueue); // that 0 is the queue family index

		// ---------------------------------------- //

		


	}

	void VKContext::SwapBuffers()
	{

	}

	void VKContext::OnImGuiRender()
	{

	}

	bool VKContext::IsDeviceSuitable(VkPhysicalDevice device)
	{
		// TODO: allow user to choose their graphics card from list of
		// vulkan-supported options.

		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		QueueFamilyIndices indices = FindQueueFamilies(device);

		if (
			deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			indices.GraphicsFamily.has_value()
			)
		{
			switch (deviceProperties.vendorID) {
				case 0x10DE: 
				{
					m_RendererInfo.Vendor = "NVIDIA";
					uint32_t driverVersion = deviceProperties.driverVersion;


					/*uint32_t major = VK_VERSION_MAJOR(driverVersion);
					uint32_t minor = VK_VERSION_MINOR(driverVersion);
					uint32_t patch = VK_VERSION_PATCH(driverVersion);*/

					//std::string major = std::to_string((driverVersion >> 22) & 0x3ff);
					//std::string minor = std::to_string((driverVersion >> 17) & 0x1f);
					//std::string patch = std::to_string(driverVersion & 0x1ffff);
					//
					//std::string strDriverVersion = major + "." + minor + "." + patch;
					//
					//m_RendererInfo.Version = strDriverVersion;
					break;
				}
				case 0x1002: 
				{
					m_RendererInfo.Version = "AMD";
					break;
				}
				case 0x8086: 
				{
					m_RendererInfo.Version = "Intel";
					break;
				}
				case 0x13B5: 
				{
					m_RendererInfo.Version = "ARM";
					break;
				}
				case 0x1010: 
				{
					m_RendererInfo.Version = "Imagination Technologies";
					break;
				}
				case 0x5143: 
				{
					m_RendererInfo.Version = "Qualcomm";
					break;
				}
			}
			
			m_RendererInfo.Renderer = deviceProperties.deviceName;

			return true;
		}

		return false;
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
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
				indices.GraphicsFamily = i;

			if (indices.IsComplete())
				break;

			i++;
		}

		return indices;
	}
}