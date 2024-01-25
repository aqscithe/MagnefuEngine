// -- PCH -- //
#include "mfpch.h"

// -- HEADER -- //
#include "GraphicsContext.h"

// -- Graphics Includes ----------------------- //
#include "CommandBuffer.hpp"
#include "spirv_parser.hpp"
// -- Aplication Includes ------------------------- //

// -- Core Includes ----------------------------- //
#include "Magnefu/Core/HashMap.hpp"
#include "Magnefu/Core/File.hpp"
#include "Magnefu/Core/Process.hpp"


// -- Other Includes ----------------------- //
#include <set>


template<class T>
constexpr const T& magnefu_min(const T& a, const T& b) 
{
    return (a < b) ? a : b;
}

template<class T>
constexpr const T& magnefu_max(const T& a, const T& b) 
{
    return (a < b) ? b : a;
}

#define VMA_MAX magnefu_max
#define VMA_MIN magnefu_min
#define VMA_USE_STL_CONTAINERS 0
#define VMA_USE_STL_VECTOR 0
#define VMA_USE_STL_UNORDERED_MAP 0
#define VMA_USE_STL_LIST 0

#if defined (_MSC_VER)
#pragma warning (disable: 4127)
#pragma warning (disable: 4189)
#pragma warning (disable: 4191)
#pragma warning (disable: 4296)
#pragma warning (disable: 4324)
#pragma warning (disable: 4355)
#pragma warning (disable: 4365)
#pragma warning (disable: 4625)
#pragma warning (disable: 4626)
#pragma warning (disable: 4668)
#pragma warning (disable: 5026)
#pragma warning (disable: 5027)
#endif // _MSC_VER


// -- Vendor Includes ---------------------- //
#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace Magnefu
{


    static void                 check_result(VkResult result);
#define                     check( result, message ) MF_CORE_ASSERT( result == VK_SUCCESS, "Vulkan assert code {} | {}", result, message )

   

    // -- Device implementation --------------------------------------------------------------- //

    // -- Methods ---------------------------------------------------------------------------- //

    // Enable this to add debugging capabilities.
    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_EXT_debug_utils.html

#define VULKAN_DEBUG_REPORT
#define VULKAN_MEMORY_BUDGETS
#define VULKAN_MEMORY_PRIORITY

//#define VULKAN_SYNCHRONIZATION_VALIDATION

    // Vulkan INSTANCE Extensions
    static const char* s_requested_extensions[] = 
    {
        VK_KHR_SURFACE_EXTENSION_NAME,      // Also obtained through glfwGetRequiredInstanceExtensions
        // Platform specific extension
    #ifdef VK_USE_PLATFORM_WIN32_KHR
            "VK_KHR_win32_surface",         // Also obtained through glfwGetRequiredInstanceExtensions
            //VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    #elif defined(VK_USE_PLATFORM_MACOS_MVK)
            VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
    #elif defined(VK_USE_PLATFORM_XCB_KHR)
            VK_KHR_XCB_SURFACE_EXTENSION_NAME,
    #elif defined(VK_USE_PLATFORM_ANDROID_KHR)
            VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
    #elif defined(VK_USE_PLATFORM_XLIB_KHR)
            VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
    #elif defined(VK_USE_PLATFORM_XCB_KHR)
            VK_KHR_XCB_SURFACE_EXTENSION_NAME,
    #elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
            VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
    #elif defined(VK_USE_PLATFORM_MIR_KHR || VK_USE_PLATFORM_DISPLAY_KHR)
            VK_KHR_DISPLAY_EXTENSION_NAME,
    #elif defined(VK_USE_PLATFORM_ANDROID_KHR)
            VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
    #elif defined(VK_USE_PLATFORM_IOS_MVK)
            VK_MVK_IOS_SURFACE_EXTENSION_NAME,
    #endif // VK_USE_PLATFORM_WIN32_KHR

    #if defined (VULKAN_DEBUG_REPORT)
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    #endif // VULKAN_DEBUG_REPORT
    };

    // Vulkan LOGICAL DEVICE Extensions
    static const char* device_extensions[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,

#if defined(VULKAN_MEMORY_BUDGETS)
        VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
#endif

#if defined(VULKAN_MEMORY_PRIORITY)
        VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME
#endif
        //VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME
    };

    static const char* s_requested_layers[] = {
    #if defined (VULKAN_DEBUG_REPORT)
        "VK_LAYER_KHRONOS_validation",
        //"VK_LAYER_LUNARG_core_validation",
        //"VK_LAYER_LUNARG_image",
        //"VK_LAYER_LUNARG_parameter_validation",
        //"VK_LAYER_LUNARG_object_tracker"
    #else
        "",
    #endif // VULKAN_DEBUG_REPORT
    };

#ifdef VULKAN_DEBUG_REPORT

    static VkBool32 debug_utils_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT types,
        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
        void* user_data) 
    {
        MF_CORE_WARN("VULKAN VALIDATION LAYER - Message ID: {} {} | Message: {}", callback_data->pMessageIdName, callback_data->messageIdNumber, callback_data->pMessage);

        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            // __debugbreak();
        }

        return VK_FALSE;
    }


    // TODO:
    // -- GPU Timestamps ----------------------------------------------------------b//

    VkDebugUtilsMessengerCreateInfoEXT create_debug_utils_messenger_info() 
    {
        VkDebugUtilsMessengerCreateInfoEXT creation_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
        creation_info.pfnUserCallback = debug_utils_callback;
        creation_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        creation_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        creation_info.pUserData = nullptr; // Optional

        return creation_info;
    }

#endif // VULKAN_DEBUG_REPORT

    static GLFWwindow* glfw_window;

    static GraphicsContext s_gpu_context;

    GraphicsContext* GraphicsContext::Instance() { return &s_gpu_context; }

    void* GraphicsContext::get_window_handle() { return (void*)glfw_window; }

    PFN_vkSetDebugUtilsObjectNameEXT    pfnSetDebugUtilsObjectNameEXT;
    PFN_vkCmdBeginDebugUtilsLabelEXT    pfnCmdBeginDebugUtilsLabelEXT;
    PFN_vkCmdEndDebugUtilsLabelEXT      pfnCmdEndDebugUtilsLabelEXT;

    static Magnefu::FlatHashMap<u64, VkRenderPass> render_pass_cache;
    static CommandBufferManager command_buffer_ring;

    static sizet            s_ubo_alignment = 256;
    static sizet            s_ssbo_alignemnt = 256;

    static const u32        k_bindless_texture_binding = 10;
    static const u32        k_max_bindless_resources = 1024;

    bool GraphicsContext::get_family_queue(VkPhysicalDevice physical_device) 
    {
        u32 queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

        VkQueueFamilyProperties* queue_families = (VkQueueFamilyProperties*)mfalloca(sizeof(VkQueueFamilyProperties) * queue_family_count, allocator);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);

        u32 family_index = 0;
        VkBool32 surface_supported;
        for (; family_index < queue_family_count; ++family_index) 
        {
            VkQueueFamilyProperties queue_family = queue_families[family_index];

            if (queue_family.queueCount > 0 && queue_family.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) 
            {
                vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, family_index, vulkan_window_surface, &surface_supported);

                if (surface_supported) 
                {
                    vulkan_main_queue_family = family_index;
                    break;
                }
            }
        }

        mffree(queue_families, allocator);

        return surface_supported;
    }

    bool GraphicsContext::check_device_extension_support(VkPhysicalDevice physical_device)
    {
        // TODO: Check device extension support
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, nullptr);

        Array<VkExtensionProperties> availableExtensions(allocator, extensionCount, extensionCount);
        
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, availableExtensions.begin());


        Array<cstring> deviceExtensions(allocator, device_extensions, device_extensions + ArraySize(device_extensions));
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        

        uint32_t apiVersion;
        if (vkEnumerateInstanceVersion(&apiVersion) == VK_SUCCESS) 
        {

            if (apiVersion == VK_API_VERSION_1_0)
            {
                requiredExtensions.insert(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
                requiredExtensions.insert(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
                requiredExtensions.insert(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
            }

            if (apiVersion < VK_API_VERSION_1_2)
            {
                requiredExtensions.insert(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
            }
              
        }
        else
        {
            MF_CORE_ERROR("Failed to enumerate the api instance version.");
        }

        MF_CORE_DEBUG("AVAILABLE EXTENSIONS: ");
        for (const auto& extension : availableExtensions) 
        {     
            requiredExtensions.erase(extension.extensionName);
            MF_CORE_DEBUG("\t{}", extension.extensionName);
        }

        return requiredExtensions.empty();

    }

    SwapChainSupportDetails GraphicsContext::query_swapchain_support(VkPhysicalDevice physical_device)
    {
        SwapChainSupportDetails details;

        // Formats
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vulkan_window_surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.Formats.set_size(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vulkan_window_surface, &formatCount, details.Formats.begin());
        }

        // Presentation Modes
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, vulkan_window_surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.PresentModes.set_size(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, vulkan_window_surface, &presentModeCount, details.PresentModes.begin());
        }

        // Capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, vulkan_window_surface, &details.Capabilities);

        return details;
    }

    void GraphicsContext::init(const DeviceCreation& creation) 
    {

        MF_CORE_INFO("Gpu Device init");


        allocator = creation.allocator;
        temporary_allocator = creation.temporary_allocator;
        string_buffer.init(1024 * 1024, creation.allocator);


        // -- Init Vulkan Instance -------------------------------------------------- //

        VkResult result;
        vulkan_allocation_callbacks = nullptr;

        VkApplicationInfo application_info {};
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pApplicationName = "Magnefu Editor - Vulkan";
        application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        application_info.pEngineName = "Magnefu Engine - Vulkan";
        application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        application_info.apiVersion = VK_API_VERSION_1_3;


        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext = nullptr;
        instanceCreateInfo.flags = 0;
        instanceCreateInfo.pApplicationInfo = &application_info;
#if defined(VULKAN_DEBUG_REPORT)
        instanceCreateInfo.enabledLayerCount = (u32)ArraySize(s_requested_layers);
        instanceCreateInfo.ppEnabledLayerNames = s_requested_layers;

        
#else
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.ppEnabledLayerNames = nullptr;
#endif
        instanceCreateInfo.enabledExtensionCount = (u32)ArraySize(s_requested_extensions);
        instanceCreateInfo.ppEnabledExtensionNames = s_requested_extensions;


#if defined(VULKAN_DEBUG_REPORT)
        const VkDebugUtilsMessengerCreateInfoEXT debug_create_info = create_debug_utils_messenger_info();

    #if defined(VULKAN_SYNCHRONIZATION_VALIDATION)
            const VkValidationFeatureEnableEXT featuresRequested[] = { VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT, VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT/*, VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT*/ };
            VkValidationFeaturesEXT features = {};
            features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
            features.pNext = &debug_create_info;
            features.enabledValidationFeatureCount = _countof(featuresRequested);
            features.pEnabledValidationFeatures = featuresRequested;
            instanceCreateInfo.pNext = &features;
    #else
            instanceCreateInfo.pNext = &debug_create_info;
    #endif // VULKAN_SYNCHRONIZATION_VALIDATION
#endif // VULKAN_DEBUG_REPORT


        // -- Create Vulkan Instance ------------------------------------------------------------------- //
        check(vkCreateInstance(&instanceCreateInfo, vulkan_allocation_callbacks, &vulkan_instance), "Failed to create Vulkan VkInstance");

        swapchain_width = creation.width;
        swapchain_height = creation.height;

        

        

        // -- Choose Extensions --------------------------------------------------------------------------- //
#ifdef VULKAN_DEBUG_REPORT
        {

            u32 num_instance_extensions;
            vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_extensions, nullptr);
            VkExtensionProperties* extensions = (VkExtensionProperties*)mfalloca(sizeof(VkExtensionProperties) * num_instance_extensions, allocator);
            vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_extensions, extensions);
            for (size_t i = 0; i < num_instance_extensions; i++) 
            {

                if (!strcmp(extensions[i].extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) 
                {
                    debug_utils_extension_present = true;
                    break;
                }
            }

            mffree(extensions, allocator);

            if (!debug_utils_extension_present) 
            {
                MF_CORE_INFO("Extension {} for debugging non present.", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
            else 
            {
                //// Create debug callback
                //auto vkCreateDebugReportCallbackEXT = ( PFN_vkCreateDebugReportCallbackEXT )vkGetInstanceProcAddr( vulkan_instance, "vkCreateDebugReportCallbackEXT" );
                //MF_CORE_ASSERT( vkCreateDebugReportCallbackEXT != NULL, "" );

                //// Setup the debug report callback
                /*VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
                debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
                debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
                debug_report_ci.pfnCallback = debug_callback;
                debug_report_ci.pUserData = NULL;
                result = vkCreateDebugReportCallbackEXT( vulkan_instance, &debug_report_ci, vulkan_allocation_callbacks, &vulkan_debug_callback );
                check( result );*/

                // Create new debug utils callback
                PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkan_instance, "vkCreateDebugUtilsMessengerEXT");

                if (vkCreateDebugUtilsMessengerEXT != nullptr)
                {
                    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info = create_debug_utils_messenger_info();

                    vkCreateDebugUtilsMessengerEXT(vulkan_instance, &debug_messenger_create_info, vulkan_allocation_callbacks, &vulkan_debug_utils_messenger);
                }
                else
                {
                    MF_CORE_DEBUG("vkCreateDebugUtilsMessengerEXT - VK_ERROR_EXTENSION_NOT_PRESENT");
                }
                
            }
        }
#endif

        //////// Choose physical device
        u32 num_physical_device;
        result = vkEnumeratePhysicalDevices(vulkan_instance, &num_physical_device, NULL);
        check(result, "No physical devices found");

        VkPhysicalDevice* gpus = (VkPhysicalDevice*)mfalloca(sizeof(VkPhysicalDevice) * num_physical_device, allocator);
        result = vkEnumeratePhysicalDevices(vulkan_instance, &num_physical_device, gpus);
        check(result, "");

        //////// Create drawable surface
        // Create surface
        GLFWwindow* window = (GLFWwindow*)creation.window;
        if (glfwVulkanSupported() == GLFW_FALSE)
            MF_CORE_DEBUG("GLFW - Vulkan not Supported!!");

        check(glfwCreateWindowSurface(vulkan_instance, window, vulkan_allocation_callbacks, &vulkan_window_surface), "Failed to create a window surface!")

        glfw_window = window;

        VkPhysicalDevice discrete_gpu = VK_NULL_HANDLE;
        VkPhysicalDevice integrated_gpu = VK_NULL_HANDLE;
        for (u32 i = 0; i < num_physical_device; ++i) 
        {
            VkPhysicalDevice physical_device = gpus[i];

            vkGetPhysicalDeviceProperties(physical_device, &vulkan_physical_properties);

            bool swapchainAdequate = false;
            bool extensionsSupported = check_device_extension_support(physical_device); // checking REQUIRED extensions

            if (extensionsSupported)
            {
                SwapChainSupportDetails swapChainSupport = query_swapchain_support(physical_device);
                swapchainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
            }

            if (vulkan_physical_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) 
            {
                if (get_family_queue(physical_device) && extensionsSupported && swapchainAdequate) 
                {
                    // NOTE(marco): prefer discrete GPU over integrated one, stop at first discrete GPU that has
                    // present capabilities
                    discrete_gpu = physical_device;
                    break;
                }

                continue;
            }

            if (vulkan_physical_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) 
            {
                if (get_family_queue(physical_device))
                {
                    integrated_gpu = physical_device;
                }

                continue;
            }
        }

        if (discrete_gpu != VK_NULL_HANDLE) 
        {
            vulkan_physical_device = discrete_gpu;
        }
        else if (integrated_gpu != VK_NULL_HANDLE) 
        {
            vulkan_physical_device = integrated_gpu;
        }
        else 
        {
            MF_CORE_ASSERT(false, "Suitable GPU device not found!");
            return;
        }

        mffree(gpus, allocator);

        vkGetPhysicalDeviceProperties(vulkan_physical_device, &vulkan_physical_properties);
        gpu_timestamp_frequency = vulkan_physical_properties.limits.timestampPeriod / (1000 * 1000);

        MF_CORE_INFO("GPU Used: {}", vulkan_physical_properties.deviceName);

        s_ubo_alignment = vulkan_physical_properties.limits.minUniformBufferOffsetAlignment;
        s_ssbo_alignemnt = vulkan_physical_properties.limits.minStorageBufferOffsetAlignment;
        

        //vulkan_physical_features.features.samplerAnisotropy
        //


        // Query bindless extension, called Descriptor Indexing (https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_EXT_descriptor_indexing.html)
        VkPhysicalDeviceDescriptorIndexingFeatures indexing_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES, nullptr };
        vulkan_physical_features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &indexing_features };

        vkGetPhysicalDeviceFeatures2(vulkan_physical_device, &vulkan_physical_features);
        // For the feature to be correctly working, we need both the possibility to partially bind a descriptor,
        // as some entries in the bindless array will be empty, and SpirV runtime descriptors.
        bindless_supported = indexing_features.descriptorBindingPartiallyBound && indexing_features.runtimeDescriptorArray;
        // TODO: remove when finished with bindless
        //bindless_supported = false;


        // -- Create logical device -------------------------------------------- //
        u32 queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(vulkan_physical_device, &queue_family_count, nullptr);
        VkQueueFamilyProperties* queue_families = (VkQueueFamilyProperties*)mfalloca(sizeof(VkQueueFamilyProperties) * queue_family_count, allocator);
        vkGetPhysicalDeviceQueueFamilyProperties(vulkan_physical_device, &queue_family_count, queue_families);

        u32 main_queue_index = u32_max;
        u32 transfer_queue_index = u32_max;
        u32 compute_queue_index = u32_max;
        u32 present_queue_index = u32_max;

        for (u32 fi = 0; fi < queue_family_count; fi++)
        {
            VkQueueFamilyProperties& queue_family = queue_families[fi];

            if (queue_family.queueCount == 0)
                continue;

            MF_CORE_DEBUG("Family {} | Flags {} | Queue Count {}", fi, queue_family.queueFlags, queue_family.queueCount);

            // Search for main queue that should be able to do all work (graphics, compute, transfer)
            if ((queue_family.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT )) == (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT ))
            {
                main_queue_index = fi;
            }

            // Search for dedicated transfer queue
            if ((queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0 && (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT))
            {
                transfer_queue_index = fi;
            }


        }

        mffree(queue_families, allocator);

        // Cache family indices
        vulkan_main_queue_family = main_queue_index;
        vulkan_transfer_queue_family = transfer_queue_index;

        const float queue_priority[] = { 1.0f };
        VkDeviceQueueCreateInfo queue_info[2] = {};
        VkDeviceQueueCreateInfo& main_queue = queue_info[0];

        main_queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        main_queue.queueFamilyIndex = vulkan_main_queue_family;
        main_queue.queueCount = 1;
        main_queue.pQueuePriorities = queue_priority;

        if (vulkan_transfer_queue_family < queue_family_count)
        {
            VkDeviceQueueCreateInfo& transfer_queue = queue_info[1];
            transfer_queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            transfer_queue.queueFamilyIndex = vulkan_transfer_queue_family;
            transfer_queue.queueCount = 1;
            transfer_queue.pQueuePriorities = queue_priority;
        }

        // Enable all features: just pass the physical features 2 struct.

        VkDeviceCreateInfo device_create_info = {};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = vulkan_transfer_queue_family < queue_family_count ? 2 : 1;
        device_create_info.pQueueCreateInfos = queue_info;
        //device_create_info.pEnabledFeatures = &vulkan_physical_features.features;
        device_create_info.enabledExtensionCount = (u32)ArraySize(device_extensions);
        device_create_info.ppEnabledExtensionNames = device_extensions;
        device_create_info.enabledLayerCount = ArraySize(s_requested_layers);
        device_create_info.ppEnabledLayerNames = s_requested_layers;
        device_create_info.pNext = &vulkan_physical_features;

        if (bindless_supported) 
        {
            indexing_features.descriptorBindingPartiallyBound = VK_TRUE;
            indexing_features.runtimeDescriptorArray = VK_TRUE;

            vulkan_physical_features.pNext = &indexing_features;
        }


        result = vkCreateDevice(vulkan_physical_device, &device_create_info, vulkan_allocation_callbacks, &vulkan_device);
        check(result, "Failed to create vulkan logical device");

        //  Get the function pointers to Debug Utils functions.
        if (debug_utils_extension_present) 
        {
            pfnSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(vulkan_device, "vkSetDebugUtilsObjectNameEXT");
            pfnCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(vulkan_device, "vkCmdBeginDebugUtilsLabelEXT");
            pfnCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(vulkan_device, "vkCmdEndDebugUtilsLabelEXT");
        }


        // Get main queue
        vkGetDeviceQueue(vulkan_device, main_queue_index, 0, &vulkan_main_queue);
        if (vulkan_transfer_queue_family < queue_family_count)
        {
            vkGetDeviceQueue(vulkan_device, transfer_queue_index, 0, &vulkan_transfer_queue);
        }

        // Create Framebuffers
        int window_width, window_height;
        glfwGetWindowSize(window, &window_width, &window_height);

        VkSampleCountFlags vulkan_max_sample_count = vulkan_physical_properties.limits.framebufferColorSampleCounts & vulkan_physical_properties.limits.framebufferDepthSampleCounts;

