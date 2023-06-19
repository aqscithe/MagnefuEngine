#include "mfpch.h"
#include "ImGuiLayer.h"

#include "Magnefu/Application.h"
#include "Platform/VK/VKContext.h"


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

static ImVec4                   s_ClearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


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
	//const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
	//const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	//wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(s_PhysicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

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

static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data)
{
	VkResult err;

	VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
	VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
	err = vkAcquireNextImageKHR(s_Device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
	{
		s_SwapChainRebuild = true;
		return;
	}

	ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
	{
		vkWaitForFences(s_Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking

		vkResetFences(s_Device, 1, &fd->Fence);
	}
	{
		vkResetCommandPool(s_Device, fd->CommandPool, 0);
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
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
		info.pWaitSemaphores = &image_acquired_semaphore;
		info.pWaitDstStageMask = &wait_stage;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &fd->CommandBuffer;
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = &render_complete_semaphore;

		vkEndCommandBuffer(fd->CommandBuffer);
		vkQueueSubmit(s_Queue, 1, &info, fd->Fence);
	}
}

static void FramePresent(ImGui_ImplVulkanH_Window* wd)
{
	if (s_SwapChainRebuild)
		return;
	VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
	VkPresentInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info.waitSemaphoreCount = 1;
	info.pWaitSemaphores = &render_complete_semaphore;
	info.swapchainCount = 1;
	info.pSwapchains = &wd->Swapchain;
	info.pImageIndices = &wd->FrameIndex;
	VkResult err = vkQueuePresentKHR(s_Queue, &info);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
	{
		s_SwapChainRebuild = true;
		return;
	}

	wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount; // Now we can use the next set of semaphores
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
		VKContext* context = static_cast<VKContext*>(app.GetWindow().GetGraphicsContext());

		VkSurfaceKHR surface = std::any_cast<VkSurfaceKHR>(context->GetContextInfo("Surface"));
		s_Instance = std::any_cast<VkInstance>(context->GetContextInfo("Instance"));
		s_Device = std::any_cast<VkDevice>(context->GetContextInfo("Device"));
		s_PhysicalDevice = std::any_cast<VkPhysicalDevice>(context->GetContextInfo("PhysicalDevice"));
		s_QueueFamily = std::any_cast<uint32_t>(context->GetContextInfo("QueueFamily"));
		s_Queue = std::any_cast<VkQueue>(context->GetContextInfo("Queue"));
		s_MinImageCount = std::any_cast<uint32_t>(context->GetContextInfo("ImageCount"));

		SetupVulkan();

		// Create Framebuffers
		int w, h;
		glfwGetFramebufferSize(window, &w, &h);
		ImGui_ImplVulkanH_Window* wd = &s_MainWindowData;
		s_MainWindowData.SurfaceFormat = std::any_cast<VkSurfaceFormatKHR>(context->GetContextInfo("SurfaceFormat"));
		SetupVulkanWindow(wd, surface, w, h);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
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
		ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

		// Upload Fonts
		{
			// Use any command queue
			VkCommandPool command_pool = wd->Frames[wd->FrameIndex].CommandPool;
			VkCommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;

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
		// Check if Swap Chain needs rebuilding(screen size has changed)
		Application& app = Application::Get();
		VKContext* context = static_cast<VKContext*>(app.GetWindow().GetGraphicsContext());
		//s_SwapChainRebuild = std::any_cast<bool>(context->GetContextInfo("SwapChainRebuild"));

		if (s_SwapChainRebuild)
		{
			GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
			s_MinImageCount = std::any_cast<uint32_t>(context->GetContextInfo("ImageCount"));

			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			if (width > 0 && height > 0)
			{
				ImGui_ImplVulkan_SetMinImageCount(s_MinImageCount);
				ImGui_ImplVulkanH_CreateOrResizeWindow(s_Instance, s_PhysicalDevice, s_Device, &s_MainWindowData, s_QueueFamily, s_Allocator, width, height, s_MinImageCount);
				s_MainWindowData.FrameIndex = 0;
				s_SwapChainRebuild = false;
			}
		}
		
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

	}

	void ImGuiLayer::EndFrame()
	{
		ImGui::Render();
		
	}

	void ImGuiLayer::OnRender()
	{
		ImDrawData* draw_data = ImGui::GetDrawData();
		const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
		ImGui_ImplVulkanH_Window* wd = &s_MainWindowData;
		if (!is_minimized)
		{
			wd->ClearValue.color.float32[0] = s_ClearColor.x * s_ClearColor.w;
			wd->ClearValue.color.float32[1] = s_ClearColor.y * s_ClearColor.w;
			wd->ClearValue.color.float32[2] = s_ClearColor.z * s_ClearColor.w;
			wd->ClearValue.color.float32[3] = s_ClearColor.w;
			FrameRender(wd, draw_data);
			FramePresent(wd);
		}

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* contextBackup = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(contextBackup);
		}
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