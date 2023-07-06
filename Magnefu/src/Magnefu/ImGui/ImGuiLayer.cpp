#include "mfpch.h"
#include "ImGuiLayer.h"

#include "Magnefu/Application.h"
#include "Platform/Vulkan/VulkanContext.h"


#include "imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"

// Data
static VkAllocationCallbacks*   s_Allocator = nullptr;
static VkInstance               s_Instance = VK_NULL_HANDLE;
static VkPhysicalDevice         s_PhysicalDevice = VK_NULL_HANDLE;
static VkDevice                 s_Device = VK_NULL_HANDLE;
static uint32_t                 s_QueueFamily = (uint32_t)-1;
static VkQueue                  s_Queue = VK_NULL_HANDLE;
static VkPipelineCache          s_PipelineCache = VK_NULL_HANDLE;
static VkDescriptorPool         s_DescriptorPool = VK_NULL_HANDLE;
static ImGui_ImplVulkanH_Window s_MainWindowData;
static int                      s_MinImageCount = 2;
static bool                     s_SwapChainRebuild = false;

static ImVec4                   s_ClearColor = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);


static void SetupVulkan()
{
	// Create Descriptor Pool
	{
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		vkCreateDescriptorPool(s_Device, &pool_info, s_Allocator, &s_DescriptorPool);
	}
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
static void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height)
{
	wd->Surface = surface;

	// Select Surface Format
	const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
	const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(s_PhysicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

	// Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
	VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
	VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
	wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(s_PhysicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
	//printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

	// Create SwapChain, RenderPass, Framebuffer, etc.
	IM_ASSERT(s_MinImageCount >= 2);
	ImGui_ImplVulkanH_CreateOrResizeWindow(s_Instance, s_PhysicalDevice, s_Device, wd, s_QueueFamily, s_Allocator, width, height, s_MinImageCount);
}

static void CleanupVulkanWindow()
{
	ImGui_ImplVulkanH_DestroyWindow(s_Instance, s_Device, &s_MainWindowData, s_Allocator);
}

static void SetupImGuiVulkanPrimitives(ImGui_ImplVulkanH_Window* wd, Magnefu::VulkanContext* context)
{
	wd->ImageCount = s_MinImageCount;
	wd->SurfaceFormat = std::any_cast<VkSurfaceFormatKHR>(context->GetContextInfo("SurfaceFormat"));
	wd->Surface = std::any_cast<VkSurfaceKHR>(context->GetContextInfo("Surface"));
	wd->PresentMode = std::any_cast<VkPresentModeKHR>(context->GetContextInfo("PresentMode"));
	wd->Swapchain = std::any_cast<VkSwapchainKHR>(context->GetContextInfo("SwapChain"));
	VkExtent2D swapChainExtent = std::any_cast<VkExtent2D>(context->GetContextInfo("SwapChainExtent"));
	wd->Width = swapChainExtent.width;
	wd->Height = swapChainExtent.height;
	wd->FrameIndex = std::any_cast<uint32_t>(context->GetContextInfo("CurrentFrame"));
}

static void SetupImGuiVulkanFrames(ImGui_ImplVulkanH_Window* wd, Magnefu::VulkanContext* context)
{
	wd->Frames = nullptr;
	IM_ASSERT(wd->Frames == nullptr);
	wd->Frames = (ImGui_ImplVulkanH_Frame*)IM_ALLOC(sizeof(ImGui_ImplVulkanH_Frame) * wd->ImageCount);
	wd->FrameSemaphores = (ImGui_ImplVulkanH_FrameSemaphores*)IM_ALLOC(sizeof(ImGui_ImplVulkanH_FrameSemaphores) * wd->ImageCount);
	memset(wd->Frames, 0, sizeof(wd->Frames[0]) * wd->ImageCount);
	memset(wd->FrameSemaphores, 0, sizeof(wd->FrameSemaphores[0]) * wd->ImageCount);

	VkImage* swapChainImages = std::any_cast<VkImage*>(context->GetContextInfo("SwapChainImages"));
	VkCommandPool commandPool = std::any_cast<VkCommandPool>(context->GetContextInfo("CommandPool"));

	for (uint32_t i = 0; i < wd->ImageCount; i++)
	{
		wd->Frames[i].Backbuffer = swapChainImages[i];  // this is m_SwapChainImages.data() in VulkanContext.cpp so do a get contextinfo for swapchainimages
		wd->Frames[i].CommandPool = commandPool;
	}
}

static void CreateImageViews(ImGui_ImplVulkanH_Window* wd)
{
	VkImageViewCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	info.format = wd->SurfaceFormat.format;
	info.components.r = VK_COMPONENT_SWIZZLE_R;
	info.components.g = VK_COMPONENT_SWIZZLE_G;
	info.components.b = VK_COMPONENT_SWIZZLE_B;
	info.components.a = VK_COMPONENT_SWIZZLE_A;
	VkImageSubresourceRange image_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	info.subresourceRange = image_range;

	for (size_t i = 0; i < wd->ImageCount; i++)
	{
		ImGui_ImplVulkanH_Frame* fd = &wd->Frames[i];
		info.image = fd->Backbuffer;
		vkCreateImageView(s_Device, &info, s_Allocator, &fd->BackbufferView);

	}
}

static void CreateRenderPass(ImGui_ImplVulkanH_Window* wd)
{
	VkAttachmentDescription attachment = {};
	attachment.format = wd->SurfaceFormat.format;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment = {};
	color_attachment.attachment = 0;
	color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.attachmentCount = 1;
	info.pAttachments = &attachment;
	info.subpassCount = 1;
	info.pSubpasses = &subpass;
	info.dependencyCount = 1;
	info.pDependencies = &dependency;
	if (vkCreateRenderPass(s_Device, &info, s_Allocator, &wd->RenderPass) != VK_SUCCESS)
		MF_CORE_ASSERT(false, "Failed to create ImGui Render Pass");
}

static void CreateFrameBuffers(ImGui_ImplVulkanH_Window* wd)
{
	VkImageView attachment[1];
	VkFramebufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.renderPass = wd->RenderPass;
	info.attachmentCount = 1;
	info.pAttachments = attachment;
	info.width = wd->Width;
	info.height = wd->Height;
	info.layers = 1;
	for (uint32_t i = 0; i < wd->ImageCount; i++)
	{
		ImGui_ImplVulkanH_Frame* fd = &wd->Frames[i];
		attachment[0] = fd->BackbufferView;
		if (vkCreateFramebuffer(s_Device, &info, s_Allocator, &fd->Framebuffer) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to to create ImGui Framebuffers");
	}
}

static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data, VkSemaphore* waitSemaphores, VkSemaphore* signalSemaphores)
{
	ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];

	{
		vkResetCommandBuffer(fd->CommandBuffer, 0);
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		if (vkBeginCommandBuffer(fd->CommandBuffer, &info) != VK_SUCCESS)
			MF_CORE_ASSERT(false, "Failed to begin imgui command buffer");
	}
	{
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = wd->RenderPass;
		info.framebuffer = fd->Framebuffer;
		info.renderArea.extent.width = wd->Width;
		info.renderArea.extent.height = wd->Height;
		info.clearValueCount = 1;
		info.pClearValues = &wd->ClearValue;
		vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	// Record dear imgui primitives into command buffer
	ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

	// Submit command buffer
	vkCmdEndRenderPass(fd->CommandBuffer);
	{
		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = waitSemaphores;
		info.pWaitDstStageMask = &wait_stage;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &fd->CommandBuffer;
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = signalSemaphores;

		vkEndCommandBuffer(fd->CommandBuffer);
		vkQueueSubmit(s_Queue, 1, &info, fd->Fence);
	}
}


namespace Magnefu
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)
	ImGuiLayer::ImGuiLayer() :
		Layer("ImGuiLayer")
	{

	}

	ImGuiLayer::~ImGuiLayer()
	{

	}

	void ImGuiLayer::OnAttach()
	{

		// TODO: Make Code API Agnostic
		// Right now it is all specific to Vulkan
		// Probably use a switch on the renderer api

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
		VulkanContext* context = static_cast<VulkanContext*>(app.GetWindow().GetGraphicsContext());

		s_Instance = std::any_cast<VkInstance>(context->GetContextInfo("Instance"));
		s_Device = std::any_cast<VkDevice>(context->GetContextInfo("Device"));
		s_PhysicalDevice = std::any_cast<VkPhysicalDevice>(context->GetContextInfo("PhysicalDevice"));
		s_QueueFamily = std::any_cast<uint32_t>(context->GetContextInfo("QueueFamily"));
		s_Queue = std::any_cast<VkQueue>(context->GetContextInfo("Queue"));
		s_MinImageCount = std::any_cast<uint32_t>(context->GetContextInfo("ImageCount"));

		
		SetupVulkan();

		ImGui_ImplVulkanH_Window* wd = &s_MainWindowData;
		SetupImGuiVulkanPrimitives(wd, context);
		
		SetupImGuiVulkanFrames(wd, context);
		CreateImageViews(wd);
		CreateRenderPass(wd);
		CreateFrameBuffers(wd);
	

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		//float fontSize = 18.0f;// *2.0f;
		//io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Bold.ttf", fontSize);
		//io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Regular.ttf", fontSize);


		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		SetDarkThemeColors();

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForVulkan(window, true); // TODO: Make this based on an API switch
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = s_Instance;
		init_info.PhysicalDevice = s_PhysicalDevice;
		init_info.Device = s_Device;
		init_info.QueueFamily = s_QueueFamily;
		init_info.Queue = s_Queue;
		init_info.PipelineCache = s_PipelineCache;
		init_info.DescriptorPool = s_DescriptorPool;
		init_info.Subpass = 0;
		init_info.MinImageCount = s_MinImageCount;
		init_info.ImageCount = wd->ImageCount;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = s_Allocator;
		init_info.CheckVkResultFn = nullptr;

		// ImGui's Vulkan Render Pipeline created here
		ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

		// Upload Fonts
		{
			// Use any command queue
			VkCommandPool command_pool = std::any_cast<VkCommandPool>(context->GetContextInfo("CommandPool"));
			VkCommandBuffer command_buffer = std::any_cast<VkCommandBuffer>(context->GetContextInfo("ImGuiCommandBuffer"));

			vkResetCommandPool(s_Device, command_pool, 0);
			VkCommandBufferBeginInfo begin_info = {};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			vkBeginCommandBuffer(command_buffer, &begin_info);

			ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

			VkSubmitInfo end_info = {};
			end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			end_info.commandBufferCount = 1;
			end_info.pCommandBuffers = &command_buffer;
			vkEndCommandBuffer(command_buffer);
			vkQueueSubmit(s_Queue, 1, &end_info, VK_NULL_HANDLE);

			vkDeviceWaitIdle(s_Device);
			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}
	}

	void ImGuiLayer::OnDetach()
	{
		//vkDeviceWaitIdle(s_Device); // already running in m_Window->OnFinish();
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		CleanupVulkanWindow();
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.m_Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			e.m_Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
	}


	void ImGuiLayer::BeginFrame()
	{
		Application& app = Application::Get();
		
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

	}

	void ImGuiLayer::EndFrame()
	{
		ImGui::EndFrame();
		ImGui::Render();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* contextBackup = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(contextBackup);
		}

	}

	void ImGuiLayer::RecreateImageResources()
	{
		Application& app = Application::Get();
		VulkanContext* context = static_cast<VulkanContext*>(app.GetWindow().GetGraphicsContext());

		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
		s_MinImageCount = std::any_cast<uint32_t>(context->GetContextInfo("ImageCount"));

		ImGui_ImplVulkanH_Window* wd = &s_MainWindowData;
		SetupImGuiVulkanPrimitives(wd, context);

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		if (width > 0 && height > 0)
		{
			SetupImGuiVulkanFrames(wd, context);
			CreateImageViews(wd);
			CreateFrameBuffers(wd);
		}
	}

	void ImGuiLayer::RecordAndSubmitCommandBuffer(uint32_t imageIndex)
	{
		Application& app = Application::Get();
		VulkanContext* context = static_cast<VulkanContext*>(app.GetWindow().GetGraphicsContext());

		ImDrawData* draw_data = ImGui::GetDrawData();
		const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
		ImGui_ImplVulkanH_Window* wd = &s_MainWindowData;

		wd->FrameIndex = imageIndex;  // NEEDS TO BE THE IMAGE INDEX, NOT CURRENT FRAME

		if (!is_minimized)
		{
			wd->ClearValue.color.float32[0] = s_ClearColor.x * s_ClearColor.w;
			wd->ClearValue.color.float32[1] = s_ClearColor.y * s_ClearColor.w;
			wd->ClearValue.color.float32[2] = s_ClearColor.z * s_ClearColor.w;
			wd->ClearValue.color.float32[3] = s_ClearColor.w;

			// NEED TO BE SURE TO PROVIDE THESE VALUES SPECIFICALLY AS THEY MATCH THE NUMBE OF FRAMES IN FLIGHT, NOT THE IMAGE COUNT
			// FRAMES IN FLIGHT -> 2
			// IMAGE COUNT -> 3
			wd->Frames[imageIndex].Fence = std::any_cast<VkFence>(context->GetContextInfo("ImGuiInFlightFence"));
			wd->Frames[imageIndex].CommandBuffer = std::any_cast<VkCommandBuffer>(context->GetContextInfo("ImGuiCommandBuffer"));

			// get wait and signal semaphores and send them to frame render
			VkSemaphore* wait = std::any_cast<VkSemaphore*>(context->GetContextInfo("ImGuiWaitSemaphores"));
			VkSemaphore* signal = std::any_cast<VkSemaphore*>(context->GetContextInfo("ImGuiSignalSemaphores"));
			

			FrameRender(wd, draw_data, wait, signal);
		}
	}

	void ImGuiLayer::OnRender()
	{
		
	}

	void ImGuiLayer::SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.6f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.3f, 0.305f, 1.f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

	}

	uint32_t ImGuiLayer::GetActiveWidgetID() const
	{
		//return GImGui->ActiveId;
		return 0;
	}

}