#if 1
        vulkan_max_sample_count_bits = VK_SAMPLE_COUNT_1_BIT;
#else
        if (vulkan_max_sample_count & VK_SAMPLE_COUNT_64_BIT) { vulkan_max_sample_count_bits = VK_SAMPLE_COUNT_64_BIT; }
        else if (vulkan_max_sample_count & VK_SAMPLE_COUNT_32_BIT) { vulkan_max_sample_count_bits = VK_SAMPLE_COUNT_32_BIT; }
        else if (vulkan_max_sample_count & VK_SAMPLE_COUNT_16_BIT) { vulkan_max_sample_count_bits = VK_SAMPLE_COUNT_16_BIT; }
        else if (vulkan_max_sample_count & VK_SAMPLE_COUNT_8_BIT)  { vulkan_max_sample_count_bits = VK_SAMPLE_COUNT_8_BIT; }
        else if (vulkan_max_sample_count & VK_SAMPLE_COUNT_4_BIT)  { vulkan_max_sample_count_bits = VK_SAMPLE_COUNT_4_BIT; }
        else if (vulkan_max_sample_count & VK_SAMPLE_COUNT_2_BIT)  { vulkan_max_sample_count_bits = VK_SAMPLE_COUNT_2_BIT; }
        else { vulkan_max_sample_count_bits = VK_SAMPLE_COUNT_1_BIT; }

#endif
        //// Select Surface Format
        //const TextureFormat::Enum swapchain_formats[] = { TextureFormat::B8G8R8A8_UNORM, TextureFormat::R8G8B8A8_UNORM, TextureFormat::B8G8R8X8_UNORM, TextureFormat::B8G8R8X8_UNORM };
        const VkFormat surface_image_formats[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
        const VkColorSpaceKHR surface_color_space = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

        u32 supported_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_physical_device, vulkan_window_surface, &supported_count, NULL);
        VkSurfaceFormatKHR* supported_formats = (VkSurfaceFormatKHR*)mfalloca(sizeof(VkSurfaceFormatKHR) * supported_count, allocator);
        vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_physical_device, vulkan_window_surface, &supported_count, supported_formats);

        // Cache render pass output
        swapchain_output.reset();

        //// Check for supported formats
        bool format_found = false;
        const u32 surface_format_count = (u32)ArraySize(surface_image_formats);

        for (int i = 0; i < surface_format_count; i++) 
        {
            for (u32 j = 0; j < supported_count; j++) 
            {
                if (supported_formats[j].format == surface_image_formats[i] && supported_formats[j].colorSpace == surface_color_space) 
                {
                    vulkan_surface_format = supported_formats[j];
                    format_found = true;
                    break;
                }
            }

            if (format_found)
                break;
        }

        // Default to the first format supported.
        if (!format_found) 
        {
            vulkan_surface_format = supported_formats[0];
            MF_CORE_ASSERT(false, "Surface Format not found");
        }
        mffree(supported_formats, allocator);

        swapchain_output.color(vulkan_surface_format.format);

        set_present_mode(present_mode);

        //////// Create swapchain
        create_swapchain();

        //////// Create VMA Allocator
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = vulkan_physical_device;
        allocatorInfo.device = vulkan_device;
        allocatorInfo.instance = vulkan_instance;

        //allocatorInfo.flags = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT | VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT | VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

#if defined(VULKAN_MEMORY_BUDGETS)
        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
#endif

#if defined(VULKAN_MEMORY_PRIORITY)
        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
#endif


        result = vmaCreateAllocator(&allocatorInfo, &vma_allocator);
        check(result, "Failed to create VMA Allocator");

        // --  Create pools ----------------------------------------------- //
        static const u32 k_global_pool_elements = 128;
        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, k_global_pool_elements },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, k_global_pool_elements },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, k_global_pool_elements },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, k_global_pool_elements },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, k_global_pool_elements },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, k_global_pool_elements },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, k_global_pool_elements },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, k_global_pool_elements },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, k_global_pool_elements },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, k_global_pool_elements },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, k_global_pool_elements}
        };

        u32 pool_size_count = (u32)ArraySize(pool_sizes);
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = k_global_pool_elements * pool_size_count;
        pool_info.poolSizeCount = pool_size_count;
        pool_info.pPoolSizes = pool_sizes;
        result = vkCreateDescriptorPool(vulkan_device, &pool_info, vulkan_allocation_callbacks, &vulkan_descriptor_pool);
        check(result, "Fialed to create descriptor pool");

        // Bindless pool
        VkDescriptorPoolSize pool_sizes_bindless[] =
        {
            {
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                k_max_bindless_resources
            },
            {
                VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                k_max_bindless_resources
            }
        };

        const u32 pool_sizes_bindless_count = (u32)ArraySize(pool_sizes_bindless);
        VkDescriptorPoolCreateInfo pool_info_bindless = {};
        pool_info_bindless.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info_bindless.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
        pool_info_bindless.maxSets = k_max_bindless_resources * pool_sizes_bindless_count;
        pool_info_bindless.poolSizeCount = pool_sizes_bindless_count;
        pool_info_bindless.pPoolSizes = pool_sizes_bindless;
        result = vkCreateDescriptorPool(vulkan_device, &pool_info_bindless, vulkan_allocation_callbacks, &vulkan_bindless_descriptor_pool);
        check(result, "Failed to create descriptor pool");


        VkDescriptorSetLayoutBinding vk_binding[4];
        for (u32 i = 0; i < (u32)ArraySize(vk_binding); i++)
        {
            vk_binding[i] = {};
        }

        VkDescriptorSetLayoutBinding& image_sampler_binding = vk_binding [ 0 ];
        image_sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        image_sampler_binding.descriptorCount = k_max_bindless_resources;
        image_sampler_binding.binding = k_bindless_texture_binding;
        image_sampler_binding.pImmutableSamplers = nullptr;
        image_sampler_binding.stageFlags = VK_SHADER_STAGE_ALL;

        VkDescriptorSetLayoutBinding& storage_binding = vk_binding [ 1 ];
        storage_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        storage_binding.descriptorCount = k_max_bindless_resources;
        storage_binding.binding = k_bindless_texture_binding + 1;
        storage_binding.pImmutableSamplers = nullptr;
        storage_binding.stageFlags = 0;

        VkDescriptorSetLayoutCreateInfo layout_info{};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = pool_sizes_bindless_count;
        layout_info.pBindings = vk_binding;
        layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

        VkDescriptorBindingFlags bindless_flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | /*VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |*/ VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

        VkDescriptorBindingFlags binding_flags[4];
        binding_flags[0] = bindless_flags;
        binding_flags[1] = bindless_flags;

        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info{};
        extended_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
        extended_info.bindingCount = pool_sizes_bindless_count;
        extended_info.pBindingFlags = binding_flags;
        extended_info.pNext = nullptr;

        layout_info.pNext = &extended_info;

        result = vkCreateDescriptorSetLayout(vulkan_device, &layout_info, vulkan_allocation_callbacks, &vulkan_bindless_descriptor_set_layout);
        check(result, "Failed to created bindless descriptor set");


        VkDescriptorSetAllocateInfo alloc_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        alloc_info.descriptorPool = vulkan_bindless_descriptor_pool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &vulkan_bindless_descriptor_set_layout;

        VkDescriptorSetVariableDescriptorCountAllocateInfoEXT count_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT };
        u32 max_binding = k_max_bindless_resources - 1;
        count_info.descriptorSetCount = 1;
        // This number is the max allocatable count
        count_info.pDescriptorCounts = &max_binding;
        //alloc_info.pNext = &count_info;

        result = vkAllocateDescriptorSets(vulkan_device, &alloc_info, &vulkan_bindless_descriptor_set);
        check(result, "Failed to allocate bindless descriptor set.");


        // Create timestamp query pool used for GPU timings.
        VkQueryPoolCreateInfo vqpci{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO, nullptr, 0, VK_QUERY_TYPE_TIMESTAMP, creation.gpu_time_queries_per_frame * 2u * k_max_frames, 0 };
        vkCreateQueryPool(vulkan_device, &vqpci, vulkan_allocation_callbacks, &vulkan_timestamp_query_pool);

        //// Init pools
        buffers.init(allocator, k_buffers_pool_size, sizeof(Buffer));
        textures.init(allocator, k_textures_pool_size, sizeof(Texture));
        render_passes.init(allocator, k_render_passes_pool_size, sizeof(RenderPass));
        descriptor_set_layouts.init(allocator, k_descriptor_set_layouts_pool_size, sizeof(DescriptorSetLayout));
        pipelines.init(allocator, k_pipelines_pool_size, sizeof(Pipeline));
        shaders.init(allocator, k_shaders_pool_size, sizeof(ShaderState));
        descriptor_sets.init(allocator, k_descriptor_sets_pool_size, sizeof(DescriptorSet));
        samplers.init(allocator, k_samplers_pool_size, sizeof(Sampler));
        //command_buffers.init( allocator, 128, sizeof( CommandBuffer ) );

        // Init render frame informations. This includes fences, semaphores, command buffers, ...
        // TODO: memory - allocate memory of all Device render frame stuff
        u8* memory = mfallocam(sizeof(GPUTimestampManager) + sizeof(CommandBuffer*) * 128, allocator);

        VkSemaphoreCreateInfo semaphore_info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        vkCreateSemaphore(vulkan_device, &semaphore_info, vulkan_allocation_callbacks, &vulkan_image_acquired_semaphore);

        for (size_t i = 0; i < k_max_swapchain_images; i++) 
        {

            vkCreateSemaphore(vulkan_device, &semaphore_info, vulkan_allocation_callbacks, &vulkan_render_complete_semaphore[i]);

            VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            vkCreateFence(vulkan_device, &fenceInfo, vulkan_allocation_callbacks, &vulkan_command_buffer_executed_fence[i]);
        }

        gpu_timestamp_manager = (GPUTimestampManager*)(memory);
        gpu_timestamp_manager->init(allocator, creation.gpu_time_queries_per_frame, k_max_frames);

        command_buffer_ring.init(this, creation.num_threads);

        // Allocate queued command buffers array
        queued_command_buffers = (CommandBuffer**)(gpu_timestamp_manager + 1);
        CommandBuffer** correctly_allocated_buffer = (CommandBuffer**)(memory + sizeof(GPUTimestampManager));
        MF_CORE_ASSERT((queued_command_buffers == correctly_allocated_buffer), "Wrong calculations for queued command buffers arrays.");

        vulkan_image_index = 0;
        current_frame = 1;
        previous_frame = 0;
        absolute_frame = 0;
        timestamps_enabled = false;

        resource_deletion_queue.init(allocator, 16);
        descriptor_set_updates.init(allocator, 16);
        texture_to_update_bindless.init(allocator, 16);

        //
        // Init primitive resources
        //
        SamplerCreation sc{};
        sc.set_address_mode_uvw(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .set_min_mag_mip(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR).set_name("Sampler Default");
        default_sampler = create_sampler(sc);

        BufferCreation fullscreen_vb_creation = { VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, ResourceUsageType::Immutable, 0, 0, 0, nullptr, "Fullscreen_vb" };
        fullscreen_vertex_buffer = create_buffer(fullscreen_vb_creation);

        // Create depth image
        TextureCreation depth_texture_creation = { nullptr, swapchain_width, swapchain_height, 1, 1, 0, VK_FORMAT_D32_SFLOAT, TextureType::Texture2D, "DepthImage_Texture" };
        depth_texture = create_texture(depth_texture_creation);

        // Cache depth texture format
        swapchain_output.depth(VK_FORMAT_D32_SFLOAT);

        RenderPassCreation swapchain_pass_creation = {};
        swapchain_pass_creation.set_type(RenderPassType::Swapchain).set_name("Swapchain");
        swapchain_pass_creation.set_operations(RenderPassOperation::Clear, RenderPassOperation::Clear, RenderPassOperation::Clear);
        swapchain_pass = create_render_pass(swapchain_pass_creation);

        // Init Dummy resources
        TextureCreation dummy_texture_creation = { nullptr, 1, 1, 1, 1, 0, VK_FORMAT_R8_UINT, TextureType::Texture2D };
        dummy_texture = create_texture(dummy_texture_creation);

        BufferCreation dummy_constant_buffer_creation = { VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, ResourceUsageType::Immutable, 16, 0, 0, nullptr, "Dummy_cb" };
        dummy_constant_buffer = create_buffer(dummy_constant_buffer_creation);

        // Get binaries path
#if defined(_MSC_VER)
        char* vulkan_env = string_buffer.reserve(512);
        ExpandEnvironmentStringsA("%VULKAN_SDK%", vulkan_env, 512);
        char* compiler_path = string_buffer.append_use_f("%s\\Bin\\", vulkan_env);
#else
        char* vulkan_env = getenv("VULKAN_SDK");
        char* compiler_path = string_buffer.append_use_f("%s/bin/", vulkan_env);
#endif

        strcpy(vulkan_binaries_path, compiler_path);
        string_buffer.clear();

        // Dynamic buffer handling
        // TODO:
        dynamic_per_frame_size = 1024 * 1024 * 10;
        BufferCreation bc;
        bc.set(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, ResourceUsageType::Immutable, dynamic_per_frame_size * k_max_frames).set_name("Dynamic_Persistent_Buffer");
        dynamic_buffer = create_buffer(bc);

        MapBufferParameters cb_map = { dynamic_buffer, 0, 0 };
        dynamic_mapped_memory = (u8*)map_buffer(cb_map);

        // Init render pass cache
        render_pass_cache.init(allocator, 16);

        
        
    }

    void GraphicsContext::shutdown() {

        vkDeviceWaitIdle(vulkan_device);

        command_buffer_ring.shutdown();

        for (size_t i = 0; i < k_max_swapchain_images; i++) {
            vkDestroySemaphore(vulkan_device, vulkan_render_complete_semaphore[i], vulkan_allocation_callbacks);
            vkDestroyFence(vulkan_device, vulkan_command_buffer_executed_fence[i], vulkan_allocation_callbacks);
        }

        vkDestroySemaphore(vulkan_device, vulkan_image_acquired_semaphore, vulkan_allocation_callbacks);

        gpu_timestamp_manager->shutdown();

        MapBufferParameters cb_map = { dynamic_buffer, 0, 0 };
        unmap_buffer(cb_map);

        // Memory: this contains allocations for gpu timestamp memory, queued command buffers and render frames.
        mffree(gpu_timestamp_manager, allocator);

        // Add resources to deletion queue
        destroy_texture(depth_texture);
        destroy_buffer(fullscreen_vertex_buffer);
        destroy_buffer(dynamic_buffer);
        destroy_render_pass(swapchain_pass);
        destroy_texture(dummy_texture);
        destroy_buffer(dummy_constant_buffer);
        destroy_sampler(default_sampler);

        
        // Destroy all pending resources.
        for (u32 i = 0; i < resource_deletion_queue.count(); i++) 
        {
            ResourceUpdate& resource_deletion = resource_deletion_queue[i];

            // Skip just freed resources.
            if (resource_deletion.current_frame == -1)
                continue;

            switch (resource_deletion.type) {

            case ResourceDeletionType::Buffer:
            {
                destroy_buffer_instant(resource_deletion.handle);
                break;
            }

            case ResourceDeletionType::Pipeline:
            {
                destroy_pipeline_instant(resource_deletion.handle);
                break;
            }

            case ResourceDeletionType::RenderPass:
            {
                destroy_render_pass_instant(resource_deletion.handle);
                break;
            }

            case ResourceDeletionType::DescriptorSet:
            {
                destroy_descriptor_set_instant(resource_deletion.handle);
                break;
            }

            case ResourceDeletionType::DescriptorSetLayout:
            {
                destroy_descriptor_set_layout_instant(resource_deletion.handle);
                break;
            }

            case ResourceDeletionType::Sampler:
            {
                destroy_sampler_instant(resource_deletion.handle);
                break;
            }

            case ResourceDeletionType::ShaderState:
            {
                destroy_shader_state_instant(resource_deletion.handle);
                break;
            }

            case ResourceDeletionType::Texture:
            {
                destroy_texture_instant(resource_deletion.handle);
                break;
            }
            }
        }


        // Destroy render passes from the cache.
        FlatHashMapIterator it = render_pass_cache.iterator_begin();
        while (it.is_valid()) {
            VkRenderPass vk_render_pass = render_pass_cache.get(it);
            vkDestroyRenderPass(vulkan_device, vk_render_pass, vulkan_allocation_callbacks);
            render_pass_cache.iterator_advance(it);
        }
        render_pass_cache.shutdown();

        // Destroy swapchain render pass, not present in the cache.
        RenderPass* vk_swapchain_pass = access_render_pass(swapchain_pass);
        vkDestroyRenderPass(vulkan_device, vk_swapchain_pass->vk_render_pass, vulkan_allocation_callbacks);

        // Destroy swapchain
        destroy_swapchain();
        vkDestroySurfaceKHR(vulkan_instance, vulkan_window_surface, vulkan_allocation_callbacks);

        vmaDestroyAllocator(vma_allocator);

        resource_deletion_queue.shutdown();
        descriptor_set_updates.shutdown();
        texture_to_update_bindless.shutdown();

        //command_buffers.shutdown();
        pipelines.shutdown();
        buffers.shutdown();
        shaders.shutdown();
        textures.shutdown();
        samplers.shutdown();
        descriptor_set_layouts.shutdown();
        descriptor_sets.shutdown();
        render_passes.shutdown();
#ifdef VULKAN_DEBUG_REPORT
        // Remove the debug report callback
        //auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr( vulkan_instance, "vkDestroyDebugReportCallbackEXT" );
        //if(vkDestroyDebugReportCallbackEXT)
        //    vkDestroyDebugReportCallbackEXT( vulkan_instance, vulkan_debug_callback, vulkan_allocation_callbacks );

        auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkan_instance, "vkDestroyDebugUtilsMessengerEXT");

        if(vkDestroyDebugUtilsMessengerEXT)
            vkDestroyDebugUtilsMessengerEXT(vulkan_instance, vulkan_debug_utils_messenger, vulkan_allocation_callbacks);
#endif // IMGUI_VULKAN_DEBUG_REPORT

        // [TAG: BINDLESS]
        if (bindless_supported) 
        {
            vkDestroyDescriptorSetLayout(vulkan_device, vulkan_bindless_descriptor_set_layout, vulkan_allocation_callbacks);
            vkDestroyDescriptorPool(vulkan_device, vulkan_bindless_descriptor_pool, vulkan_allocation_callbacks);
        }

        vkDestroyDescriptorPool(vulkan_device, vulkan_descriptor_pool, vulkan_allocation_callbacks);
        vkDestroyQueryPool(vulkan_device, vulkan_timestamp_query_pool, vulkan_allocation_callbacks);

        vkDestroyDevice(vulkan_device, vulkan_allocation_callbacks);

        vkDestroyInstance(vulkan_instance, vulkan_allocation_callbacks);

        string_buffer.shutdown();

        MF_CORE_INFO("Gpu Device shutdown");
    }


    

    // Resource Creation ////////////////////////////////////////////////////////////
    static void vulkan_create_texture(GraphicsContext& gpu, const TextureCreation& creation, TextureHandle handle, Texture* texture) {

        texture->width = creation.width;
        texture->height = creation.height;
        texture->depth = creation.depth;
        texture->mipmaps = creation.mipmaps;
        texture->type = creation.type;
        texture->name = creation.name;
        texture->vk_format = creation.format;
        texture->sampler = nullptr;
        texture->flags = creation.flags;

        texture->handle = handle;

        //// Create the image
        VkImageCreateInfo image_info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        image_info.format = texture->vk_format;
        image_info.flags = 0;
        image_info.imageType = to_vk_image_type(creation.type);
        image_info.extent.width = creation.width;
        image_info.extent.height = creation.height;
        image_info.extent.depth = creation.depth;
        image_info.mipLevels = creation.mipmaps;
        image_info.arrayLayers = 1;
        image_info.samples = gpu.vulkan_max_sample_count_bits;
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;

        const bool is_render_target = (creation.flags & TextureFlags::RenderTarget_mask) == TextureFlags::RenderTarget_mask;
        const bool is_compute_used = (creation.flags & TextureFlags::Compute_mask) == TextureFlags::Compute_mask;

        // Default to always readable from shader.
        image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

        image_info.usage |= is_compute_used ? VK_IMAGE_USAGE_STORAGE_BIT : 0;

        if (TextureFormat::has_depth_or_stencil(creation.format)) {
            // Depth/Stencil textures are normally textures you render into.
            image_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        }
        else {
            image_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT; // TODO
            image_info.usage |= is_render_target ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0;
        }

        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VmaAllocationCreateInfo memory_info{};
        memory_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        check(vmaCreateImage(gpu.vma_allocator, &image_info, &memory_info,
            &texture->vk_image, &texture->vma_allocation, nullptr), "failed to create image");

        gpu.set_resource_name(VK_OBJECT_TYPE_IMAGE, (u64)texture->vk_image, creation.name);

        //// Create the image view
        VkImageViewCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        info.image = texture->vk_image;
        info.viewType = to_vk_image_view_type(creation.type);
        info.format = image_info.format;

        if (TextureFormat::has_depth_or_stencil(creation.format)) {

            info.subresourceRange.aspectMask = TextureFormat::has_depth(creation.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
            // TODO:gs
            //info.subresourceRange.aspectMask |= TextureFormat::has_stencil( creation.format ) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
        }
        else {
            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        info.subresourceRange.levelCount = 1;
        info.subresourceRange.layerCount = 1;
        check(vkCreateImageView(gpu.vulkan_device, &info, gpu.vulkan_allocation_callbacks, &texture->vk_image_view), "Failed to create image view");

        gpu.set_resource_name(VK_OBJECT_TYPE_IMAGE_VIEW, (u64)texture->vk_image_view, creation.name);

        texture->vk_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;

        // Add deferred bindless update.
        if (gpu.bindless_supported) 
        {
            ResourceUpdate resource_update{ ResourceDeletionType::Texture, texture->handle.index, gpu.current_frame };
            gpu.texture_to_update_bindless.push(resource_update);
        }
    }

    static void upload_texture_data(Texture* texture, void* upload_data, GraphicsContext& gpu)
    {

        // Create stating buffer
        VkBufferCreateInfo buffer_info{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        u32 image_size = texture->width * texture->height * 4;
        buffer_info.size = image_size;

        VmaAllocationCreateInfo memory_info{};
        memory_info.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
        memory_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        VmaAllocationInfo allocation_info{};
        VkBuffer staging_buffer;
        VmaAllocation staging_allocation;
        (vmaCreateBuffer(gpu.vma_allocator, &buffer_info, &memory_info,
            &staging_buffer, &staging_allocation, &allocation_info));

        // Copy buffer_data
        void* destination_data;
        vmaMapMemory(gpu.vma_allocator, staging_allocation, &destination_data);
        memcpy(destination_data, upload_data, static_cast<size_t>(image_size));
        vmaUnmapMemory(gpu.vma_allocator, staging_allocation);

        // Execute command buffer
        VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        // TODO: threading
        CommandBuffer* command_buffer = gpu.get_command_buffer(0, false);
        vkBeginCommandBuffer(command_buffer->vk_command_buffer, &beginInfo);

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { texture->width, texture->height, texture->depth };

        // Copy from the staging buffer to the image
        util_add_image_barrier(command_buffer->vk_command_buffer, texture->vk_image, RESOURCE_STATE_UNDEFINED, RESOURCE_STATE_COPY_DEST, 0, 1, false);

        vkCmdCopyBufferToImage(command_buffer->vk_command_buffer, staging_buffer, texture->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        // Prepare first mip to create lower mipmaps
        if (texture->mipmaps > 1) {
            util_add_image_barrier(command_buffer->vk_command_buffer, texture->vk_image, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_COPY_SOURCE, 0, 1, false);
        }

        i32 w = texture->width;
        i32 h = texture->height;

        for (int mip_index = 1; mip_index < texture->mipmaps; ++mip_index) {
            util_add_image_barrier(command_buffer->vk_command_buffer, texture->vk_image, RESOURCE_STATE_UNDEFINED, RESOURCE_STATE_COPY_DEST, mip_index, 1, false);

            VkImageBlit blit_region{ };
            blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit_region.srcSubresource.mipLevel = mip_index - 1;
            blit_region.srcSubresource.baseArrayLayer = 0;
            blit_region.srcSubresource.layerCount = 1;

            blit_region.srcOffsets[0] = { 0, 0, 0 };
            blit_region.srcOffsets[1] = { w, h, 1 };

            w /= 2;
            h /= 2;

            blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit_region.dstSubresource.mipLevel = mip_index;
            blit_region.dstSubresource.baseArrayLayer = 0;
            blit_region.dstSubresource.layerCount = 1;

            blit_region.dstOffsets[0] = { 0, 0, 0 };
            blit_region.dstOffsets[1] = { w, h, 1 };

            vkCmdBlitImage(command_buffer->vk_command_buffer, texture->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit_region, VK_FILTER_LINEAR);

            // Prepare current mip for next level
            util_add_image_barrier(command_buffer->vk_command_buffer, texture->vk_image, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_COPY_SOURCE, mip_index, 1, false);
        }

        // Transition
        util_add_image_barrier(command_buffer->vk_command_buffer, texture->vk_image, (texture->mipmaps > 1) ? RESOURCE_STATE_COPY_SOURCE : RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_SHADER_RESOURCE, 0, texture->mipmaps, false);

        vkEndCommandBuffer(command_buffer->vk_command_buffer);

        // Submit command buffer
        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &command_buffer->vk_command_buffer;

        vkQueueSubmit(gpu.vulkan_main_queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(gpu.vulkan_main_queue);

        vmaDestroyBuffer(gpu.vma_allocator, staging_buffer, staging_allocation);

        // TODO: free command buffer
        vkResetCommandBuffer(command_buffer->vk_command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

        texture->vk_image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }


    TextureHandle GraphicsContext::create_texture(const TextureCreation& creation) 
    {

        u32 resource_index = textures.obtain_resource();
        TextureHandle handle = { resource_index };
        if (resource_index == k_invalid_index) {
            return handle;
        }

        Texture* texture = access_texture(handle);

        vulkan_create_texture(*this, creation, handle, texture);

        //// Copy buffer_data if present
        if (creation.initial_data) 
        {
            upload_texture_data(texture, creation.initial_data, *this);
        }

        return handle;
    }

    // helper method
    bool is_end_of_line(char c) {
        bool result = ((c == '\n') || (c == '\r'));
        return(result);
    }

    void dump_shader_code(StringBuffer& temp_string_buffer, cstring code, VkShaderStageFlagBits stage, cstring name)
    {
        MF_CORE_ERROR("Error in creation of shader {}, stage {}. Writing shader:", name, to_stage_defines(stage));

        cstring current_code = code;
        u32 line_index = 1;
        while (current_code) {

            cstring end_of_line = current_code;
            if (!end_of_line || *end_of_line == 0) {
                break;
            }
            while (!is_end_of_line(*end_of_line)) {
                ++end_of_line;
            }
            if (*end_of_line == '\r') {
                ++end_of_line;
            }
            if (*end_of_line == '\n') {
                ++end_of_line;
            }

            temp_string_buffer.clear();
            char* line = temp_string_buffer.append_use_substring(current_code, 0, (end_of_line - current_code));
            MF_CORE_INFO("{}: {}", line_index++, line);

            current_code = end_of_line;
        }
    }

    VkShaderModuleCreateInfo GraphicsContext::compile_shader(cstring code, u32 code_size, VkShaderStageFlagBits stage, cstring name) {

        VkShaderModuleCreateInfo shader_create_info = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };

        // Compile from glsl to SpirV.
        // TODO: detect if input is HLSL.
        const char* temp_filename = "temp.shader";

        // Write current shader to file.
        FILE* temp_shader_file = fopen(temp_filename, "w");
        fwrite(code, code_size, 1, temp_shader_file);
        fclose(temp_shader_file);

        sizet current_marker = temporary_allocator->getMarker();
        StringBuffer temp_string_buffer;
        temp_string_buffer.init(mfkilo(1), temporary_allocator);

        // Add uppercase define as STAGE_NAME
        char* stage_define = temp_string_buffer.append_use_f("%s_%s", to_stage_defines(stage), name);
        sizet stage_define_length = strlen(stage_define);
        for (u32 i = 0; i < stage_define_length; ++i) {
            stage_define[i] = toupper(stage_define[i]);
        }
        // Compile to SPV
#if defined(_MSC_VER)
        char* glsl_compiler_path = temp_string_buffer.append_use_f("%sglslangValidator.exe", vulkan_binaries_path);
        char* final_spirv_filename = temp_string_buffer.append_use("shader_final.spv");
        // TODO: add optional debug information in shaders (option -g).
        char* arguments = temp_string_buffer.append_use_f("glslangValidator.exe %s -V --target-env vulkan1.2 -o %s -S %s --D %s --D %s", temp_filename, final_spirv_filename, to_compiler_extension(stage), stage_define, to_stage_defines(stage));
#else
        char* glsl_compiler_path = temp_string_buffer.append_use_f("%sglslangValidator", vulkan_binaries_path);
        char* final_spirv_filename = temp_string_buffer.append_use("shader_final.spv");
        char* arguments = temp_string_buffer.append_use_f("%s -V --target-env vulkan1.2 -o %s -S %s --D %s --D %s", temp_filename, final_spirv_filename, to_compiler_extension(stage), stage_define, to_stage_defines(stage));
#endif
        process_execute(".", glsl_compiler_path, arguments, "");

        bool optimize_shaders = false;

        if (optimize_shaders) {
            // TODO: add optional optimization stage
            //"spirv-opt -O input -o output
            char* spirv_optimizer_path = temp_string_buffer.append_use_f("%sspirv-opt.exe", vulkan_binaries_path);
            char* optimized_spirv_filename = temp_string_buffer.append_use_f("shader_opt.spv");
            char* spirv_opt_arguments = temp_string_buffer.append_use_f("spirv-opt.exe -O --preserve-bindings %s -o %s", final_spirv_filename, optimized_spirv_filename);

            process_execute(".", spirv_optimizer_path, spirv_opt_arguments, "");

            // Read back SPV file.
            shader_create_info.pCode = reinterpret_cast<const u32*>(file_read_binary(optimized_spirv_filename, temporary_allocator, &shader_create_info.codeSize));

            file_delete(optimized_spirv_filename);
        }
        else {
            // Read back SPV file.
            shader_create_info.pCode = reinterpret_cast<const u32*>(file_read_binary(final_spirv_filename, temporary_allocator, &shader_create_info.codeSize));
        }

        // Handling compilation error
        if (shader_create_info.pCode == nullptr) {
            dump_shader_code(temp_string_buffer, code, stage, name);
        }

        // Temporary files cleanup
        file_delete(temp_filename);
        file_delete(final_spirv_filename);

        return shader_create_info;
    }

    ShaderStateHandle GraphicsContext::create_shader_state(const ShaderStateCreation& creation) {

        ShaderStateHandle handle = { k_invalid_index };

        if (creation.stages_count == 0 || creation.stages == nullptr) {
            MF_CORE_ERROR("Shader {} does not contain shader stages.", creation.name);
            return handle;
        }

        handle.index = shaders.obtain_resource();
        if (handle.index == k_invalid_index) {
            return handle;
        }

        // For each shader stage, compile them individually.
        u32 compiled_shaders = 0;

        ShaderState* shader_state = access_shader_state(handle);
        shader_state->graphics_pipeline = true;
        shader_state->active_shaders = 0;

        sizet current_temporary_marker = temporary_allocator->getMarker();

        StringBuffer name_buffer;
        name_buffer.init(4096, temporary_allocator);

        // Parse result needs to be always in memory as its used to free descriptor sets.
        shader_state->parse_result = (spirv::ParseResult*)allocator->allocate(sizeof(spirv::ParseResult), 64);
        memset(shader_state->parse_result, 0, sizeof(spirv::ParseResult));

        for (compiled_shaders = 0; compiled_shaders < creation.stages_count; ++compiled_shaders)
        {
            const ShaderStage& stage = creation.stages[compiled_shaders];

            // Gives priority to compute: if any is present (and it should not be) then it is not a graphics pipeline.
            if (stage.type == VK_SHADER_STAGE_COMPUTE_BIT)
            {
                shader_state->graphics_pipeline = false;
            }

            VkShaderModuleCreateInfo shader_create_info = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };

            if (creation.spv_input) 
            {
                shader_create_info.codeSize = stage.code_size;
                shader_create_info.pCode = reinterpret_cast<const u32*>(stage.code);
            }
            else 
            {
                shader_create_info = compile_shader(stage.code, stage.code_size, stage.type, creation.name);
            }

            // Compile shader module
            VkPipelineShaderStageCreateInfo& shader_stage_info = shader_state->shader_stage_info[compiled_shaders];
            memset(&shader_stage_info, 0, sizeof(VkPipelineShaderStageCreateInfo));
            shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_stage_info.pName = "main";
            shader_stage_info.stage = stage.type;

            check(vkCreateShaderModule(vulkan_device, &shader_create_info, nullptr, &shader_state->shader_stage_info[compiled_shaders].module), "Failed to create shader module");

            spirv::parse_binary(shader_create_info.pCode, shader_create_info.codeSize, name_buffer, shader_state->parse_result);

            

            set_resource_name(VK_OBJECT_TYPE_SHADER_MODULE, (u64)shader_state->shader_stage_info[compiled_shaders].module, creation.name);
        }

        
        temporary_allocator->freeToMarker(current_temporary_marker);

        bool creation_failed = compiled_shaders != creation.stages_count;
        if (!creation_failed) 
        {
            shader_state->active_shaders = compiled_shaders;
            shader_state->name = creation.name;
        }

        if (creation_failed) 
        {
            destroy_shader_state(handle);
            handle.index = k_invalid_index;

            // Dump shader code
            MF_CORE_ERROR("Error in creation of shader {}. Dumping all shader informations.", creation.name);
            for (compiled_shaders = 0; compiled_shaders < creation.stages_count; ++compiled_shaders) 
            {
                const ShaderStage& stage = creation.stages[compiled_shaders];
                MF_CORE_INFO("{}:\n{}\n", stage.type, stage.code);
            }
        }

        return handle;
    }

    PipelineHandle GraphicsContext::create_pipeline(const PipelineCreation& creation, const char* cache_path)
    {
        PipelineHandle handle = { pipelines.obtain_resource() };
        if (handle.index == k_invalid_index) 
        {
            return handle;
        }


        VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
        VkPipelineCacheCreateInfo pipeline_cache_create_info{ VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };

        bool cache_exists = file_exists(cache_path);
        if (cache_path != nullptr && cache_exists)
        {
            FileReadResult read_result = file_read_binary(cache_path, allocator);

            VkPipelineCacheHeaderVersionOne* cache_header = (VkPipelineCacheHeaderVersionOne*)read_result.data;

            if (cache_header->deviceID == vulkan_physical_properties.deviceID &&
                cache_header->vendorID == vulkan_physical_properties.vendorID &&
                memcmp(cache_header->pipelineCacheUUID, vulkan_physical_properties.pipelineCacheUUID, VK_UUID_SIZE) == 0)
            {
                pipeline_cache_create_info.initialDataSize = read_result.size;
                pipeline_cache_create_info.pInitialData = read_result.data;
            }
            else
            {
                cache_exists = false;
            }

            check(vkCreatePipelineCache(vulkan_device, &pipeline_cache_create_info, vulkan_allocation_callbacks, &pipeline_cache), "Failed to create pipeline cache");

            allocator->deallocate(read_result.data);
        }
        else
        {
            check(vkCreatePipelineCache(vulkan_device, &pipeline_cache_create_info, vulkan_allocation_callbacks, &pipeline_cache), "Failed to create pipeline cache");
        }

        ShaderStateHandle shader_state_handle = create_shader_state(creation.shaders);
        if (shader_state_handle.index == k_invalid_index)
        {
            // Shader did not compile.
            pipelines.release_resource(handle.index);
            handle.index = k_invalid_index;

            return handle;
        }

        // Now that shaders have compiled we can create the pipeline.
        Pipeline* pipeline = access_pipeline(handle);
        ShaderState* shader_state_data = access_shader_state(shader_state_handle);

        pipeline->shader_state = shader_state_handle;

        VkDescriptorSetLayout vk_layouts[k_max_descriptor_set_layouts];

        u32 num_active_layouts = shader_state_data->parse_result->set_count;

        // Create VkPipelineLayout
        for (u32 l = 0; l < shader_state_data->parse_result->set_count; ++l) 
        {
            pipeline->descriptor_set_layout_handle[l] = create_descriptor_set_layout(shader_state_data->parse_result->sets[l]);
            pipeline->descriptor_set_layout[l] = access_descriptor_set_layout(pipeline->descriptor_set_layout_handle[l]);

            vk_layouts[l] = pipeline->descriptor_set_layout[l]->vk_descriptor_set_layout;
        }

        // TODO: improve.
        // Add bindless resource layout after other layouts.
            // [TAG: BINDLESS]
        u32 bindless_active = 0;
        if (bindless_supported) 
        {
            vk_layouts[num_active_layouts] = vulkan_bindless_descriptor_set_layout;
            bindless_active = 1;
        }

        VkPipelineLayoutCreateInfo pipeline_layout_info = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        pipeline_layout_info.pSetLayouts = vk_layouts;
        pipeline_layout_info.setLayoutCount = num_active_layouts + bindless_active;

        VkPipelineLayout pipeline_layout;
        check(vkCreatePipelineLayout(vulkan_device, &pipeline_layout_info, vulkan_allocation_callbacks, &pipeline_layout), "Failed to create pipeline layout");
        // Cache pipeline layout
        pipeline->vk_pipeline_layout = pipeline_layout;
        pipeline->num_active_layouts = num_active_layouts;

        // Create full pipeline
        if (shader_state_data->graphics_pipeline) 
        {
            VkGraphicsPipelineCreateInfo pipeline_info = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

            //// Shader stage
            pipeline_info.pStages = shader_state_data->shader_stage_info;
            pipeline_info.stageCount = shader_state_data->active_shaders;
            //// PipelineLayout
            pipeline_info.layout = pipeline_layout;

            //// Vertex input
            VkPipelineVertexInputStateCreateInfo vertex_input_info = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

            // Vertex attributes.
            VkVertexInputAttributeDescription vertex_attributes[8];
            if (creation.vertex_input.num_vertex_attributes) 
            {

                for (u32 i = 0; i < creation.vertex_input.num_vertex_attributes; ++i)
                {
                    const VertexAttribute& vertex_attribute = creation.vertex_input.vertex_attributes[i];
                    vertex_attributes[i] = { vertex_attribute.location, vertex_attribute.binding, to_vk_vertex_format(vertex_attribute.format), vertex_attribute.offset };
                }

                vertex_input_info.vertexAttributeDescriptionCount = creation.vertex_input.num_vertex_attributes;
                vertex_input_info.pVertexAttributeDescriptions = vertex_attributes;
            }
            else 
            {
                vertex_input_info.vertexAttributeDescriptionCount = 0;
                vertex_input_info.pVertexAttributeDescriptions = nullptr;
            }
            // Vertex bindings
            VkVertexInputBindingDescription vertex_bindings[8];
            if (creation.vertex_input.num_vertex_streams) 
            {
                vertex_input_info.vertexBindingDescriptionCount = creation.vertex_input.num_vertex_streams;

                for (u32 i = 0; i < creation.vertex_input.num_vertex_streams; ++i)
                {
                    const VertexStream& vertex_stream = creation.vertex_input.vertex_streams[i];
                    VkVertexInputRate vertex_rate = vertex_stream.input_rate == VertexInputRate::PerVertex ? VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX : VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE;
                    vertex_bindings[i] = { vertex_stream.binding, vertex_stream.stride, vertex_rate };
                }
                vertex_input_info.pVertexBindingDescriptions = vertex_bindings;
            }
            else {
                vertex_input_info.vertexBindingDescriptionCount = 0;
                vertex_input_info.pVertexBindingDescriptions = nullptr;
            }

            pipeline_info.pVertexInputState = &vertex_input_info;

            //// Input Assembly
            VkPipelineInputAssemblyStateCreateInfo input_assembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
            input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            input_assembly.primitiveRestartEnable = VK_FALSE;

            pipeline_info.pInputAssemblyState = &input_assembly;

            //// Color Blending
            VkPipelineColorBlendAttachmentState color_blend_attachment[8];

            if (creation.blend_state.active_states) {
                for (size_t i = 0; i < creation.blend_state.active_states; i++) {
                    const BlendState& blend_state = creation.blend_state.blend_states[i];

                    color_blend_attachment[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                    color_blend_attachment[i].blendEnable = blend_state.blend_enabled ? VK_TRUE : VK_FALSE;
                    color_blend_attachment[i].srcColorBlendFactor = blend_state.source_color;
                    color_blend_attachment[i].dstColorBlendFactor = blend_state.destination_color;
                    color_blend_attachment[i].colorBlendOp = blend_state.color_operation;

                    if (blend_state.separate_blend) {
                        color_blend_attachment[i].srcAlphaBlendFactor = blend_state.source_alpha;
                        color_blend_attachment[i].dstAlphaBlendFactor = blend_state.destination_alpha;
                        color_blend_attachment[i].alphaBlendOp = blend_state.alpha_operation;
                    }
                    else {
                        color_blend_attachment[i].srcAlphaBlendFactor = blend_state.source_color;
                        color_blend_attachment[i].dstAlphaBlendFactor = blend_state.destination_color;
                        color_blend_attachment[i].alphaBlendOp = blend_state.color_operation;
                    }
                }
            }
            else {
                // Default non blended state
                color_blend_attachment[0] = {};
                color_blend_attachment[0].blendEnable = VK_FALSE;
                color_blend_attachment[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            }

            VkPipelineColorBlendStateCreateInfo color_blending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
            color_blending.logicOpEnable = VK_FALSE;
            color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
            color_blending.attachmentCount = creation.blend_state.active_states ? creation.blend_state.active_states : 1; // Always have 1 blend defined.
            color_blending.pAttachments = color_blend_attachment;
            color_blending.blendConstants[0] = 0.0f; // Optional
            color_blending.blendConstants[1] = 0.0f; // Optional
            color_blending.blendConstants[2] = 0.0f; // Optional
            color_blending.blendConstants[3] = 0.0f; // Optional

            pipeline_info.pColorBlendState = &color_blending;

            //// Depth Stencil
            VkPipelineDepthStencilStateCreateInfo depth_stencil{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

            depth_stencil.depthWriteEnable = creation.depth_stencil.depth_write_enable ? VK_TRUE : VK_FALSE;
            depth_stencil.stencilTestEnable = creation.depth_stencil.stencil_enable ? VK_TRUE : VK_FALSE;
            depth_stencil.depthTestEnable = creation.depth_stencil.depth_enable ? VK_TRUE : VK_FALSE;
            depth_stencil.depthCompareOp = creation.depth_stencil.depth_comparison;
            depth_stencil.minDepthBounds = 0.0f; // Optional
            depth_stencil.maxDepthBounds = 1.0f; // Optional
            depth_stencil.front = {}; // Optional
            depth_stencil.back = {}; // Optional
            if (creation.depth_stencil.stencil_enable) 
            {
                // TODO: add stencil
                MF_CORE_ASSERT(false, "Depth Stencil not added");
            }

            pipeline_info.pDepthStencilState = &depth_stencil;

            //// Multisample
            VkPipelineMultisampleStateCreateInfo multisampling = {};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = vulkan_max_sample_count_bits;
            multisampling.minSampleShading = 1.0f; // Optional
            multisampling.pSampleMask = nullptr; // Optional
            multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
            multisampling.alphaToOneEnable = VK_FALSE; // Optional

            pipeline_info.pMultisampleState = &multisampling;

            //// Rasterizer
            VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;  // VK_POLYGON_MODE_LINE for wireframe
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = creation.rasterization.cull_mode;
            rasterizer.frontFace = creation.rasterization.front;
            rasterizer.depthBiasEnable = VK_FALSE;
            rasterizer.depthBiasConstantFactor = 0.0f; // Optional
            rasterizer.depthBiasClamp = 0.0f; // Optional
            rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

            pipeline_info.pRasterizationState = &rasterizer;

            //// Tessellation
            pipeline_info.pTessellationState;


            //// Viewport state
            VkViewport viewport = {};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float)swapchain_width;
            viewport.height = (float)swapchain_height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor = {};
            scissor.offset = { 0, 0 };
            scissor.extent = { swapchain_width, swapchain_height };

            VkPipelineViewportStateCreateInfo viewport_state{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
            viewport_state.viewportCount = 1;
            viewport_state.pViewports = &viewport;
            viewport_state.scissorCount = 1;
            viewport_state.pScissors = &scissor;

            pipeline_info.pViewportState = &viewport_state;

            //// Render Pass
            pipeline_info.renderPass = get_vulkan_render_pass(creation.render_pass, creation.name);

            //// Dynamic states
            VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            VkPipelineDynamicStateCreateInfo dynamic_state{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
            dynamic_state.dynamicStateCount = (u32)ArraySize(dynamic_states);
            dynamic_state.pDynamicStates = dynamic_states;

            pipeline_info.pDynamicState = &dynamic_state;

            vkCreateGraphicsPipelines(vulkan_device, pipeline_cache, 1, &pipeline_info, vulkan_allocation_callbacks, &pipeline->vk_pipeline);

            pipeline->vk_bind_point = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
        }
        else 
        {
            VkComputePipelineCreateInfo pipeline_info{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };

            pipeline_info.stage = shader_state_data->shader_stage_info[0];
            pipeline_info.layout = pipeline_layout;

            vkCreateComputePipelines(vulkan_device, pipeline_cache, 1, &pipeline_info, vulkan_allocation_callbacks, &pipeline->vk_pipeline);

            pipeline->vk_bind_point = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE;
        }

        if (cache_path != nullptr && !cache_exists) 
        {
            sizet cache_data_size = 0;
            check(vkGetPipelineCacheData(vulkan_device, pipeline_cache, &cache_data_size, nullptr), "Failed to retrieve pipeline cache data");

            void* cache_data = allocator->allocate(cache_data_size, 64);
            check(vkGetPipelineCacheData(vulkan_device, pipeline_cache, &cache_data_size, cache_data), "Failed to retrieve pipeline cache data");

            file_write_binary(cache_path, cache_data, cache_data_size);

            allocator->deallocate(cache_data);
        }

        vkDestroyPipelineCache(vulkan_device, pipeline_cache, vulkan_allocation_callbacks);

        return handle;
    }

    BufferHandle GraphicsContext::create_buffer(const BufferCreation& creation) {
        BufferHandle handle = { buffers.obtain_resource() };
        if (handle.index == k_invalid_index) {
            return handle;
        }

        Buffer* buffer = access_buffer(handle);

        buffer->name = creation.name;
        buffer->size = creation.size;
        buffer->type_flags = creation.type_flags;
        buffer->usage = creation.usage;
        buffer->handle = handle;
        buffer->global_offset = 0;
        buffer->parent_buffer = k_invalid_buffer;

        // Cache and calculate if dynamic buffer can be used.
        static const VkBufferUsageFlags k_dynamic_buffer_mask = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        const bool use_global_buffer = (creation.type_flags & k_dynamic_buffer_mask) != 0;
        if (creation.usage == ResourceUsageType::Dynamic && use_global_buffer) {
            buffer->parent_buffer = dynamic_buffer;
            return handle;
        }

        VkBufferCreateInfo buffer_info{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |creation.type_flags;
        buffer_info.size = creation.size > 0 ? creation.size : 1;       // 0 sized creations are not permitted.

        // NOTE(marco): technically we could map a buffer if the device exposes a heap
        // with MEMORY_PROPERTY_DEVICE_LOCAL_BIT and MEMORY_PROPERTY_HOST_VISIBLE_BIT
        // but that's usually very small (256MB) unless resizable bar is enabled.
        // We simply don't allow it for now.
        MF_CORE_ASSERT(!(creation.persistent && creation.device_only), "");

        VmaAllocationCreateInfo memory_info{};
        memory_info.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
        if (creation.persistent)
        {
            memory_info.flags = memory_info.flags | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }

        if (creation.device_only)
        {
            memory_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        }
        else 
        {
            memory_info.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
        }

        VmaAllocationInfo allocation_info{};
        check(vmaCreateBuffer(vma_allocator, &buffer_info, &memory_info,
            &buffer->vk_buffer, &buffer->vma_allocation, &allocation_info), "Failed to create buffer");

        set_resource_name(VK_OBJECT_TYPE_BUFFER, (u64)buffer->vk_buffer, creation.name);

        buffer->vk_device_memory = allocation_info.deviceMemory;

        if (creation.initial_data) 
        {
            void* data;
            vmaMapMemory(vma_allocator, buffer->vma_allocation, &data);
            memcpy(data, creation.initial_data, (size_t)creation.size);
            vmaUnmapMemory(vma_allocator, buffer->vma_allocation);
        }

        if ( creation.persistent )
        {
            buffer->mapped_data = static_cast<u8 *>(allocation_info.pMappedData);
        }

        return handle;
    }

    SamplerHandle GraphicsContext::create_sampler(const SamplerCreation& creation) {
        SamplerHandle handle = { samplers.obtain_resource() };
        if (handle.index == k_invalid_index) {
            return handle;
        }

        Sampler* sampler = access_sampler(handle);

        sampler->address_mode_u = creation.address_mode_u;
        sampler->address_mode_v = creation.address_mode_v;
        sampler->address_mode_w = creation.address_mode_w;
        sampler->min_filter = creation.min_filter;
        sampler->mag_filter = creation.mag_filter;
        sampler->mip_filter = creation.mip_filter;
        sampler->name = creation.name;

        VkSamplerCreateInfo create_info{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        create_info.addressModeU = creation.address_mode_u;
        create_info.addressModeV = creation.address_mode_v;
        create_info.addressModeW = creation.address_mode_w;
        create_info.minFilter = creation.min_filter;
        create_info.magFilter = creation.mag_filter;
        create_info.mipmapMode = creation.mip_filter;
        create_info.minLod = 0.f;
        create_info.anisotropyEnable = vulkan_physical_features.features.samplerAnisotropy;
        create_info.maxAnisotropy = vulkan_physical_properties.limits.maxSamplerAnisotropy;
        create_info.compareEnable = VK_FALSE;
        create_info.compareOp = VK_COMPARE_OP_ALWAYS;
        create_info.unnormalizedCoordinates = 0;
        create_info.borderColor = VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_WHITE;
        // TODO:
        /*float                   mipLodBias;
        float                   maxAnisotropy;
        VkCompareOp             compareOp;
        float                   minLod;
        float                   maxLod;
        VkBorderColor           borderColor;
        VkBool32                unnormalizedCoordinates;*/

        check(vkCreateSampler(vulkan_device, &create_info, vulkan_allocation_callbacks, &sampler->vk_sampler), "Failed to create sampler");

        set_resource_name(VK_OBJECT_TYPE_SAMPLER, (u64)sampler->vk_sampler, creation.name);

        return handle;
    }


    DescriptorSetLayoutHandle GraphicsContext::create_descriptor_set_layout(const DescriptorSetLayoutCreation& creation) {
        DescriptorSetLayoutHandle handle = { descriptor_set_layouts.obtain_resource() };
        if (handle.index == k_invalid_index) {
            return handle;
        }

        DescriptorSetLayout* descriptor_set_layout = access_descriptor_set_layout(handle);

        // TODO: add support for multiple sets.
        // Create flattened binding list
        descriptor_set_layout->num_bindings = (u16)creation.num_bindings;
        u8* memory = mfallocam((sizeof(VkDescriptorSetLayoutBinding) + sizeof(DescriptorBinding)) * creation.num_bindings, allocator);
        descriptor_set_layout->bindings = (DescriptorBinding*)memory;
        descriptor_set_layout->vk_binding = (VkDescriptorSetLayoutBinding*)(memory + sizeof(DescriptorBinding) * creation.num_bindings);
        descriptor_set_layout->handle = handle;
        descriptor_set_layout->set_index = u16(creation.set_index);

        u32 used_bindings = 0;
        for (u32 r = 0; r < creation.num_bindings; ++r) {
            DescriptorBinding& binding = descriptor_set_layout->bindings[r];
            const DescriptorSetLayoutCreation::Binding& input_binding = creation.bindings[r];
            binding.start = input_binding.start == u16_max ? (u16)r : input_binding.start;
            binding.count = 1;
            binding.type = input_binding.type;
            binding.name = input_binding.name;

            // [TAG: BINDLESS]
            // Skip bindings for images and textures as they are bindless, thus bound in the global bindless arrays (one for images, one for textures).
            if (bindless_supported && (binding.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER || binding.type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE))
            {
                continue;
            }

            VkDescriptorSetLayoutBinding& vk_binding = descriptor_set_layout->vk_binding[used_bindings];
            ++used_bindings;

            vk_binding.binding = binding.start;
            vk_binding.descriptorType = input_binding.type;
            vk_binding.descriptorType = vk_binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : vk_binding.descriptorType;
            vk_binding.descriptorCount = 1;

            // TODO:
            vk_binding.stageFlags = VK_SHADER_STAGE_ALL;
            vk_binding.pImmutableSamplers = nullptr;
        }

        // Create the descriptor set layout
        VkDescriptorSetLayoutCreateInfo layout_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        layout_info.bindingCount = used_bindings;// creation.num_bindings;
        layout_info.pBindings = descriptor_set_layout->vk_binding;

        vkCreateDescriptorSetLayout(vulkan_device, &layout_info, vulkan_allocation_callbacks, &descriptor_set_layout->vk_descriptor_set_layout);

        return handle;
    }

    //
    //
    void GraphicsContext::fill_write_descriptor_sets(GraphicsContext& gpu, const DescriptorSetLayout* descriptor_set_layout, VkDescriptorSet vk_descriptor_set,
        VkWriteDescriptorSet* descriptor_write, VkDescriptorBufferInfo* buffer_info, VkDescriptorImageInfo* image_info,
        VkSampler vk_default_sampler, u32& num_resources, const ResourceHandle* resources, const SamplerHandle* samplers, const u16* bindings) {

        u32 used_resources = 0;
        for (u32 r = 0; r < num_resources; r++) {

            // Binding array contains the index into the resource layout binding to retrieve
            // the correct binding informations.
            u32 layout_binding_index = bindings[r];

            const DescriptorBinding& binding = descriptor_set_layout->bindings[layout_binding_index];

            // [TAG: BINDLESS]
        // Skip bindless descriptors as they are bound in the global bindless arrays.
            if (gpu.bindless_supported && (binding.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER || binding.type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)) {
                continue;
            }

            u32 i = used_resources;
            ++used_resources;

            descriptor_write[i] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            descriptor_write[i].dstSet = vk_descriptor_set;
            // Use binding array to get final binding point.
            const u32 binding_point = binding.start;
            descriptor_write[i].dstBinding = binding_point;
            descriptor_write[i].dstArrayElement = 0;
            descriptor_write[i].descriptorCount = 1;

            switch (binding.type) {
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            {
                descriptor_write[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

                TextureHandle texture_handle = { resources[r] };
                Texture* texture_data = gpu.access_texture(texture_handle);

                // Find proper sampler.
                // TODO: improve. Remove the single texture interface ?
                image_info[i].sampler = vk_default_sampler;
                if (texture_data->sampler) {
                    image_info[i].sampler = texture_data->sampler->vk_sampler;
                }
                // TODO: else ?
                if (samplers[r].index != k_invalid_index) {
                    Sampler* sampler = gpu.access_sampler({ samplers[r] });
                    image_info[i].sampler = sampler->vk_sampler;
                }

                image_info[i].imageLayout = TextureFormat::has_depth_or_stencil(texture_data->vk_format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                image_info[i].imageView = texture_data->vk_image_view;

                descriptor_write[i].pImageInfo = &image_info[i];

                break;
            }

            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            {
                descriptor_write[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

                TextureHandle texture_handle = { resources[r] };
                Texture* texture_data = gpu.access_texture(texture_handle);

                image_info[i].sampler = nullptr;
                image_info[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                image_info[i].imageView = texture_data->vk_image_view;

                descriptor_write[i].pImageInfo = &image_info[i];

                break;
            }

            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            {
                BufferHandle buffer_handle = { resources[r] };
                Buffer* buffer = gpu.access_buffer(buffer_handle);

                descriptor_write[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptor_write[i].descriptorType = buffer->usage == ResourceUsageType::Dynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

                // Bind parent buffer if present, used for dynamic resources.
                if (buffer->parent_buffer.index != k_invalid_index) {
                    Buffer* parent_buffer = gpu.access_buffer(buffer->parent_buffer);

                    buffer_info[i].buffer = parent_buffer->vk_buffer;
                }
                else {
                    buffer_info[i].buffer = buffer->vk_buffer;
                }

                buffer_info[i].offset = 0;
                buffer_info[i].range = buffer->size;

                descriptor_write[i].pBufferInfo = &buffer_info[i];

                break;
            }

            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            {
                BufferHandle buffer_handle = { resources[r] };
                Buffer* buffer = gpu.access_buffer(buffer_handle);

                descriptor_write[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                // Bind parent buffer if present, used for dynamic resources.
                if (buffer->parent_buffer.index != k_invalid_index) {
                    Buffer* parent_buffer = gpu.access_buffer(buffer->parent_buffer);

                    buffer_info[i].buffer = parent_buffer->vk_buffer;
                }
                else {
                    buffer_info[i].buffer = buffer->vk_buffer;
                }

                buffer_info[i].offset = 0;
                buffer_info[i].range = buffer->size;

                descriptor_write[i].pBufferInfo = &buffer_info[i];

                break;
            }

            default:
            {
                MF_CORE_ASSERT(false, "Resource type {} not supported in descriptor set creation!", binding.type);
                break;
            }
            }
        }

        num_resources = used_resources;
    }

    DescriptorSetHandle GraphicsContext::create_descriptor_set(const DescriptorSetCreation& creation) {
        DescriptorSetHandle handle = { descriptor_sets.obtain_resource() };
        if (handle.index == k_invalid_index) {
            return handle;
        }

        DescriptorSet* descriptor_set = access_descriptor_set(handle);
        const DescriptorSetLayout* descriptor_set_layout = access_descriptor_set_layout(creation.layout);

        // Allocate descriptor set
        VkDescriptorSetAllocateInfo alloc_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        alloc_info.descriptorPool = vulkan_descriptor_pool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &descriptor_set_layout->vk_descriptor_set_layout;

        check(vkAllocateDescriptorSets(vulkan_device, &alloc_info, &descriptor_set->vk_descriptor_set), "Failed to allocate descriptor sets");
        // Cache data
        u8* memory = mfallocam((sizeof(ResourceHandle) + sizeof(SamplerHandle) + sizeof(u16)) * creation.num_resources, allocator);
        descriptor_set->resources = (ResourceHandle*)memory;
        descriptor_set->samplers = (SamplerHandle*)(memory + sizeof(ResourceHandle) * creation.num_resources);
        descriptor_set->bindings = (u16*)(memory + (sizeof(ResourceHandle) + sizeof(SamplerHandle)) * creation.num_resources);
        descriptor_set->num_resources = creation.num_resources;
        descriptor_set->layout = descriptor_set_layout;

        // Update descriptor set
        VkWriteDescriptorSet descriptor_write[8];
        VkDescriptorBufferInfo buffer_info[8];
        VkDescriptorImageInfo image_info[8];

        Sampler* vk_default_sampler = access_sampler(default_sampler);

        u32 num_resources = creation.num_resources;
        fill_write_descriptor_sets(*this, descriptor_set_layout, descriptor_set->vk_descriptor_set, descriptor_write, buffer_info, image_info, vk_default_sampler->vk_sampler,
            num_resources, creation.resources, creation.samplers, creation.bindings);

        // Cache resources
        for (u32 r = 0; r < creation.num_resources; r++) {
            descriptor_set->resources[r] = creation.resources[r];
            descriptor_set->samplers[r] = creation.samplers[r];
            descriptor_set->bindings[r] = creation.bindings[r];
        }

        vkUpdateDescriptorSets(vulkan_device, num_resources, descriptor_write, 0, nullptr);

        return handle;
    }

    static void vulkan_create_swapchain_pass(GraphicsContext& gpu, const RenderPassCreation& creation, RenderPass* render_pass) {
        // Color attachment
        VkAttachmentDescription color_attachment = {};
        color_attachment.format = gpu.vulkan_surface_format.format;
        color_attachment.samples = gpu.vulkan_max_sample_count_bits;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // if MSAA use VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference color_attachment_ref = {};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 

        // Depth attachment
        VkAttachmentDescription depth_attachment{};
        Texture* depth_texture_vk = gpu.access_texture(gpu.depth_texture);
        depth_attachment.format = depth_texture_vk->vk_format;
        depth_attachment.samples = gpu.vulkan_max_sample_count_bits;
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_attachment_ref{};
        depth_attachment_ref.attachment = 1;
        depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        /*VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = gpu.vulkan_surface_format.format;
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentResolveRef{};
        colorAttachmentResolveRef.attachment = 2;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;*/


        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;
        subpass.pDepthStencilAttachment = &depth_attachment_ref;
        //subpass.pResolveAttachments = &colorAttachmentResolveRef;

        // Subpass Dependencies
        /*VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;*/


        VkAttachmentDescription attachments[] = { color_attachment, depth_attachment }; //, colorAttachmentResolve };
        VkRenderPassCreateInfo render_pass_info = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
        render_pass_info.attachmentCount = ArraySize(attachments);
        render_pass_info.pAttachments = attachments;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        //render_pass_info.dependencyCount = 1;
        //render_pass_info.pDependencies = &dependency;

        check(vkCreateRenderPass(gpu.vulkan_device, &render_pass_info, nullptr, &render_pass->vk_render_pass), "Failed to create renderpass");

        gpu.set_resource_name(VK_OBJECT_TYPE_RENDER_PASS, (u64)render_pass->vk_render_pass, creation.name);

        // Create framebuffer into the device.
        VkFramebufferCreateInfo framebuffer_info{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        framebuffer_info.renderPass = render_pass->vk_render_pass;
        framebuffer_info.attachmentCount = 2;
        framebuffer_info.width = gpu.swapchain_width;
        framebuffer_info.height = gpu.swapchain_height;
        framebuffer_info.layers = 1;


        VkImageView framebuffer_attachments[3];
        framebuffer_attachments[1] = depth_texture_vk->vk_image_view;
        // color image view?

        for (size_t i = 0; i < gpu.vulkan_swapchain_image_count; i++) {
            framebuffer_attachments[0] = gpu.vulkan_swapchain_image_views[i];
            framebuffer_info.pAttachments = framebuffer_attachments;

            vkCreateFramebuffer(gpu.vulkan_device, &framebuffer_info, nullptr, &gpu.vulkan_swapchain_framebuffers[i]);
            gpu.set_resource_name(VK_OBJECT_TYPE_FRAMEBUFFER, (u64)gpu.vulkan_swapchain_framebuffers[i], creation.name);
        }

        render_pass->width = gpu.swapchain_width;
        render_pass->height = gpu.swapchain_height;

        // Manually transition the texture
        VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        CommandBuffer* command_buffer = gpu.get_command_buffer(0, false);
        vkBeginCommandBuffer(command_buffer->vk_command_buffer, &beginInfo);

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { gpu.swapchain_width, gpu.swapchain_height, 1 };

        // Transition
        for (size_t i = 0; i < gpu.vulkan_swapchain_image_count; i++) {
            util_add_image_barrier(command_buffer->vk_command_buffer, gpu.vulkan_swapchain_images[i], RESOURCE_STATE_UNDEFINED, RESOURCE_STATE_PRESENT, 0, 1, false);
        }

        vkEndCommandBuffer(command_buffer->vk_command_buffer);

        // Submit command buffer
        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &command_buffer->vk_command_buffer;

        vkQueueSubmit(gpu.vulkan_main_queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(gpu.vulkan_main_queue);
    }

    static void vulkan_create_framebuffer(GraphicsContext& gpu, RenderPass* render_pass, const TextureHandle* output_textures, u32 num_render_targets, TextureHandle depth_stencil_texture) {
        // Create framebuffer
        VkFramebufferCreateInfo framebuffer_info{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        framebuffer_info.renderPass = render_pass->vk_render_pass;
        framebuffer_info.width = render_pass->width;
        framebuffer_info.height = render_pass->height;
        framebuffer_info.layers = 1;

        VkImageView framebuffer_attachments[k_max_image_outputs + 1]{};
        u32 active_attachments = 0;
        for (; active_attachments < num_render_targets; ++active_attachments) {
            Texture* texture_vk = gpu.access_texture(output_textures[active_attachments]);
            framebuffer_attachments[active_attachments] = texture_vk->vk_image_view;
        }

        if (depth_stencil_texture.index != k_invalid_index) {
            Texture* depth_texture_vk = gpu.access_texture(depth_stencil_texture);
            framebuffer_attachments[active_attachments++] = depth_texture_vk->vk_image_view;
        }
        framebuffer_info.pAttachments = framebuffer_attachments;
        framebuffer_info.attachmentCount = active_attachments;

        vkCreateFramebuffer(gpu.vulkan_device, &framebuffer_info, nullptr, &render_pass->vk_frame_buffer);
        gpu.set_resource_name(VK_OBJECT_TYPE_FRAMEBUFFER, (u64)render_pass->vk_frame_buffer, render_pass->name);
    }

    //
    //
    static VkRenderPass vulkan_create_render_pass(GraphicsContext& gpu, const RenderPassOutput& output, cstring name) {
        VkAttachmentDescription color_attachments[8] = {};
        VkAttachmentReference color_attachments_ref[8] = {};

        VkAttachmentLoadOp color_op, depth_op, stencil_op;
        VkImageLayout color_initial, depth_initial;

        switch (output.color_operation) {
        case RenderPassOperation::Load:
            color_op = VK_ATTACHMENT_LOAD_OP_LOAD;
            color_initial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            break;
        case RenderPassOperation::Clear:
            color_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
            color_initial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            break;
        default:
            color_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color_initial = VK_IMAGE_LAYOUT_UNDEFINED;
            break;
        }

        switch (output.depth_operation) {
        case RenderPassOperation::Load:
            depth_op = VK_ATTACHMENT_LOAD_OP_LOAD;
            depth_initial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            break;
        case RenderPassOperation::Clear:
            depth_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depth_initial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            break;
        default:
            depth_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depth_initial = VK_IMAGE_LAYOUT_UNDEFINED;
            break;
        }

        switch (output.stencil_operation) {
        case RenderPassOperation::Load:
            stencil_op = VK_ATTACHMENT_LOAD_OP_LOAD;
            break;
        case RenderPassOperation::Clear:
            stencil_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
            break;
        default:
            stencil_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            break;
        }

        // Color attachments
        u32 c = 0;
        for (; c < output.num_color_formats; ++c) {
            VkAttachmentDescription& color_attachment = color_attachments[c];
            color_attachment.format = output.color_formats[c];
            color_attachment.samples = gpu.vulkan_max_sample_count_bits;
            color_attachment.loadOp = color_op;
            color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment.stencilLoadOp = stencil_op;
            color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            color_attachment.initialLayout = color_initial;
            color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentReference& color_attachment_ref = color_attachments_ref[c];
            color_attachment_ref.attachment = c;
            color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        // Depth attachment
        VkAttachmentDescription depth_attachment{};
        VkAttachmentReference depth_attachment_ref{};

        if (output.depth_stencil_format != VK_FORMAT_UNDEFINED) {

            depth_attachment.format = output.depth_stencil_format;
            depth_attachment.samples = gpu.vulkan_max_sample_count_bits;
            depth_attachment.loadOp = depth_op;
            depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            depth_attachment.stencilLoadOp = stencil_op;
            depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depth_attachment.initialLayout = depth_initial;
            depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            depth_attachment_ref.attachment = c;
            depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        // Create subpass.
        // TODO: for now is just a simple subpass, evolve API.
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        // Calculate active attachments for the subpass
        VkAttachmentDescription attachments[k_max_image_outputs + 1]{};
        u32 active_attachments = 0;
        for (; active_attachments < output.num_color_formats; ++active_attachments) {
            attachments[active_attachments] = color_attachments[active_attachments];
            ++active_attachments;
        }
        subpass.colorAttachmentCount = active_attachments ? active_attachments - 1 : 0;
        subpass.pColorAttachments = color_attachments_ref;

        subpass.pDepthStencilAttachment = nullptr;

        u32 depth_stencil_count = 0;
        if (output.depth_stencil_format != VK_FORMAT_UNDEFINED) {
            attachments[subpass.colorAttachmentCount] = depth_attachment;

            subpass.pDepthStencilAttachment = &depth_attachment_ref;

            depth_stencil_count = 1;
        }

        VkRenderPassCreateInfo render_pass_info = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };

        render_pass_info.attachmentCount = (active_attachments ? active_attachments - 1 : 0) + depth_stencil_count;
        render_pass_info.pAttachments = attachments;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;

        // Create external subpass dependencies
        //VkSubpassDependency external_dependencies[ 16 ];
        //u32 num_external_dependencies = 0;

        VkRenderPass vk_render_pass;
        check(vkCreateRenderPass(gpu.vulkan_device, &render_pass_info, nullptr, &vk_render_pass), "failed to create renderpass");
        
        gpu.set_resource_name(VK_OBJECT_TYPE_RENDER_PASS, (u64)vk_render_pass, name);

        return vk_render_pass;
    }

    //
    //
    static RenderPassOutput fill_render_pass_output(GraphicsContext& gpu, const RenderPassCreation& creation) {
        RenderPassOutput output;
        output.reset();

        for (u32 i = 0; i < creation.num_render_targets; ++i) {
            Texture* texture_vk = gpu.access_texture(creation.output_textures[i]);
            output.color(texture_vk->vk_format);
        }
        if (creation.depth_stencil_texture.index != k_invalid_index) {
            Texture* texture_vk = gpu.access_texture(creation.depth_stencil_texture);
            output.depth(texture_vk->vk_format);
        }

        output.color_operation = creation.color_operation;
        output.depth_operation = creation.depth_operation;
        output.stencil_operation = creation.stencil_operation;

        return output;
    }

    RenderPassHandle GraphicsContext::create_render_pass(const RenderPassCreation& creation) {
        RenderPassHandle handle = { render_passes.obtain_resource() };
        if (handle.index == k_invalid_index) {
            return handle;
        }

        RenderPass* render_pass = access_render_pass(handle);
        render_pass->type = creation.type;
        // Init the rest of the struct.
        render_pass->num_render_targets = (u8)creation.num_render_targets;
        render_pass->dispatch_x = 0;
        render_pass->dispatch_y = 0;
        render_pass->dispatch_z = 0;
        render_pass->name = creation.name;
        render_pass->vk_frame_buffer = nullptr;
        render_pass->vk_render_pass = nullptr;
        render_pass->scale_x = creation.scale_x;
        render_pass->scale_y = creation.scale_y;
        render_pass->resize = creation.resize;

        // Cache texture handles
        u32 c = 0;
        for (; c < creation.num_render_targets; ++c) {
            Texture* texture_vk = access_texture(creation.output_textures[c]);

            render_pass->width = texture_vk->width;
            render_pass->height = texture_vk->height;

            // Cache texture handles
            render_pass->output_textures[c] = creation.output_textures[c];
        }

        render_pass->output_depth = creation.depth_stencil_texture;

        switch (creation.type) {
        case RenderPassType::Swapchain:
        {
            vulkan_create_swapchain_pass(*this, creation, render_pass);

            break;
        }

        case RenderPassType::Compute:
        {
            break;
        }

        case RenderPassType::Geometry:
        {
            render_pass->output = fill_render_pass_output(*this, creation);
            render_pass->vk_render_pass = get_vulkan_render_pass(render_pass->output, creation.name);

            vulkan_create_framebuffer(*this, render_pass, creation.output_textures, creation.num_render_targets, creation.depth_stencil_texture);

            break;
        }
        }

        return handle;
    }


    // Resource Destruction /////////////////////////////////////////////////////////

    void GraphicsContext::destroy_buffer(BufferHandle buffer) {
        if (buffer.index < buffers.pool_size) {
            resource_deletion_queue.push({ ResourceDeletionType::Buffer, buffer.index, current_frame });
        }
        else {
            MF_CORE_ERROR("Graphics error: trying to free invalid Buffer {}", buffer.index);
        }
    }

    void GraphicsContext::destroy_texture(TextureHandle texture)
    {
        if (texture.index < textures.pool_size) 
        {
            resource_deletion_queue.push({ ResourceDeletionType::Texture, texture.index, current_frame });
            texture_to_update_bindless.push({ ResourceDeletionType::Texture, texture.index, current_frame });
        }
        else 
        {
            MF_CORE_ERROR("Graphics error: trying to free invalid Texture {}", texture.index);
        }
    }

    void GraphicsContext::destroy_pipeline(PipelineHandle pipeline) 
    {
        if (pipeline.index < pipelines.pool_size)
        {
            resource_deletion_queue.push({ ResourceDeletionType::Pipeline, pipeline.index, current_frame });
            // Shader state creation is handled internally when creating a pipeline, thus add this to track correctly.
            Pipeline* v_pipeline = access_pipeline(pipeline);

            ShaderState* shader_state_data = access_shader_state(v_pipeline->shader_state);
            for (u32 l = 0; l < shader_state_data->parse_result->set_count; ++l) 
            {
                destroy_descriptor_set_layout(v_pipeline->descriptor_set_layout_handle[l]);
            }
            destroy_shader_state(v_pipeline->shader_state);
        }
        else
        {
            MF_CORE_ERROR("Graphics error: trying to free invalid Pipeline {}", pipeline.index);
        }
    }

    void GraphicsContext::destroy_sampler(SamplerHandle sampler)
    {
        if (sampler.index < samplers.pool_size) 
        {
            resource_deletion_queue.push({ ResourceDeletionType::Sampler, sampler.index, current_frame });
        }
        else {
            MF_CORE_ERROR("Graphics error: trying to free invalid Sampler {}", sampler.index);
        }
    }

    void GraphicsContext::destroy_descriptor_set_layout(DescriptorSetLayoutHandle descriptor_set_layout) {
        if (descriptor_set_layout.index < descriptor_set_layouts.pool_size) {
            resource_deletion_queue.push({ ResourceDeletionType::DescriptorSetLayout, descriptor_set_layout.index, current_frame });
        }
        else {
            MF_CORE_ERROR("Graphics error: trying to free invalid DescriptorSetLayout {}", descriptor_set_layout.index);
        }
    }

    void GraphicsContext::destroy_descriptor_set(DescriptorSetHandle descriptor_set) {
        if (descriptor_set.index < descriptor_sets.pool_size) {
            resource_deletion_queue.push({ ResourceDeletionType::DescriptorSet, descriptor_set.index, current_frame });
        }
        else {
            MF_CORE_ERROR("Graphics error: trying to free invalid DescriptorSet {}", descriptor_set.index);
        }
    }

    void GraphicsContext::destroy_render_pass(RenderPassHandle render_pass) 
    {
        if (render_pass.index < render_passes.pool_size)
        {
            resource_deletion_queue.push({ ResourceDeletionType::RenderPass, render_pass.index, current_frame });
        }
        else
        {
            MF_CORE_ERROR("Graphics error: trying to free invalid RenderPass {}", render_pass.index);
        }
    }

    void GraphicsContext::destroy_shader_state(ShaderStateHandle shader) 
    {
        if (shader.index < shaders.pool_size)
        {
            resource_deletion_queue.push({ ResourceDeletionType::ShaderState, shader.index, current_frame });

            ShaderState* state = access_shader_state(shader);

            allocator->deallocate(state->parse_result);

        }
        else
        {
            MF_CORE_ERROR("Graphics error: trying to free invalid Shader {}", shader.index);
        }
    }

    // Real destruction methods - the other enqueue only the resources.
    void GraphicsContext::destroy_buffer_instant(ResourceHandle buffer) 
    {

        Buffer* v_buffer = (Buffer*)buffers.access_resource(buffer);

        if (v_buffer && v_buffer->parent_buffer.index == k_invalid_buffer.index) 
        {
            vmaDestroyBuffer(vma_allocator, v_buffer->vk_buffer, v_buffer->vma_allocation);
        }
        buffers.release_resource(buffer);
    }

    void GraphicsContext::destroy_texture_instant(ResourceHandle texture) {
        Texture* v_texture = (Texture*)textures.access_resource(texture);

        if (v_texture) {
            //MF_CORE_ERROR( "Destroying image view %x %u\n", v_texture->vk_image_view, v_texture->handle.index );
            vkDestroyImageView(vulkan_device, v_texture->vk_image_view, vulkan_allocation_callbacks);
            vmaDestroyImage(vma_allocator, v_texture->vk_image, v_texture->vma_allocation);
        }
        textures.release_resource(texture);
    }

    void GraphicsContext::destroy_pipeline_instant(ResourceHandle pipeline) {
        Pipeline* v_pipeline = (Pipeline*)pipelines.access_resource(pipeline);

        if (v_pipeline) {
            vkDestroyPipeline(vulkan_device, v_pipeline->vk_pipeline, vulkan_allocation_callbacks);

            vkDestroyPipelineLayout(vulkan_device, v_pipeline->vk_pipeline_layout, vulkan_allocation_callbacks);
        }
        pipelines.release_resource(pipeline);
    }

    void GraphicsContext::destroy_sampler_instant(ResourceHandle sampler) {
        Sampler* v_sampler = (Sampler*)samplers.access_resource(sampler);

        if (v_sampler) {
            vkDestroySampler(vulkan_device, v_sampler->vk_sampler, vulkan_allocation_callbacks);
        }
        samplers.release_resource(sampler);
    }

    void GraphicsContext::destroy_descriptor_set_layout_instant(ResourceHandle descriptor_set_layout) {
        DescriptorSetLayout* v_descriptor_set_layout = (DescriptorSetLayout*)descriptor_set_layouts.access_resource(descriptor_set_layout);

        if (v_descriptor_set_layout) {
            vkDestroyDescriptorSetLayout(vulkan_device, v_descriptor_set_layout->vk_descriptor_set_layout, vulkan_allocation_callbacks);

            // This contains also vk_binding allocation.
            mffree(v_descriptor_set_layout->bindings, allocator);
        }
        descriptor_set_layouts.release_resource(descriptor_set_layout);
    }

    void GraphicsContext::destroy_descriptor_set_instant(ResourceHandle descriptor_set) {
        DescriptorSet* v_descriptor_set = (DescriptorSet*)descriptor_sets.access_resource(descriptor_set);

        if (v_descriptor_set) {
            // Contains the allocation for all the resources, binding and samplers arrays.
            mffree(v_descriptor_set->resources, allocator);
            // This is freed with the DescriptorSet pool.
            //vkFreeDescriptorSets
        }
        descriptor_sets.release_resource(descriptor_set);
    }

    void GraphicsContext::destroy_render_pass_instant(ResourceHandle render_pass) {
        RenderPass* v_render_pass = (RenderPass*)render_passes.access_resource(render_pass);

        if (v_render_pass) {

            if (v_render_pass->num_render_targets)
                vkDestroyFramebuffer(vulkan_device, v_render_pass->vk_frame_buffer, vulkan_allocation_callbacks);

            // NOTE: this is now destroyed with the render pass cache, to avoid double deletes.
            //vkDestroyRenderPass( vulkan_device, v_render_pass->vk_render_pass, vulkan_allocation_callbacks );
        }
        render_passes.release_resource(render_pass);
    }

    void GraphicsContext::destroy_shader_state_instant(ResourceHandle shader) {
        ShaderState* v_shader_state = (ShaderState*)shaders.access_resource(shader);
        if (v_shader_state) {

            for (size_t i = 0; i < v_shader_state->active_shaders; i++) {
                vkDestroyShaderModule(vulkan_device, v_shader_state->shader_stage_info[i].module, vulkan_allocation_callbacks);
            }
        }
        shaders.release_resource(shader);
    }

    void GraphicsContext::set_resource_name(VkObjectType type, u64 handle, const char* name) 
    {

        if (!debug_utils_extension_present || pfnSetDebugUtilsObjectNameEXT == nullptr)
        {
            return;
        }
        VkDebugUtilsObjectNameInfoEXT name_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
        name_info.objectType = type;
        name_info.objectHandle = handle;
        name_info.pObjectName = name;
        pfnSetDebugUtilsObjectNameEXT(vulkan_device, &name_info);
    }

    void GraphicsContext::push_marker(VkCommandBuffer command_buffer, cstring name) {

        VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
        label.pLabelName = name;
        label.color[0] = 1.0f;
        label.color[1] = 1.0f;
        label.color[2] = 1.0f;
        label.color[3] = 1.0f;
        pfnCmdBeginDebugUtilsLabelEXT(command_buffer, &label);
    }

    void GraphicsContext::pop_marker(VkCommandBuffer command_buffer) {
        pfnCmdEndDebugUtilsLabelEXT(command_buffer);
    }

    // Swapchain //////////////////////////////////////////////////////////////

    template<class T>
    constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
        MF_CORE_ASSERT(!(hi < lo), "How high can you get? How low can you go?");
        return (v < lo) ? lo : (hi < v) ? hi : v;
    }

    void GraphicsContext::create_swapchain() {

        //// Check if surface is supported
        // TODO: Windows only!
        VkBool32 surface_supported;
        vkGetPhysicalDeviceSurfaceSupportKHR(vulkan_physical_device, vulkan_main_queue_family, vulkan_window_surface, &surface_supported);
        if (surface_supported != VK_TRUE) 
        {
            MF_CORE_ERROR("Error no WSI support on physical device 0");
        }

        VkSurfaceCapabilitiesKHR surface_capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan_physical_device, vulkan_window_surface, &surface_capabilities);

        VkExtent2D swapchain_extent = surface_capabilities.currentExtent;
        if (swapchain_extent.width == UINT32_MAX) 
        {
            swapchain_extent.width = clamp(swapchain_extent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
            swapchain_extent.height = clamp(swapchain_extent.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
        }

        MF_CORE_INFO("Create swapchain {} {} saved {} {}, min image count {}, max image count {}", swapchain_extent.width, swapchain_extent.height, swapchain_width, swapchain_height, surface_capabilities.minImageCount, surface_capabilities.maxImageCount);

        swapchain_width = (u16)swapchain_extent.width;
        swapchain_height = (u16)swapchain_extent.height;

        //vulkan_swapchain_image_count = surface_capabilities.minImageCount + 2;

        VkSwapchainCreateInfoKHR swapchain_create_info = {};
        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_create_info.surface = vulkan_window_surface;
        swapchain_create_info.minImageCount = vulkan_swapchain_image_count;
        swapchain_create_info.imageFormat = vulkan_surface_format.format;
        swapchain_create_info.imageExtent = swapchain_extent;
        swapchain_create_info.clipped = VK_TRUE;
        swapchain_create_info.imageArrayLayers = 1;
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.preTransform = surface_capabilities.currentTransform;
        swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchain_create_info.presentMode = vulkan_present_mode;

        VkResult result = vkCreateSwapchainKHR(vulkan_device, &swapchain_create_info, 0, &vulkan_swapchain);
        check(result, "Failed to create swap chain");   

        //// Cache swapchain images
        vkGetSwapchainImagesKHR(vulkan_device, vulkan_swapchain, &vulkan_swapchain_image_count, NULL);
        vkGetSwapchainImagesKHR(vulkan_device, vulkan_swapchain, &vulkan_swapchain_image_count, vulkan_swapchain_images);

        for (size_t iv = 0; iv < vulkan_swapchain_image_count; iv++) {
            // Create an image view which we can render into.
            VkImageViewCreateInfo view_info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
            view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_info.format = vulkan_surface_format.format;
            view_info.image = vulkan_swapchain_images[iv];
            view_info.subresourceRange.levelCount = 1;
            view_info.subresourceRange.layerCount = 1;
            view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view_info.components.r = VK_COMPONENT_SWIZZLE_R;
            view_info.components.g = VK_COMPONENT_SWIZZLE_G;
            view_info.components.b = VK_COMPONENT_SWIZZLE_B;
            view_info.components.a = VK_COMPONENT_SWIZZLE_A;

            check(vkCreateImageView(vulkan_device, &view_info, vulkan_allocation_callbacks, &vulkan_swapchain_image_views[iv]), "Failed to create an image view");
        }
    }

    void GraphicsContext::destroy_swapchain() {

        for (size_t iv = 0; iv < vulkan_swapchain_image_count; iv++) {
            vkDestroyImageView(vulkan_device, vulkan_swapchain_image_views[iv], vulkan_allocation_callbacks);
            vkDestroyFramebuffer(vulkan_device, vulkan_swapchain_framebuffers[iv], vulkan_allocation_callbacks);
        }

        vkDestroySwapchainKHR(vulkan_device, vulkan_swapchain, vulkan_allocation_callbacks);
    }

    VkRenderPass GraphicsContext::get_vulkan_render_pass(const RenderPassOutput& output, cstring name) {

        // Hash the memory output and find a compatible VkRenderPass.
        // In current form RenderPassOutput should track everything needed, including load operations.
        u64 hashed_memory = Magnefu::hash_bytes((void*)&output, sizeof(RenderPassOutput));
        VkRenderPass vulkan_render_pass = render_pass_cache.get(hashed_memory);
        if (vulkan_render_pass) {
            return vulkan_render_pass;
        }
        vulkan_render_pass = vulkan_create_render_pass(*this, output, name);
        render_pass_cache.insert(hashed_memory, vulkan_render_pass);

        return vulkan_render_pass;
    }

    //
    //
    static void vulkan_resize_texture(GraphicsContext& gpu, Texture* v_texture, Texture* v_texture_to_delete, u16 width, u16 height, u16 depth) {

        // Cache handles to be delayed destroyed
        v_texture_to_delete->vk_image_view = v_texture->vk_image_view;
        v_texture_to_delete->vk_image = v_texture->vk_image;
        v_texture_to_delete->vma_allocation = v_texture->vma_allocation;

        // Re-create image in place.
        TextureCreation tc;
        tc.set_flags(v_texture->mipmaps, v_texture->flags).set_format_type(v_texture->vk_format, v_texture->type).set_name(v_texture->name).set_size(width, height, depth);
        vulkan_create_texture(gpu, tc, v_texture->handle, v_texture);
    }

    void GraphicsContext::resize_swapchain() {

        vkDeviceWaitIdle(vulkan_device);

        VkSurfaceCapabilitiesKHR surface_capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan_physical_device, vulkan_window_surface, &surface_capabilities);
        VkExtent2D swapchain_extent = surface_capabilities.currentExtent;

        // Skip zero-sized swapchain
        //MF_CORE_ERROR( "Requested swapchain resize %u %u\n", swapchain_extent.width, swapchain_extent.height );
        if (swapchain_extent.width == 0 || swapchain_extent.height == 0) {
            //MF_CORE_ERROR( "Cannot create a zero-sized swapchain\n" );
            return;
        }

        // Internal destroy of swapchain pass to retain the same handle.
        RenderPass* vk_swapchain_pass = access_render_pass(swapchain_pass);
        vkDestroyRenderPass(vulkan_device, vk_swapchain_pass->vk_render_pass, vulkan_allocation_callbacks);

        // Destroy swapchain images and framebuffers
        destroy_swapchain();
        vkDestroySurfaceKHR(vulkan_instance, vulkan_window_surface, vulkan_allocation_callbacks);

        // Recreate window surface
        check(
            glfwCreateWindowSurface(vulkan_instance, glfw_window, vulkan_allocation_callbacks, &vulkan_window_surface),
            "Failed to create a window surface!"
        );

        
        // Create swapchain
        create_swapchain();

        // Resize depth texture, maintaining handle, using a dummy texture to destroy.
        TextureHandle texture_to_delete = { textures.obtain_resource() };
        Texture* vk_texture_to_delete = access_texture(texture_to_delete);
        vk_texture_to_delete->handle = texture_to_delete;
        Texture* vk_depth_texture = access_texture(depth_texture);
        vulkan_resize_texture(*this, vk_depth_texture, vk_texture_to_delete, swapchain_width, swapchain_height, 1);

        destroy_texture(texture_to_delete);

        RenderPassCreation swapchain_pass_creation = {};
        swapchain_pass_creation.set_type(RenderPassType::Swapchain).set_name("Swapchain");
        vulkan_create_swapchain_pass(*this, swapchain_pass_creation, vk_swapchain_pass);

        vkDeviceWaitIdle(vulkan_device);
    }

    // Descriptor Set /////////////////////////////////////////////////////////

    void GraphicsContext::update_descriptor_set(DescriptorSetHandle descriptor_set) {

        if (descriptor_set.index < descriptor_sets.pool_size) {

            DescriptorSetUpdate new_update = { descriptor_set, current_frame };
            descriptor_set_updates.push(new_update);


        }
        else {
            MF_CORE_ERROR("Graphics error: trying to update invalid DescriptorSet {}", descriptor_set.index);
        }
    }

    void GraphicsContext::update_descriptor_set_instant(const DescriptorSetUpdate& update) {

        // Use a dummy descriptor set to delete the vulkan descriptor set handle
        DescriptorSetHandle dummy_delete_descriptor_set_handle = { descriptor_sets.obtain_resource() };
        DescriptorSet* dummy_delete_descriptor_set = access_descriptor_set(dummy_delete_descriptor_set_handle);

        DescriptorSet* descriptor_set = access_descriptor_set(update.descriptor_set);
        const DescriptorSetLayout* descriptor_set_layout = descriptor_set->layout;

        dummy_delete_descriptor_set->vk_descriptor_set = descriptor_set->vk_descriptor_set;
        dummy_delete_descriptor_set->bindings = nullptr;
        dummy_delete_descriptor_set->resources = nullptr;
        dummy_delete_descriptor_set->samplers = nullptr;
        dummy_delete_descriptor_set->num_resources = 0;

        destroy_descriptor_set(dummy_delete_descriptor_set_handle);

        // Allocate the new descriptor set and update its content.
        VkWriteDescriptorSet descriptor_write[8];
        VkDescriptorBufferInfo buffer_info[8];
        VkDescriptorImageInfo image_info[8];

        Sampler* vk_default_sampler = access_sampler(default_sampler);

        VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        allocInfo.descriptorPool = vulkan_descriptor_pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &descriptor_set->layout->vk_descriptor_set_layout;
        vkAllocateDescriptorSets(vulkan_device, &allocInfo, &descriptor_set->vk_descriptor_set);

        u32 num_resources = descriptor_set_layout->num_bindings;
        fill_write_descriptor_sets(*this, descriptor_set_layout, descriptor_set->vk_descriptor_set, descriptor_write, buffer_info, image_info, vk_default_sampler->vk_sampler,
            num_resources, descriptor_set->resources, descriptor_set->samplers, descriptor_set->bindings);

        vkUpdateDescriptorSets(vulkan_device, num_resources, descriptor_write, 0, nullptr);
    }

    u32 GraphicsContext::get_memory_heap_count()
    {
        return vma_allocator->GetMemoryHeapCount();
    }

    //
    //
    void GraphicsContext::resize_output_textures(RenderPassHandle render_pass, u32 width, u32 height) {

        // For each texture, create a temporary pooled texture and cache the handles to delete.
        // This is because we substitute just the Vulkan texture when resizing so that
        // external users don't need to update the handle.

        RenderPass* vk_render_pass = access_render_pass(render_pass);
        if (vk_render_pass) {
            // No need to resize!
            if (!vk_render_pass->resize) {
                return;
            }

            // Calculate new width and height based on render pass sizing informations.
            u16 new_width = (u16)(width * vk_render_pass->scale_x);
            u16 new_height = (u16)(height * vk_render_pass->scale_y);

            // Resize textures if needed
            const u32 rts = vk_render_pass->num_render_targets;
            for (u32 i = 0; i < rts; ++i) {
                TextureHandle texture = vk_render_pass->output_textures[i];
                Texture* vk_texture = access_texture(texture);

                if (vk_texture->width == new_width && vk_texture->height == new_height) {
                    continue;
                }

                // Queue deletion of texture by creating a temporary one
                TextureHandle texture_to_delete = { textures.obtain_resource() };
                Texture* vk_texture_to_delete = access_texture(texture_to_delete);
                // Update handle so it can be used to update bindless to dummy texture.
                vk_texture_to_delete->handle = texture_to_delete;
                vulkan_resize_texture(*this, vk_texture, vk_texture_to_delete, new_width, new_height, 1);

                destroy_texture(texture_to_delete);
            }

            if (vk_render_pass->output_depth.index != k_invalid_index) {
                Texture* vk_texture = access_texture(vk_render_pass->output_depth);

                if (vk_texture->width != new_width || vk_texture->height != new_height) {
                    // Queue deletion of texture by creating a temporary one
                    TextureHandle texture_to_delete = { textures.obtain_resource() };
                    Texture* vk_texture_to_delete = access_texture(texture_to_delete);
                    // Update handle so it can be used to update bindless to dummy texture.
                    vk_texture_to_delete->handle = texture_to_delete;
                    vulkan_resize_texture(*this, vk_texture, vk_texture_to_delete, new_width, new_height, 1);

                    destroy_texture(texture_to_delete);
                }
            }

            // Again: create temporary resource to use the standard deferred deletion mechanism.
            RenderPassHandle render_pass_to_destroy = { render_passes.obtain_resource() };
            RenderPass* vk_render_pass_to_destroy = access_render_pass(render_pass_to_destroy);

            vk_render_pass_to_destroy->vk_frame_buffer = vk_render_pass->vk_frame_buffer;
            // This is checked in the destroy method to proceed with frame buffer destruction.
            vk_render_pass_to_destroy->num_render_targets = 1;
            // Set this to 0 so deletion won't be performed.
            vk_render_pass_to_destroy->vk_render_pass = 0;

            destroy_render_pass(render_pass_to_destroy);

            // Update render pass size
            vk_render_pass->width = new_width;
            vk_render_pass->height = new_height;

            // Recreate framebuffer if present (mainly for dispatch only passes)
            if (vk_render_pass->vk_frame_buffer) {
                vulkan_create_framebuffer(*this, vk_render_pass, vk_render_pass->output_textures, vk_render_pass->num_render_targets, vk_render_pass->output_depth);
            }
        }
    }

    //
    //

    void GraphicsContext::fill_barrier(RenderPassHandle render_pass, ExecutionBarrier& out_barrier) {

        RenderPass* vk_render_pass = access_render_pass(render_pass);

        out_barrier.num_image_barriers = 0;

        if (vk_render_pass) {
            const u32 rts = vk_render_pass->num_render_targets;
            for (u32 i = 0; i < rts; ++i) {
                out_barrier.image_barriers[out_barrier.num_image_barriers++].texture = vk_render_pass->output_textures[i];
            }

            if (vk_render_pass->output_depth.index != k_invalid_index) {
                out_barrier.image_barriers[out_barrier.num_image_barriers++].texture = vk_render_pass->output_depth;
            }
        }
    }

    void GraphicsContext::new_frame() {

        // Fence wait and reset
        VkFence* render_complete_fence = &vulkan_command_buffer_executed_fence[current_frame];

        if (vkGetFenceStatus(vulkan_device, *render_complete_fence) != VK_SUCCESS) {
            vkWaitForFences(vulkan_device, 1, render_complete_fence, VK_TRUE, UINT64_MAX);
        }

        vkResetFences(vulkan_device, 1, render_complete_fence);

        VkResult result = vkAcquireNextImageKHR(vulkan_device, vulkan_swapchain, UINT64_MAX, vulkan_image_acquired_semaphore, VK_NULL_HANDLE, &vulkan_image_index);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            resize_swapchain();
        }

        // Command pool reset
        command_buffer_ring.reset_pools(current_frame);
        // Dynamic memory update
        const u32 used_size = dynamic_allocated_size - (dynamic_per_frame_size * previous_frame);
        dynamic_max_per_frame_size = magnefu_max(used_size, dynamic_max_per_frame_size);
        dynamic_allocated_size = dynamic_per_frame_size * current_frame;

        // Descriptor Set Updates
        if (descriptor_set_updates.count()) 
        {
            for (i32 i = descriptor_set_updates.count() - 1; i >= 0; i--) 
            {
                DescriptorSetUpdate& update = descriptor_set_updates[i];

                //if ( update.frame_issued == current_frame )
                {
                    update_descriptor_set_instant(update);

                    update.frame_issued = u32_max;
                    descriptor_set_updates.delete_swap(i);
                }
            }
        }
    }

    void GraphicsContext::present() {

        VkFence* render_complete_fence = &vulkan_command_buffer_executed_fence[current_frame];
        VkSemaphore* render_complete_semaphore = &vulkan_render_complete_semaphore[current_frame];

        // Copy all commands
        VkCommandBuffer enqueued_command_buffers[4];
        for (u32 c = 0; c < num_queued_command_buffers; c++) {

            CommandBuffer* command_buffer = queued_command_buffers[c];

            //// trying to fix issue with un deleted command buffers at program close
            //for(u32 set = 0; set < command_buffer->descriptor_sets.used_indices; set++)
            //{
            //    DescriptorSet* ds = (DescriptorSet*)command_buffer->descriptor_sets.access_resource(set);
            //    
            //    destroy_descriptor_set((DescriptorSetHandle)set);
            //    
            //}

            enqueued_command_buffers[c] = command_buffer->vk_command_buffer;
            // NOTE: why it was needing current_pipeline to be setup ?
            if (command_buffer->is_recording && command_buffer->current_render_pass && (command_buffer->current_render_pass->type != RenderPassType::Compute))
                vkCmdEndRenderPass(command_buffer->vk_command_buffer);

            vkEndCommandBuffer(command_buffer->vk_command_buffer);
        }

        if (texture_to_update_bindless.count()) 
        {
            // Handle deferred writes to bindless textures.
            VkWriteDescriptorSet bindless_descriptor_writes[k_max_bindless_resources];
            VkDescriptorImageInfo bindless_image_info[k_max_bindless_resources];

            Texture* vk_dummy_texture = access_texture(dummy_texture);

            u32 current_write_index = 0;
            for (i32 it = texture_to_update_bindless.count() - 1; it >= 0; it--) {
                ResourceUpdate& texture_to_update = texture_to_update_bindless[it];

                //if ( texture_to_update.current_frame == current_frame )
                {
                    Texture* texture = access_texture({ texture_to_update.handle });
                    VkWriteDescriptorSet& descriptor_write = bindless_descriptor_writes[current_write_index];
                    descriptor_write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                    descriptor_write.descriptorCount = 1;
                    descriptor_write.dstArrayElement = texture_to_update.handle;
                    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptor_write.dstSet = vulkan_bindless_descriptor_set;
                    descriptor_write.dstBinding = k_bindless_texture_binding;

                    // Handles should be the same.
                    MF_CORE_ASSERT(texture->handle.index == texture_to_update.handle, "Texture handle mismatch");

                    Sampler* vk_default_sampler = access_sampler(default_sampler);
                    VkDescriptorImageInfo& descriptor_image_info = bindless_image_info[current_write_index];

                    if (texture->sampler != nullptr) 
                    {
                        descriptor_image_info.sampler = texture->sampler->vk_sampler;
                    }
                    else {
                        descriptor_image_info.sampler = vk_default_sampler->vk_sampler;
                    }

                    descriptor_image_info.imageView = texture->vk_format != VK_FORMAT_UNDEFINED ? texture->vk_image_view : vk_dummy_texture->vk_image_view;
                    descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    descriptor_write.pImageInfo = &descriptor_image_info;

                    texture_to_update.current_frame = u32_max;

                    texture_to_update_bindless.delete_swap(it);

                    ++current_write_index;
                }
            }

            if (current_write_index) 
            {
                vkUpdateDescriptorSets(vulkan_device, current_write_index, bindless_descriptor_writes, 0, nullptr);
            }
        }

        // Submit command buffers
        VkSemaphore wait_semaphores[] = { vulkan_image_acquired_semaphore };
        VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submit_info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = wait_semaphores;
        submit_info.pWaitDstStageMask = wait_stages;
        submit_info.commandBufferCount = num_queued_command_buffers;
        submit_info.pCommandBuffers = enqueued_command_buffers;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = render_complete_semaphore;

        vkQueueSubmit(vulkan_main_queue, 1, &submit_info, *render_complete_fence);

        VkPresentInfoKHR present_info{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = render_complete_semaphore;

        VkSwapchainKHR swap_chains[] = { vulkan_swapchain };
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swap_chains;
        present_info.pImageIndices = &vulkan_image_index;
        present_info.pResults = nullptr; // Optional
        VkResult result = vkQueuePresentKHR(vulkan_main_queue, &present_info);

        num_queued_command_buffers = 0;

        //
        // GPU Timestamp resolve
        if (timestamps_enabled) {
            if (gpu_timestamp_manager->has_valid_queries()) {
                // Query GPU for all timestamps.
                const u32 query_offset = (current_frame * gpu_timestamp_manager->queries_per_frame) * 2;
                const u32 query_count = gpu_timestamp_manager->current_query * 2;
                vkGetQueryPoolResults(vulkan_device, vulkan_timestamp_query_pool, query_offset, query_count,
                    sizeof(u64) * query_count * 2, &gpu_timestamp_manager->timestamps_data[query_offset],
                    sizeof(gpu_timestamp_manager->timestamps_data[0]), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

                // Calculate and cache the elapsed time
                for (u32 i = 0; i < gpu_timestamp_manager->current_query; i++) {
                    u32 index = (current_frame * gpu_timestamp_manager->queries_per_frame) + i;

                    GPUTimestamp& timestamp = gpu_timestamp_manager->timestamps[index];

                    double start = (double)gpu_timestamp_manager->timestamps_data[(index * 2)];
                    double end = (double)gpu_timestamp_manager->timestamps_data[(index * 2) + 1];
                    double range = end - start;
                    double elapsed_time = range * gpu_timestamp_frequency;

                    timestamp.elapsed_ms = elapsed_time;
                    timestamp.frame_index = absolute_frame;

                    //print_format( "%s: %2.3f d(%u) - ", timestamp.name, elapsed_time, timestamp.depth );
                }
                //print_format( "\n" );
            }
            else if (gpu_timestamp_manager->current_query) 
            {
                MF_CORE_WARN("Asymmetrical GPU queries, missing pop of some markers!");
            }

            gpu_timestamp_manager->reset();
            gpu_timestamp_reset = true;
        }
        else {
            gpu_timestamp_reset = false;
        }


        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resized) {
            resized = false;
            resize_swapchain();

            // Advance frame counters that are skipped during this frame.
            frame_counters_advance();

            return;
        }

        //Magnefu::print_format( "Index %u, %u, %u\n", current_frame, previous_frame, vulkan_image_index );

        // This is called inside resize_swapchain as well to correctly work.
        frame_counters_advance();

        // Resource deletion using reverse iteration and swap with last element.
        if (resource_deletion_queue.count() > 0) {
            for (i32 i = resource_deletion_queue.count() - 1; i >= 0; i--) {
                ResourceUpdate& resource_deletion = resource_deletion_queue[i];

                if (resource_deletion.current_frame == current_frame) {

                    switch (resource_deletion.type) {

                    case ResourceDeletionType::Buffer:
                    {
                        destroy_buffer_instant(resource_deletion.handle);
                        break;
                    }

                    case ResourceDeletionType::Pipeline:
                    {
                        destroy_pipeline_instant(resource_deletion.handle);
                        break;
                    }

                    case ResourceDeletionType::RenderPass:
                    {
                        destroy_render_pass_instant(resource_deletion.handle);
                        break;
                    }

                    case ResourceDeletionType::DescriptorSet:
                    {
                        destroy_descriptor_set_instant(resource_deletion.handle);
                        break;
                    }

                    case ResourceDeletionType::DescriptorSetLayout:
                    {
                        destroy_descriptor_set_layout_instant(resource_deletion.handle);
                        break;
                    }

                    case ResourceDeletionType::Sampler:
                    {
                        destroy_sampler_instant(resource_deletion.handle);
                        break;
                    }

                    case ResourceDeletionType::ShaderState:
                    {
                        destroy_shader_state_instant(resource_deletion.handle);
                        break;
                    }

                    case ResourceDeletionType::Texture:
                    {
                        destroy_texture_instant(resource_deletion.handle);
                        break;
                    }
                    }

                    // Mark resource as free
                    resource_deletion.current_frame = u32_max;

                    // Swap element
                    resource_deletion_queue.delete_swap(i);
                }
            }
        }
    }

    static VkPresentModeKHR to_vk_present_mode(PresentMode::Enum mode) 
    {
        switch (mode) 
        {
        case PresentMode::VSyncFast:
            return VK_PRESENT_MODE_MAILBOX_KHR;
        case PresentMode::VSyncRelaxed:
            return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
        case PresentMode::Immediate:
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
        case PresentMode::VSync:
        default:
            return VK_PRESENT_MODE_FIFO_KHR;
        }
    }

    void GraphicsContext::set_present_mode(PresentMode::Enum mode) {

        // Request a certain mode and confirm that it is available. If not use VK_PRESENT_MODE_FIFO_KHR which is mandatory
        u32 supported_count = 0;

        static VkPresentModeKHR supported_mode_allocated[8];
        vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_physical_device, vulkan_window_surface, &supported_count, NULL);
        MF_CORE_ASSERT((supported_count < 8), "Supported present modes > 8");
        vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_physical_device, vulkan_window_surface, &supported_count, supported_mode_allocated);

        bool mode_found = false;
        VkPresentModeKHR requested_mode = to_vk_present_mode(mode);
        for (u32 j = 0; j < supported_count; j++) 
        {
            if (requested_mode == supported_mode_allocated[j])
            {
                mode_found = true;
                break;
            }
        }

        // Default to VK_PRESENT_MODE_FIFO_KHR that is guaranteed to always be supported
        vulkan_present_mode = mode_found ? requested_mode : VK_PRESENT_MODE_FIFO_KHR;
        // Use 4 for immediate ?
        vulkan_swapchain_image_count = 3;// vulkan_present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR ? 2 : 3;

        present_mode = mode_found ? mode : PresentMode::VSync;
    }

    void GraphicsContext::link_texture_sampler(TextureHandle texture, SamplerHandle sampler) 
    {

        Texture* texture_vk = access_texture(texture);
        Sampler* sampler_vk = access_sampler(sampler);

        texture_vk->sampler = sampler_vk;
    }

    void GraphicsContext::frame_counters_advance() {
        previous_frame = current_frame;
        current_frame = (current_frame + 1) % vulkan_swapchain_image_count;

        ++absolute_frame;
    }

    //
    //
    void GraphicsContext::queue_command_buffer(CommandBuffer* command_buffer) {

        queued_command_buffers[num_queued_command_buffers++] = command_buffer;
    }

    //
    //
    CommandBuffer* GraphicsContext::get_command_buffer(u32 thread_index, bool begin) {
        CommandBuffer* cb = command_buffer_ring.get_command_buffer(current_frame, thread_index, begin);

        // The first commandbuffer issued in the frame is used to reset the timestamp queries used.
        if (gpu_timestamp_reset && begin)
        {
            // These are currently indices!
            vkCmdResetQueryPool(cb->vk_command_buffer, vulkan_timestamp_query_pool, current_frame * gpu_timestamp_manager->queries_per_frame * 2, gpu_timestamp_manager->queries_per_frame);

            gpu_timestamp_reset = false;
        }

        return cb;
    }

    CommandBuffer* GraphicsContext::get_secondary_command_buffer(u32 thread_index)
    {
        CommandBuffer* cb = command_buffer_ring.get_secondary_command_buffer(current_frame, thread_index);
        return cb;
    }


    // Resource Description Query ///////////////////////////////////////////////////

    void GraphicsContext::query_buffer(BufferHandle buffer, BufferDescription& out_description) {
        if (buffer.index != k_invalid_index) {
            const Buffer* buffer_data = access_buffer(buffer);

            out_description.name = buffer_data->name;
            out_description.size = buffer_data->size;
            out_description.type_flags = buffer_data->type_flags;
            out_description.usage = buffer_data->usage;
            out_description.parent_handle = buffer_data->parent_buffer;
            out_description.native_handle = (void*)&buffer_data->vk_buffer;
        }
    }

    void GraphicsContext::query_texture(TextureHandle texture, TextureDescription& out_description) {
        if (texture.index != k_invalid_index) {
            const Texture* texture_data = access_texture(texture);

            out_description.width = texture_data->width;
            out_description.height = texture_data->height;
            out_description.depth = texture_data->depth;
            out_description.format = texture_data->vk_format;
            out_description.mipmaps = texture_data->mipmaps;
            out_description.type = texture_data->type;
            out_description.render_target = (texture_data->flags & TextureFlags::RenderTarget_mask) == TextureFlags::RenderTarget_mask;
            out_description.compute_access = (texture_data->flags & TextureFlags::Compute_mask) == TextureFlags::Compute_mask;
            out_description.native_handle = (void*)&texture_data->vk_image;
            out_description.name = texture_data->name;
        }
    }

    void GraphicsContext::query_pipeline(PipelineHandle pipeline, PipelineDescription& out_description) {
        if (pipeline.index != k_invalid_index) {
            const Pipeline* pipeline_data = access_pipeline(pipeline);

            out_description.shader = pipeline_data->shader_state;
        }
    }

    void GraphicsContext::query_sampler(SamplerHandle sampler, SamplerDescription& out_description) {
        if (sampler.index != k_invalid_index) {
            const Sampler* sampler_data = access_sampler(sampler);

            out_description.address_mode_u = sampler_data->address_mode_u;
            out_description.address_mode_v = sampler_data->address_mode_v;
            out_description.address_mode_w = sampler_data->address_mode_w;

            out_description.min_filter = sampler_data->min_filter;
            out_description.mag_filter = sampler_data->mag_filter;
            out_description.mip_filter = sampler_data->mip_filter;

            out_description.name = sampler_data->name;
        }
    }

    void GraphicsContext::query_descriptor_set_layout(DescriptorSetLayoutHandle descriptor_set_layout, DescriptorSetLayoutDescription& out_description) {
        if (descriptor_set_layout.index != k_invalid_index) {
            const DescriptorSetLayout* descriptor_set_layout_data = access_descriptor_set_layout(descriptor_set_layout);

            const u32 num_bindings = descriptor_set_layout_data->num_bindings;
            for (size_t i = 0; i < num_bindings; i++) {
                out_description.bindings[i].name = descriptor_set_layout_data->bindings[i].name;
                out_description.bindings[i].type = descriptor_set_layout_data->bindings[i].type;
            }

            out_description.num_active_bindings = descriptor_set_layout_data->num_bindings;
        }
    }

    void GraphicsContext::query_descriptor_set(DescriptorSetHandle descriptor_set, DescriptorSetDescription& out_description) {
        if (descriptor_set.index != k_invalid_index) {
            const DescriptorSet* descriptor_set_data = access_descriptor_set(descriptor_set);

            out_description.num_active_resources = descriptor_set_data->num_resources;
            for (u32 i = 0; i < out_description.num_active_resources; ++i) {
                //out_description.resources[ i ].data = descriptor_set_data->resources[ i ].data;
            }
        }
    }

    const RenderPassOutput& GraphicsContext::get_render_pass_output(RenderPassHandle render_pass) const {
        const RenderPass* vulkan_render_pass = access_render_pass(render_pass);
        return vulkan_render_pass->output;
    }

    // Resource Map/Unmap ///////////////////////////////////////////////////////////


    void* GraphicsContext::map_buffer(const MapBufferParameters& parameters) {
        if (parameters.buffer.index == k_invalid_index)
            return nullptr;

        Buffer* buffer = access_buffer(parameters.buffer);

        if (buffer->parent_buffer.index == dynamic_buffer.index) {

            buffer->global_offset = dynamic_allocated_size;

            return dynamic_allocate(parameters.size == 0 ? buffer->size : parameters.size);
        }

        void* data;
        vmaMapMemory(vma_allocator, buffer->vma_allocation, &data);

        return data;
    }

    void GraphicsContext::unmap_buffer(const MapBufferParameters& parameters) {
        if (parameters.buffer.index == k_invalid_index)
            return;

        Buffer* buffer = access_buffer(parameters.buffer);
        if (buffer->parent_buffer.index == dynamic_buffer.index)
            return;

        vmaUnmapMemory(vma_allocator, buffer->vma_allocation);
    }

    void* GraphicsContext::dynamic_allocate(u32 size) {
        void* mapped_memory = dynamic_mapped_memory + dynamic_allocated_size;
        dynamic_allocated_size += (u32)Magnefu::memoryAlign(size, s_ubo_alignment);
        return mapped_memory;
    }

    void GraphicsContext::set_buffer_global_offset(BufferHandle buffer, u32 offset) {
        if (buffer.index == k_invalid_index)
            return;

        Buffer* vulkan_buffer = access_buffer(buffer);
        vulkan_buffer->global_offset = offset;
    }

    u32 GraphicsContext::get_gpu_timestamps(GPUTimestamp* out_timestamps) {
        return gpu_timestamp_manager->resolve(previous_frame, out_timestamps);

    }

    void GraphicsContext::push_gpu_timestamp(CommandBuffer* command_buffer, const char* name) {
        if (!timestamps_enabled)
            return;

        u32 query_index = gpu_timestamp_manager->push(current_frame, name);
        vkCmdWriteTimestamp(command_buffer->vk_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, vulkan_timestamp_query_pool, query_index);
    }

    void GraphicsContext::pop_gpu_timestamp(CommandBuffer* command_buffer) {
        if (!timestamps_enabled)
            return;

        u32 query_index = gpu_timestamp_manager->pop(current_frame);
        vkCmdWriteTimestamp(command_buffer->vk_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, vulkan_timestamp_query_pool, query_index);
    }

    void GraphicsContext::CalculateMemoryStats()
    {
        //vmaCalculateStatistics(m_VmaAllocator, &m_VulkanMemory.TotalStats);
        vmaGetHeapBudgets(vma_allocator, &vma_budget);
    }

    VMAMemoryStats GraphicsContext::GetMemoryStats()
    {
        CalculateMemoryStats();

        auto& vulkanStats = vma_budget.statistics;
        auto& vulkanBudget = vma_budget;
        VMAMemoryStats stats;

        stats.blockCount = vulkanStats.blockCount;
        stats.blockBytes = (uint64_t)vulkanStats.blockBytes;
        stats.allocationCount = vulkanStats.allocationCount;
        stats.allocationBytes = (uint64_t)vulkanStats.allocationBytes;
        stats.usage = vulkanBudget.usage;
        stats.budget = vulkanBudget.budget;

        return stats;
    }


    // Utility methods //////////////////////////////////////////////////////////////

    void check_result(VkResult result) {
        if (result == VK_SUCCESS) {
            return;
        }

        MF_CORE_ERROR("Vulkan error: code({})", result);
        if (result < 0) {
            MF_CORE_ASSERT(false, "Vulkan error: aborting.");
        }
    }
    // Device ////////////////////////////////////////////////////////////////

    BufferHandle GraphicsContext::get_fullscreen_vertex_buffer() const {
        return fullscreen_vertex_buffer;
    }

    RenderPassHandle GraphicsContext::get_swapchain_pass() const {
        return swapchain_pass;
    }

    TextureHandle GraphicsContext::get_dummy_texture() const {
        return dummy_texture;
    }

    BufferHandle GraphicsContext::get_dummy_constant_buffer() const {
        return dummy_constant_buffer;
    }

    void GraphicsContext::resize(u16 width, u16 height) {
        swapchain_width = width;
        swapchain_height = height;

        resized = true;
    }


    // Resource Access //////////////////////////////////////////////////////////////
    ShaderState* GraphicsContext::access_shader_state(ShaderStateHandle shader) {
        return (ShaderState*)shaders.access_resource(shader.index);
    }

    const ShaderState* GraphicsContext::access_shader_state(ShaderStateHandle shader) const {
        return (const ShaderState*)shaders.access_resource(shader.index);
    }

    Texture* GraphicsContext::access_texture(TextureHandle texture) 
    {
        return (Texture*)textures.access_resource(texture.index);
    }

    const Texture* GraphicsContext::access_texture(TextureHandle texture) const 
    {
        return (const Texture*)textures.access_resource(texture.index);
    }

    Buffer* GraphicsContext::access_buffer(BufferHandle buffer) {
        return (Buffer*)buffers.access_resource(buffer.index);
    }

    const Buffer* GraphicsContext::access_buffer(BufferHandle buffer) const {
        return (const Buffer*)buffers.access_resource(buffer.index);
    }

    Pipeline* GraphicsContext::access_pipeline(PipelineHandle pipeline) {
        return (Pipeline*)pipelines.access_resource(pipeline.index);
    }

    const Pipeline* GraphicsContext::access_pipeline(PipelineHandle pipeline) const {
        return (const Pipeline*)pipelines.access_resource(pipeline.index);
    }

    Sampler* GraphicsContext::access_sampler(SamplerHandle sampler) {
        return (Sampler*)samplers.access_resource(sampler.index);
    }

    const Sampler* GraphicsContext::access_sampler(SamplerHandle sampler) const {
        return (const Sampler*)samplers.access_resource(sampler.index);
    }

    DescriptorSetLayout* GraphicsContext::access_descriptor_set_layout(DescriptorSetLayoutHandle descriptor_set_layout) {
        return (DescriptorSetLayout*)descriptor_set_layouts.access_resource(descriptor_set_layout.index);
    }

    const DescriptorSetLayout* GraphicsContext::access_descriptor_set_layout(DescriptorSetLayoutHandle descriptor_set_layout) const {
        return (const DescriptorSetLayout*)descriptor_set_layouts.access_resource(descriptor_set_layout.index);
    }

    DescriptorSetLayoutHandle GraphicsContext::get_descriptor_set_layout(PipelineHandle pipeline_handle, int layout_index) {
        Pipeline* pipeline = access_pipeline(pipeline_handle);
        MF_CORE_ASSERT((pipeline != nullptr), "Pipeline doesnt' exist");

        return  pipeline->descriptor_set_layout_handle[layout_index];
    }

    DescriptorSetLayoutHandle GraphicsContext::get_descriptor_set_layout(PipelineHandle pipeline_handle, int layout_index) const {
        const Pipeline* pipeline = access_pipeline(pipeline_handle);
        MF_CORE_ASSERT((pipeline != nullptr), "Pipeline doesn't exist");

        return  pipeline->descriptor_set_layout_handle[layout_index];
    }

    DescriptorSet* GraphicsContext::access_descriptor_set(DescriptorSetHandle descriptor_set) {
        return (DescriptorSet*)descriptor_sets.access_resource(descriptor_set.index);
    }

    const DescriptorSet* GraphicsContext::access_descriptor_set(DescriptorSetHandle descriptor_set) const {
        return (const DescriptorSet*)descriptor_sets.access_resource(descriptor_set.index);
    }

    RenderPass* GraphicsContext::access_render_pass(RenderPassHandle render_pass) {
        return (RenderPass*)render_passes.access_resource(render_pass.index);
    }

    const RenderPass* GraphicsContext::access_render_pass(RenderPassHandle render_pass) const {
        return (const RenderPass*)render_passes.access_resource(render_pass.index);
    }
    /*
    // Building Helpers /////////////////////////////////////////////////////////////

    // SortKey //////////////////////////////////////////////////////////////////////
    static const u64                    k_stage_shift               = (56);

    u64 SortKey::get_key( u64 stage_index ) {
        return ( ( stage_index << k_stage_shift ) );
    }*/

    // GPU Timestamp Manager ////////////////////////////////////////////////////////

    void GPUTimestampManager::init(Allocator* allocator_, u16 queries_per_frame_, u16 max_frames) {

        allocator = allocator_;
        queries_per_frame = queries_per_frame_;

        // Data is start, end in 2 u64 numbers.
        const u32 k_data_per_query = 2;
        const sizet allocated_size = sizeof(GPUTimestamp) * queries_per_frame * max_frames + sizeof(u64) * queries_per_frame * max_frames * k_data_per_query;
        u8* memory = mfallocam(allocated_size, allocator);

        timestamps = (GPUTimestamp*)memory;
        // Data is start, end in 2 u64 numbers.
        timestamps_data = (u64*)(memory + sizeof(GPUTimestamp) * queries_per_frame * max_frames);

        reset();
    }

    void GPUTimestampManager::shutdown() {

        mffree(timestamps, allocator);
    }

    void GPUTimestampManager::reset() {
        current_query = 0;
        parent_index = 0;
        current_frame_resolved = false;
        depth = 0;
    }

    bool GPUTimestampManager::has_valid_queries() const {
        // Even number of queries means asymettrical queries, thus we don't sample.
        return current_query > 0 && (depth == 0);
    }

    u32 GPUTimestampManager::resolve(u32 current_frame, GPUTimestamp* timestamps_to_fill) {
        Magnefu::memoryCopy(timestamps_to_fill, &timestamps[current_frame * queries_per_frame], sizeof(GPUTimestamp) * current_query);
        return current_query;
    }

    u32 GPUTimestampManager::push(u32 current_frame, const char* name) {
        u32 query_index = (current_frame * queries_per_frame) + current_query;

        GPUTimestamp& timestamp = timestamps[query_index];
        timestamp.parent_index = (u16)parent_index;
        timestamp.start = query_index * 2;
        timestamp.end = timestamp.start + 1;
        timestamp.name = name;
        timestamp.depth = (u16)depth++;

        parent_index = current_query;
        ++current_query;

        return (query_index * 2);
    }

    u32 GPUTimestampManager::pop(u32 current_frame) {

        u32 query_index = (current_frame * queries_per_frame) + parent_index;
        GPUTimestamp& timestamp = timestamps[query_index];
        // Go up a level
        parent_index = timestamp.parent_index;
        --depth;

        return (query_index * 2) + 1;
    }

    DeviceCreation& DeviceCreation::set_window(u32 width_, u32 height_, void* handle) {
        width = (u16)width_;
        height = (u16)height_;
        window = handle;
        return *this;
    }

    DeviceCreation& DeviceCreation::set_allocator(Allocator* allocator_) {
        allocator = allocator_;
        return *this;
    }

    DeviceCreation& DeviceCreation::set_stack_allocator(StackAllocator* allocator) {
        temporary_allocator = allocator;
        return *this;
    }

    DeviceCreation& DeviceCreation::set_num_threads(u16 num_threads_)
    {
        num_threads = num_threads_;
        return *this;
    }

} // namespace raptor