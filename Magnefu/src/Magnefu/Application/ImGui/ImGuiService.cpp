
// -- PCH -- //
#include "mfpch.h"

// -- HEADER -- //
#include "ImGuiService.hpp"

// -- Application Includes ---------//
#include "Magnefu/Application/Application.h"

// -- Graphics Includes --------------------- //
#include "Magnefu/Graphics/GraphicsContext.h"
#include "Magnefu/Graphics/CommandBuffer.hpp"

// -- Core Includes ---------------------------------- //


// -- vendor Includes ------------------------------ //
#include "imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"






// Data
//static VkAllocationCallbacks*   s_Allocator = nullptr;
//static VkInstance               s_Instance = VK_NULL_HANDLE;
//static VkPhysicalDevice         s_PhysicalDevice = VK_NULL_HANDLE;
//static VkDevice                 s_Device = VK_NULL_HANDLE;
//static uint32_t                 s_QueueFamily = (uint32_t)-1;
//static VkQueue                  s_Queue = VK_NULL_HANDLE;
//static VkPipelineCache          s_PipelineCache = VK_NULL_HANDLE;
//static VkDescriptorPool         s_DescriptorPool = VK_NULL_HANDLE;
//static ImGui_ImplVulkanH_Window s_MainWindowData;
//static int                      s_MinImageCount = 2;
//static bool                     s_SwapChainRebuild = false;
//
//static ImVec4                   s_ClearColor = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
//
//
//static void SetupVulkan()
//{
//	// Create Descriptor Pool
//	{
//		VkDescriptorPoolSize pool_sizes[] =
//		{
//			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
//			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
//			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
//			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
//			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
//			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
//			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
//			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
//			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
//			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
//			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
//		};
//
//		VkDescriptorPoolCreateInfo pool_info = {};
//		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
//		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
//		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
//		pool_info.pPoolSizes = pool_sizes;
//		vkCreateDescriptorPool(s_Device, &pool_info, s_Allocator, &s_DescriptorPool);
//	}
//}
//
//// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
//// Your real engine/app may not use them.
//static void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height)
//{
//	wd->Surface = surface;
//
//	// Select Surface Format
//	const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
//	const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
//	wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(s_PhysicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);
//
//	// Select Present Mode
//#ifdef IMGUI_UNLIMITED_FRAME_RATE
//	VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
//#else
//	VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
//#endif
//	wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(s_PhysicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
//	//printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);
//
//	// Create SwapChain, RenderPass, Framebuffer, etc.
//	IM_ASSERT(s_MinImageCount >= 2);
//	ImGui_ImplVulkanH_CreateOrResizeWindow(s_Instance, s_PhysicalDevice, s_Device, wd, s_QueueFamily, s_Allocator, width, height, s_MinImageCount);
//}
//
//static void CleanupVulkanWindow()
//{
//	ImGui_ImplVulkanH_DestroyWindow(s_Instance, s_Device, &s_MainWindowData, s_Allocator);
//}
//
//static void SetupImGuiVulkanPrimitives(ImGui_ImplVulkanH_Window* wd, Magnefu::GraphicsContext* gpu)
//{
//	wd->ImageCount = s_MinImageCount;
//	wd->SurfaceFormat = gpu->vulkan_surface_format;
//	wd->Surface = gpu->vulkan_window_surface;
//	wd->PresentMode = gpu->vulkan_present_mode;
//	wd->Swapchain = gpu->vulkan_swapchain;
//	wd->Width = gpu->swapchain_width;
//	wd->Height = gpu->swapchain_height;
//	wd->FrameIndex = gpu->current_frame;
//}
//
//static void SetupImGuiVulkanFrames(ImGui_ImplVulkanH_Window* wd, Magnefu::GraphicsContext* gpu)
//{
//	wd->Frames = nullptr;
//	IM_ASSERT(wd->Frames == nullptr);
//	wd->Frames = (ImGui_ImplVulkanH_Frame*)IM_ALLOC(sizeof(ImGui_ImplVulkanH_Frame) * wd->ImageCount);
//	wd->FrameSemaphores = (ImGui_ImplVulkanH_FrameSemaphores*)IM_ALLOC(sizeof(ImGui_ImplVulkanH_FrameSemaphores) * wd->ImageCount);
//	memset(wd->Frames, 0, sizeof(wd->Frames[0]) * wd->ImageCount);
//	memset(wd->FrameSemaphores, 0, sizeof(wd->FrameSemaphores[0]) * wd->ImageCount);
//
//	VkImage* swapChainImages = gpu->vulkan_swapchain_images;
//	VkCommandPool commandPool = gpu->;
//
//	for (uint32_t i = 0; i < wd->ImageCount; i++)
//	{
//		wd->Frames[i].Backbuffer = swapChainImages[i];  // this is m_SwapChainImages.data() in VulkanContext.cpp so do a get contextinfo for swapchainimages
//		wd->Frames[i].CommandPool = commandPool;
//	}
//}
//
//static void CreateImageViews(ImGui_ImplVulkanH_Window* wd)
//{
//	VkImageViewCreateInfo info = {};
//	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//	info.viewType = VK_IMAGE_VIEW_TYPE_2D;
//	info.format = wd->SurfaceFormat.format;
//	info.components.r = VK_COMPONENT_SWIZZLE_R;
//	info.components.g = VK_COMPONENT_SWIZZLE_G;
//	info.components.b = VK_COMPONENT_SWIZZLE_B;
//	info.components.a = VK_COMPONENT_SWIZZLE_A;
//	VkImageSubresourceRange image_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
//	info.subresourceRange = image_range;
//
//	for (size_t i = 0; i < wd->ImageCount; i++)
//	{
//		ImGui_ImplVulkanH_Frame* fd = &wd->Frames[i];
//		info.image = fd->Backbuffer;
//		vkCreateImageView(s_Device, &info, s_Allocator, &fd->BackbufferView);
//
//	}
//}
//
//static void CreateRenderPass(ImGui_ImplVulkanH_Window* wd)
//{
//	VkAttachmentDescription attachment = {};
//	attachment.format = wd->SurfaceFormat.format;
//	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
//	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
//	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//	attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//
//	VkAttachmentReference color_attachment = {};
//	color_attachment.attachment = 0;
//	color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//	VkSubpassDescription subpass = {};
//	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//	subpass.colorAttachmentCount = 1;
//	subpass.pColorAttachments = &color_attachment;
//
//	VkSubpassDependency dependency = {};
//	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//	dependency.dstSubpass = 0;
//	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	dependency.srcAccessMask = 0;
//	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//
//	VkRenderPassCreateInfo info = {};
//	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//	info.attachmentCount = 1;
//	info.pAttachments = &attachment;
//	info.subpassCount = 1;
//	info.pSubpasses = &subpass;
//	info.dependencyCount = 1;
//	info.pDependencies = &dependency;
//	if (vkCreateRenderPass(s_Device, &info, s_Allocator, &wd->RenderPass) != VK_SUCCESS)
//		MF_CORE_ASSERT(false, "Failed to create ImGui Render Pass");
//}
//
//static void CreateFrameBuffers(ImGui_ImplVulkanH_Window* wd)
//{
//	VkImageView attachment[1];
//	VkFramebufferCreateInfo info = {};
//	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//	info.renderPass = wd->RenderPass;
//	info.attachmentCount = 1;
//	info.pAttachments = attachment;
//	info.width = wd->Width;
//	info.height = wd->Height;
//	info.layers = 1;
//	for (uint32_t i = 0; i < wd->ImageCount; i++)
//	{
//		ImGui_ImplVulkanH_Frame* fd = &wd->Frames[i];
//		attachment[0] = fd->BackbufferView;
//		if (vkCreateFramebuffer(s_Device, &info, s_Allocator, &fd->Framebuffer) != VK_SUCCESS)
//			MF_CORE_ASSERT(false, "Failed to to create ImGui Framebuffers");
//	}
//}
//
//static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data, VkSemaphore* waitSemaphores, VkSemaphore* signalSemaphores)
//{
//	ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
//
//	{
//		vkResetCommandBuffer(fd->CommandBuffer, 0);
//		VkCommandBufferBeginInfo info = {};
//		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//		if (vkBeginCommandBuffer(fd->CommandBuffer, &info) != VK_SUCCESS)
//			MF_CORE_ASSERT(false, "Failed to begin imgui command buffer");
//	}
//	{
//		VkRenderPassBeginInfo info = {};
//		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//		info.renderPass = wd->RenderPass;
//		info.framebuffer = fd->Framebuffer;
//		info.renderArea.extent.width = wd->Width;
//		info.renderArea.extent.height = wd->Height;
//		info.clearValueCount = 1;
//		info.pClearValues = &wd->ClearValue;
//		vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
//	}
//
//	// Record dear imgui primitives into command buffer
//	ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);
//
//	// Submit command buffer
//	vkCmdEndRenderPass(fd->CommandBuffer);
//	{
//		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//		VkSubmitInfo info = {};
//		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//		info.waitSemaphoreCount = 1;
//		info.pWaitSemaphores = waitSemaphores;
//		info.pWaitDstStageMask = &wait_stage;
//		info.commandBufferCount = 1;
//		info.pCommandBuffers = &fd->CommandBuffer;
//		info.signalSemaphoreCount = 1;
//		info.pSignalSemaphores = signalSemaphores;
//
//		vkEndCommandBuffer(fd->CommandBuffer);
//		vkQueueSubmit(s_Queue, 1, &info, fd->Fence);
//	}
//}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Graphics Data
static Magnefu::TextureHandle g_font_texture;
static Magnefu::PipelineHandle g_imgui_pipeline;
static Magnefu::BufferHandle g_vb, g_ib;
static Magnefu::BufferHandle g_ui_cb;
static Magnefu::DescriptorSetLayoutHandle g_descriptor_set_layout;
static Magnefu::DescriptorSetHandle g_ui_descriptor_set;  // Font descriptor set

static uint32_t g_vb_size = 665536, g_ib_size = 665536;

Magnefu::FlatHashMap<Magnefu::ResourceHandle, Magnefu::ResourceHandle> g_texture_to_descriptor_set;


static const char* g_vertex_shader_code = {
	"#version 450\n"
	"layout( location = 0 ) in vec2 Position;\n"
	"layout( location = 1 ) in vec2 UV;\n"
	"layout( location = 2 ) in uvec4 Color;\n"
	"layout( location = 0 ) out vec2 Frag_UV;\n"
	"layout( location = 1 ) out vec4 Frag_Color;\n"
	"layout( std140, binding = 0 ) uniform LocalConstants { mat4 ProjMtx; };\n"
	"void main()\n"
	"{\n"
	"    Frag_UV = UV;\n"
	"    Frag_Color = Color / 255.0f;\n"
	"    gl_Position = ProjMtx * vec4( Position.xy,0,1 );\n"
	"}\n"
};

static const char* g_vertex_shader_code_bindless = {
	"#version 450\n"
	"layout( location = 0 ) in vec2 Position;\n"
	"layout( location = 1 ) in vec2 UV;\n"
	"layout( location = 2 ) in uvec4 Color;\n"
	"layout( location = 0 ) out vec2 Frag_UV;\n"
	"layout( location = 1 ) out vec4 Frag_Color;\n"
	"layout (location = 2) flat out uint texture_id;\n"
	"layout( std140, binding = 0 ) uniform LocalConstants { mat4 ProjMtx; };\n"
	"void main()\n"
	"{\n"
	"    Frag_UV = UV;\n"
	"    Frag_Color = Color / 255.0f;\n"
	"    texture_id = gl_InstanceIndex;\n"
	"    gl_Position = ProjMtx * vec4( Position.xy,0,1 );\n"
	"}\n"
};

static const char* g_fragment_shader_code = {
	"#version 450\n"
	"#extension GL_EXT_nonuniform_qualifier : enable\n"
	"layout (location = 0) in vec2 Frag_UV;\n"
	"layout (location = 1) in vec4 Frag_Color;\n"
	"layout (location = 0) out vec4 Out_Color;\n"
	"layout (binding = 1) uniform sampler2D Texture;\n"
	"void main()\n"
	"{\n"
	"    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
	"}\n"
};

static const char* g_fragment_shader_code_bindless = {
	"#version 450\n"
	"#extension GL_EXT_nonuniform_qualifier : enable\n"
	"layout (location = 0) in vec2 Frag_UV;\n"
	"layout (location = 1) in vec4 Frag_Color;\n"
	"layout (location = 2) flat in uint texture_id;\n"
	"layout (location = 0) out vec4 Out_Color;\n"
	"#extension GL_EXT_nonuniform_qualifier : enable\n"
	"layout (set = 1, binding = 10) uniform sampler2D textures[];\n"
	"void main()\n"
	"{\n"
	"    Out_Color = Frag_Color * texture(textures[nonuniformEXT(texture_id)], Frag_UV.st);\n"
	"}\n"
};


static Magnefu::ImGuiService s_imgui_service;

Magnefu::ImGuiService* Magnefu::ImGuiService::Instance() 
{
	return &s_imgui_service;
}


namespace Magnefu
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)
	ImGuiService::ImGuiService() :
		Layer("ImGuiService")
	{

	}

	ImGuiService::~ImGuiService()
	{

	}



	void ImGuiService::Init(void* configuration)
	{
		ImGuiServiceConfiguration* imgui_config = (ImGuiServiceConfiguration*)configuration;
		gpu = imgui_config->gpu;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();


		// Setup Platform/Renderer backends
		//%%%
		ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)imgui_config->window_handle, true); // TODO: Make this based on an API switch
		/*ImGui_ImplVulkan_InitInfo init_info = {};
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
		init_info.CheckVkResultFn = nullptr;*/

		// ImGui's Vulkan Render Pipeline created here
		//ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);
		//%%%

		//%%%
		ImGuiIO& io = ImGui::GetIO();
		io.BackendRendererName = "Magnefu_ImGui";
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		//float fontSize = 18.0f;// *2.0f;
		//io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Bold.ttf", fontSize);
		//io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Regular.ttf", fontSize);
		// 
		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		//SetDarkThemeColors();
		//%%%

		using namespace Magnefu;

		// Load font texture atlas //////////////////////////////////////////////////
		unsigned char* pixels;
		int width, height;
		// Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be
		// compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id,
		// consider calling GetTexDataAsAlpha8() instead to save on GPU memory.
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		TextureCreation texture_creation;// = { pixels, ( u16 )width, ( u16 )height, 1, 1, 0, TextureFormat::R8G8B8A8_UNORM, TextureType::Texture2D };
		texture_creation.set_format_type(VK_FORMAT_R8G8B8A8_UNORM, TextureType::Texture2D).set_data(pixels).set_size(width, height, 1).set_flags(1, 0).set_name("ImGui_Font");
		g_font_texture = gpu->create_texture(texture_creation);

		// Store our identifier
		io.Fonts->TexID = (ImTextureID)&g_font_texture;

		// Manual code. Used to remove dependency from that.
		ShaderStateCreation shader_creation{};

		if (gpu->bindless_supported) {
			shader_creation.set_name("ImGui").add_stage(g_vertex_shader_code_bindless, (uint32_t)strlen(g_vertex_shader_code_bindless), VK_SHADER_STAGE_VERTEX_BIT)
				.add_stage(g_fragment_shader_code_bindless, (uint32_t)strlen(g_fragment_shader_code_bindless), VK_SHADER_STAGE_FRAGMENT_BIT);
		}
		else {
			shader_creation.set_name("ImGui").add_stage(g_vertex_shader_code, (uint32_t)strlen(g_vertex_shader_code), VK_SHADER_STAGE_VERTEX_BIT)
				.add_stage(g_fragment_shader_code, (uint32_t)strlen(g_fragment_shader_code), VK_SHADER_STAGE_FRAGMENT_BIT);
		}


		PipelineCreation pipeline_creation = {};
		pipeline_creation.name = "Pipeline_ImGui";
		pipeline_creation.shaders = shader_creation;

		pipeline_creation.blend_state.add_blend_state().set_color(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD);

		pipeline_creation.vertex_input.add_vertex_attribute({ 0, 0, 0, VertexComponentFormat::Float2 })
			.add_vertex_attribute({ 1, 0, 8, VertexComponentFormat::Float2 })
			.add_vertex_attribute({ 2, 0, 16, VertexComponentFormat::UByte4N });

		pipeline_creation.vertex_input.add_vertex_stream({ 0, 20, VertexInputRate::PerVertex });
		pipeline_creation.render_pass = gpu->get_swapchain_output();

		DescriptorSetLayoutCreation descriptor_set_layout_creation{};
		if (gpu->bindless_supported) {
			descriptor_set_layout_creation.add_binding({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1, "LocalConstants" }).add_binding({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10, 1, "Texture" }).set_name("RLL_ImGui");
		}
		else {
			descriptor_set_layout_creation.add_binding({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1, "LocalConstants" }).set_name("RLL_ImGui");
			descriptor_set_layout_creation.add_binding({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, 1, "LocalConstants" }).set_name("RLL_ImGui");
		}


		g_descriptor_set_layout = gpu->create_descriptor_set_layout(descriptor_set_layout_creation);

		pipeline_creation.add_descriptor_set_layout(g_descriptor_set_layout);

		g_imgui_pipeline = gpu->create_pipeline(pipeline_creation);

		// Create constant buffer
		BufferCreation cb_creation;
		cb_creation.set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, ResourceUsageType::Dynamic, 64).set_name("CB_ImGui");
		g_ui_cb = gpu->create_buffer(cb_creation);

		// Create descriptor set
		DescriptorSetCreation ds_creation{};
		if (!gpu->bindless_supported) {
			ds_creation.set_layout(pipeline_creation.descriptor_set_layout[0]).buffer(g_ui_cb, 0).texture(g_font_texture, 1).set_name("RL_ImGui");
		}
		else {
			ds_creation.set_layout(pipeline_creation.descriptor_set_layout[0]).buffer(g_ui_cb, 0).set_name("RL_ImGui");
		}
		g_ui_descriptor_set = gpu->create_descriptor_set(ds_creation);

		// Add descriptor set to the map
		// Old Map
		g_texture_to_descriptor_set.init(&MemoryService::Instance()->systemAllocator, 4);
		g_texture_to_descriptor_set.insert(g_font_texture.index, g_ui_descriptor_set.index);

		// Create vertex and index buffers //////////////////////////////////////////
		BufferCreation vb_creation;
		vb_creation.set(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, ResourceUsageType::Dynamic, g_vb_size).set_name("VB_ImGui");
		g_vb = gpu->create_buffer(vb_creation);

		BufferCreation ib_creation;
		ib_creation.set(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, ResourceUsageType::Dynamic, g_ib_size).set_name("IB_ImGui");
		g_ib = gpu->create_buffer(ib_creation);
	}

	void ImGuiService::Shutdown() 
	{

		FlatHashMapIterator it = g_texture_to_descriptor_set.iterator_begin();
		while (it.is_valid()) 
		{
			Magnefu::ResourceHandle handle = g_texture_to_descriptor_set.get(it);
			gpu->destroy_descriptor_set({ handle });

			g_texture_to_descriptor_set.iterator_advance(it);
		}

		g_texture_to_descriptor_set.shutdown();

		gpu->destroy_buffer(g_vb);
		gpu->destroy_buffer(g_ib);
		gpu->destroy_buffer(g_ui_cb);
		gpu->destroy_descriptor_set_layout(g_descriptor_set_layout);

		gpu->destroy_pipeline(g_imgui_pipeline);
		gpu->destroy_texture(g_font_texture);


		//ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiService::BeginFrame() 
	{
		// this part may be unnecessary
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)gpu->swapchain_width, (float)gpu->swapchain_height);

		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiService::Render(CommandBuffer& commands) 
	{
		//ImGui::EndFrame();// Called by ImGui::Render()
		ImGui::Render();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* contextBackup = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(contextBackup);
		}

		ImDrawData* draw_data = ImGui::GetDrawData();

		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
		int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
		if (fb_width <= 0 || fb_height <= 0)
			return;

		// Vulkan backend has a different origin than OpenGL.
		bool clip_origin_lower_left = false;

#if defined(GL_CLIP_ORIGIN) && !defined(__APPLE__)
		GLenum last_clip_origin = 0; glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)&last_clip_origin); // Support for GL 4.5's glClipControl(GL_UPPER_LEFT)
		if (last_clip_origin == GL_UPPER_LEFT)
			clip_origin_lower_left = false;
#endif
		size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
		size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);

		if (vertex_size >= g_vb_size || index_size >= g_ib_size) 
		{
			MF_CORE_ERROR("ImGui Backend Error: vertex/index overflow!");
			return;
		}

		if (vertex_size == 0 && index_size == 0) 
		{
			return;
		}

		using namespace Magnefu;

		// Upload data
		ImDrawVert* vtx_dst = NULL;
		ImDrawIdx* idx_dst = NULL;

		MapBufferParameters map_parameters_vb = { g_vb, 0, (u32)vertex_size };
		vtx_dst = (ImDrawVert*)gpu->map_buffer(map_parameters_vb);

		if (vtx_dst) 
		{
			for (int n = 0; n < draw_data->CmdListsCount; n++) 
			{

				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
				vtx_dst += cmd_list->VtxBuffer.Size;
			}

			gpu->unmap_buffer(map_parameters_vb);
		}

		MapBufferParameters map_parameters_ib = { g_ib, 0, (u32)index_size };
		idx_dst = (ImDrawIdx*)gpu->map_buffer(map_parameters_ib);

		if (idx_dst)
		{
			for (int n = 0; n < draw_data->CmdListsCount; n++) 
			{

				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
				idx_dst += cmd_list->IdxBuffer.Size;
			}

			gpu->unmap_buffer(map_parameters_ib);
		}

		// TODO_KS: Add the sorting.
		commands.push_marker("ImGUI");

		// todo: key
		commands.bind_pass(gpu->get_swapchain_pass());
		commands.bind_pipeline(g_imgui_pipeline);
		commands.bind_vertex_buffer(g_vb, 0, 0);
		commands.bind_index_buffer(g_ib, 0, VK_INDEX_TYPE_UINT16);

		const Viewport viewport = { 0, 0, (u16)fb_width, (u16)fb_height, 0.0f, 1.0f };
		commands.set_viewport(&viewport);

		// Setup viewport, orthographic projection matrix
		// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin is typically (0,0) for single viewport apps.
		float L = draw_data->DisplayPos.x;
		float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
		float T = draw_data->DisplayPos.y;
		float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
		const float ortho_projection[4][4] =
		{
			{ 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
			{ 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
			{ 0.0f,         0.0f,        -1.0f,   0.0f },
			{ (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
		};

		MapBufferParameters cb_map = { g_ui_cb, 0, 0 };
		float* cb_data = (float*)gpu->map_buffer(cb_map);
		if (cb_data) {
			memcpy(cb_data, &ortho_projection[0][0], 64);
			gpu->unmap_buffer(cb_map);
		}

		// Will project scissor/clipping rectangles into framebuffer space
		ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
		ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		// Render command lists
		//
		int counts = draw_data->CmdListsCount;

		TextureHandle last_texture = g_font_texture;
		// todo:map
		DescriptorSetHandle last_descriptor_set = { g_texture_to_descriptor_set.get(last_texture.index) };

		commands.bind_descriptor_set(&last_descriptor_set, 1, nullptr, 0);

		uint32_t vtx_buffer_offset = 0, index_buffer_offset = 0;
		for (int n = 0; n < counts; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];

			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback)
				{
					// User callback (registered via ImDrawList::AddCallback)
					pcmd->UserCallback(cmd_list, pcmd);
				}
				else
				{
					// Project scissor/clipping rectangles into framebuffer space
					ImVec4 clip_rect;
					clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
					clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
					clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
					clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

					if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
					{
						// Apply scissor/clipping rectangle
						if (clip_origin_lower_left) {
							Rect2DInt scissor_rect = { (int16_t)clip_rect.x, (int16_t)(fb_height - clip_rect.w), (uint16_t)(clip_rect.z - clip_rect.x), (uint16_t)(clip_rect.w - clip_rect.y) };
							commands.set_scissor(&scissor_rect);
						}
						else {
							Rect2DInt scissor_rect = { (int16_t)clip_rect.x, (int16_t)clip_rect.y, (uint16_t)(clip_rect.z - clip_rect.x), (uint16_t)(clip_rect.w - clip_rect.y) };
							commands.set_scissor(&scissor_rect);
						}

						// Retrieve
						TextureHandle new_texture = *(TextureHandle*)(pcmd->TextureId);
						if (!gpu->bindless_supported) {
							if (new_texture.index != last_texture.index && new_texture.index != k_invalid_texture.index) {
								last_texture = new_texture;
								FlatHashMapIterator it = g_texture_to_descriptor_set.find(last_texture.index);

								// TODO: invalidate handles and update descriptor set when needed ?
								// Found this problem when reusing the handle from a previous
								// If not present
								if (it.is_invalid()) {
									// Create new descriptor set
									DescriptorSetCreation ds_creation{};

									ds_creation.set_layout(g_descriptor_set_layout).buffer(g_ui_cb, 0).texture(last_texture, 1).set_name("RL_Dynamic_ImGUI");
									last_descriptor_set = gpu->create_descriptor_set(ds_creation);

									g_texture_to_descriptor_set.insert(new_texture.index, last_descriptor_set.index);
								}
								else {
									last_descriptor_set.index = g_texture_to_descriptor_set.get(it);
								}
								commands.bind_descriptor_set(&last_descriptor_set, 1, nullptr, 0);
							}
						}

						commands.draw_indexed(Magnefu::TopologyType::Triangle, pcmd->ElemCount, 1, index_buffer_offset + pcmd->IdxOffset, vtx_buffer_offset + pcmd->VtxOffset, new_texture.index);
					}
				}

			}
			index_buffer_offset += cmd_list->IdxBuffer.Size;
			vtx_buffer_offset += cmd_list->VtxBuffer.Size;
		}

		commands.pop_marker();
	}

	static void set_style_dark_gold();
	static void set_style_green_blue();
	static void set_style_dark_red();
	static void set_style_dark_classic();

	void ImGuiService::SetStyle(ImGuiStyles style) 
	{

		switch (style) {
		case GreenBlue:
		{
			set_style_green_blue();
			break;
		}

		case DarkRed:
		{
			set_style_dark_red();
			break;
		}

		case DarkGold:
		{
			set_style_dark_gold();
			break;
		}

		case DarkClassic:
		{
			set_style_dark_classic();
		}

		default:
		case Default:
		{
			ImGui::StyleColorsDark();
			break;
		}
		}
	}

	void ImGuiService::RemoveCachedTexture(Magnefu::TextureHandle& texture) 
	{
		FlatHashMapIterator it = g_texture_to_descriptor_set.find(texture.index);
		if (it.is_valid()) 
		{

			// Destroy descriptor set
			Magnefu::DescriptorSetHandle descriptor_set{ g_texture_to_descriptor_set.get(it) };
			gpu->destroy_descriptor_set(descriptor_set);

			// Remove from cache
			g_texture_to_descriptor_set.remove(texture.index);
		}

	}

	void set_style_dark_red() 
	{
		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.37f, 0.14f, 0.14f, 0.67f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.39f, 0.20f, 0.20f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.48f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.48f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.56f, 0.10f, 0.10f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.19f, 0.19f, 0.40f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.89f, 0.00f, 0.19f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(1.00f, 0.19f, 0.19f, 0.40f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.80f, 0.17f, 0.00f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.89f, 0.00f, 0.19f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.33f, 0.35f, 0.36f, 0.53f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.76f, 0.28f, 0.44f, 0.67f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.47f, 0.47f, 0.47f, 0.67f);
		colors[ImGuiCol_Separator] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
		colors[ImGuiCol_Tab] = ImVec4(0.07f, 0.07f, 0.07f, 0.51f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.86f, 0.23f, 0.43f, 0.67f);
		colors[ImGuiCol_TabActive] = ImVec4(0.19f, 0.19f, 0.19f, 0.57f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.05f, 0.05f, 0.05f, 0.90f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.13f, 0.13f, 0.74f);
#if defined(IMGUI_HAS_DOCK)
		colors[ImGuiCol_DockingPreview] = ImVec4(0.47f, 0.47f, 0.47f, 0.47f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
#endif // IMGUI_HAS_DOCK
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
#if defined(IMGUI_HAS_TABLE)
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
#endif // IMGUI_HAS_TABLE
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	}


	void set_style_green_blue() 
	{
		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.44f, 0.44f, 0.44f, 0.60f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.57f, 0.57f, 0.57f, 0.70f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.76f, 0.76f, 0.76f, 0.80f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.13f, 0.75f, 0.55f, 0.80f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.13f, 0.75f, 0.75f, 0.80f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
		colors[ImGuiCol_Button] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
		colors[ImGuiCol_Header] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
		colors[ImGuiCol_Separator] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
		colors[ImGuiCol_Tab] = ImVec4(0.13f, 0.75f, 0.55f, 0.80f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.80f);
		colors[ImGuiCol_TabActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.36f, 0.36f, 0.36f, 0.54f);
#if defined(IMGUI_HAS_DOCK)
		colors[ImGuiCol_DockingPreview] = ImVec4(0.13f, 0.75f, 0.55f, 0.80f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.13f, 0.13f, 0.13f, 0.80f);
#endif // IMGUI_HAS_DOCK
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
#if defined (IMGUI_HAS_TABLE)
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
#endif // IMGUI_HAS_TABLE
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	}

	static void set_style_dark_gold() 
	{
		ImGuiStyle* style = &ImGui::GetStyle();
		ImVec4* colors = style->Colors;

		colors[ImGuiCol_Text] = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.81f, 0.83f, 0.81f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.93f, 0.65f, 0.14f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

		style->FramePadding = ImVec2(4, 2);
		style->ItemSpacing = ImVec2(10, 2);
		style->IndentSpacing = 12;
		style->ScrollbarSize = 10;

		style->WindowRounding = 4;
		style->FrameRounding = 4;
		style->PopupRounding = 4;
		style->ScrollbarRounding = 6;
		style->GrabRounding = 4;
		style->TabRounding = 4;

		style->WindowTitleAlign = ImVec2(1.0f, 0.5f);
		style->WindowMenuButtonPosition = ImGuiDir_Right;

		style->DisplaySafeAreaPadding = ImVec2(4, 4);
	}

	static void set_style_dark_classic()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.7f, 1.0f, 1.0f);
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

		// Text
		colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
	}

	// File Dialog //////////////////////////////////////////////////////////////////
	/*

	//
	//
	struct FileDialogOpenMap {

		char*                           key;
		bool                            value;

	}; // struct FileDialogOpenMap

	static string_hash( FileDialogOpenMap ) file_dialog_open_map = nullptr;

	static Magnefu::Directory             directory;
	static char                         filename[MAX_PATH];
	static char                         last_path[MAX_PATH];
	static char                         last_extension[16];
	static bool                         scan_folder             = true;
	static bool                         init                    = false;

	static Magnefu::StringArray           files;
	static Magnefu::StringArray           directories;

	bool imgui_file_dialog_open( const char* button_name, const char* path, const char* extension ) {

		bool opened = string_hash_get( file_dialog_open_map, button_name );
		if ( ImGui::Button( button_name ) ) {
			opened = true;
		}

		bool selected = false;

		if ( opened && ImGui::Begin( "Magnefu_imgui_file_dialog", &opened, ImGuiWindowFlags_AlwaysAutoResize ) ) {

			ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 20, 20 ) );
			ImGui::Text( directory.path );
			ImGui::PopStyleVar();

			ImGui::Separator();

			ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 20, 4 ) );

			Magnefu::MemoryAllocator* allocator = Magnefu::memory_get_system_allocator();

			if ( !init ) {
				init = true;

				files.init( 10000, allocator );
				directories.init( 10000, allocator );

				string_hash_init_dynamic( file_dialog_open_map );

				filename[0] = 0;
				last_path[0] = 0;
				last_extension[0] = 0;
			}

			if ( strcmp( path, last_path ) != 0 ) {
				strcpy( last_path, path );

				Magnefu::file_open_directory( path, &directory );

				scan_folder = true;
			}

			if ( strcmp( extension, last_extension ) != 0 ) {
				strcpy( last_extension, extension );

				scan_folder = true;
			}

			// Search files
			if ( scan_folder ) {
				scan_folder = false;

				Magnefu::file_find_files_in_path( extension, directory.path, files, directories );
			}

			for ( size_t d = 0; d < directories.get_string_count(); ++d ) {

				const char* directory_name = directories.get_string( d );
				if ( ImGui::Selectable( directory_name, selected, ImGuiSelectableFlags_AllowDoubleClick ) ) {

					if ( strcmp( directory_name, ".." ) == 0 ) {
						Magnefu::file_parent_directory( &directory );
					} else {
						Magnefu::file_sub_directory( &directory, directory_name );
					}

					scan_folder = true;
				}
			}

			for ( size_t f = 0; f < files.get_string_count(); ++f ) {
				const char* file_name = files.get_string( f );
				if ( ImGui::Selectable( file_name, selected, ImGuiSelectableFlags_AllowDoubleClick ) ) {

					strcpy( filename, directory.path );
					filename[strlen( filename ) - 1] = 0;
					strcat( filename, file_name );

					selected = true;
					opened = false;
				}
			}

			ImGui::PopStyleVar();

			ImGui::End();
		}

		// Update opened map
		string_hash_put( file_dialog_open_map, button_name, opened );

		return selected;
	}

	const char * imgui_file_dialog_get_filename() {
		return filename;
	}

	bool imgui_path_dialog_open( const char* button_name, const char* path ) {
		bool opened = string_hash_get( file_dialog_open_map, button_name );
		if ( ImGui::Button( button_name ) ) {
			opened = true;
		}

		bool selected = false;

		if ( opened && ImGui::Begin( "Magnefu_imgui_file_dialog", &opened, ImGuiWindowFlags_AlwaysAutoResize ) ) {

			ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 20, 20 ) );
			ImGui::Text( directory.path );
			ImGui::PopStyleVar();

			ImGui::Separator();

			ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 20, 4 ) );

			Magnefu::MemoryAllocator* allocator = Magnefu::memory_get_system_allocator();

			if ( !init ) {
				init = true;

				files.init( 10000, allocator );
				directories.init( 10000, allocator );

				string_hash_init_dynamic( file_dialog_open_map );

				filename[0] = 0;
				last_path[0] = 0;
				last_extension[0] = 0;
			}

			if ( strcmp( path, last_path ) != 0 ) {
				strcpy( last_path, path );

				Magnefu::file_open_directory( path, &directory );

				scan_folder = true;
			}

			// Search files
			if ( scan_folder ) {
				scan_folder = false;

				Magnefu::file_find_files_in_path( ".", directory.path, files, directories );
			}

			for ( size_t d = 0; d < directories.get_string_count(); ++d ) {

				const char* directory_name = directories.get_string( d );
				if ( ImGui::Selectable( directory_name, selected, ImGuiSelectableFlags_AllowDoubleClick ) ) {

					if ( strcmp( directory_name, ".." ) == 0 ) {
						Magnefu::file_parent_directory( &directory );
					} else {
						Magnefu::file_sub_directory( &directory, directory_name );
					}

					scan_folder = true;
				}
			}

			if ( ImGui::Button( "Choose Current Folder" ) ) {
				strcpy( last_path, directory.path );
				// Remove the asterisk
				last_path[strlen( last_path ) - 1] = 0;

				selected = true;
				opened = false;
			}
			ImGui::SameLine();
			if ( ImGui::Button( "Cancel" ) ) {
				opened = false;
			}

			ImGui::PopStyleVar();

			ImGui::End();
		}

		// Update opened map
		string_hash_put( file_dialog_open_map, button_name, opened );

		return selected;
	}

	const char* imgui_path_dialog_get_path() {
		return last_path;
	}

	////////////////////////////////////////////////////////////
	*/
	// Usage:
	//  static ExampleAppLog my_log;
	//  my_log.AddLog("Hello %d world\n", 123);
	//  my_log.Draw("title");
	struct ExampleAppLog 
	{
		ImGuiTextBuffer     Buf;
		ImGuiTextFilter     Filter;
		ImVector<int>       LineOffsets;        // Index to lines offset. We maintain this with AddLog() calls, allowing us to have a random access on lines
		bool                AutoScroll;     // Keep scrolling if already at the bottom

		ExampleAppLog() 
		{
			AutoScroll = true;
			Clear();
		}

		void    Clear() 
		{
			Buf.clear();
			LineOffsets.clear();
			LineOffsets.push_back(0);
		}

		void    AddLog(const char* fmt, ...) IM_FMTARGS(2) 
		{
			int old_size = Buf.size();
			va_list args;
			va_start(args, fmt);
			Buf.appendfv(fmt, args);
			va_end(args);
			for (int new_size = Buf.size(); old_size < new_size; old_size++)
				if (Buf[old_size] == '\n')
					LineOffsets.push_back(old_size + 1);
		}

		void    Draw(const char* title, bool* p_open = NULL) {
			if (!ImGui::Begin(title, p_open)) {
				ImGui::End();
				return;
			}

			// Options menu
			if (ImGui::BeginPopup("Options")) {
				ImGui::Checkbox("Auto-scroll", &AutoScroll);
				ImGui::EndPopup();
			}

			// Main window
			if (ImGui::Button("Options"))
				ImGui::OpenPopup("Options");
			ImGui::SameLine();
			bool clear = ImGui::Button("Clear");
			ImGui::SameLine();
			bool copy = ImGui::Button("Copy");
			ImGui::SameLine();
			Filter.Draw("Filter", -100.0f);

			ImGui::Separator();
			ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

			if (clear)
				Clear();
			if (copy)
				ImGui::LogToClipboard();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			const char* buf = Buf.begin();
			const char* buf_end = Buf.end();
			if (Filter.IsActive()) {
				// In this example we don't use the clipper when Filter is enabled.
				// This is because we don't have a random access on the result on our filter.
				// A real application processing logs with ten of thousands of entries may want to store the result of search/filter.
				// especially if the filtering function is not trivial (e.g. reg-exp).
				for (int line_no = 0; line_no < LineOffsets.Size; line_no++) {
					const char* line_start = buf + LineOffsets[line_no];
					const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
					if (Filter.PassFilter(line_start, line_end))
						ImGui::TextUnformatted(line_start, line_end);
				}
			}
			else {
				// The simplest and easy way to display the entire buffer:
				//   ImGui::TextUnformatted(buf_begin, buf_end);
				// And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward to skip non-visible lines.
				// Here we instead demonstrate using the clipper to only process lines that are within the visible area.
				// If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them on your side is recommended.
				// Using ImGuiListClipper requires A) random access into your data, and B) items all being the  same height,
				// both of which we can handle since we an array pointing to the beginning of each line of text.
				// When using the filter (in the block of code above) we don't have random access into the data to display anymore, which is why we don't use the clipper.
				// Storing or skimming through the search result would make it possible (and would be recommended if you want to search through tens of thousands of entries)
				ImGuiListClipper clipper;
				clipper.Begin(LineOffsets.Size);
				while (clipper.Step()) {
					for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++) {
						const char* line_start = buf + LineOffsets[line_no];
						const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
						ImGui::TextUnformatted(line_start, line_end);
					}
				}
				clipper.End();
			}
			ImGui::PopStyleVar();

			if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.0f);

			ImGui::EndChild();
			ImGui::End();
		}
	}; // struct ExampleAppLog

	static ExampleAppLog        s_imgui_log;
	static bool                 s_imgui_log_open = true;

	static void imgui_print(const char* text) 
	{
		s_imgui_log.AddLog("%s", text);
	}

	void imgui_log_init() 
	{

		LogService::Instance()->SetCallback(&imgui_print);
	}

	void imgui_log_shutdown() 
	{

		LogService::Instance()->SetCallback(nullptr);
	}

	void imgui_log_draw() {
		s_imgui_log.Draw("Log", &s_imgui_log_open);
	}

	void ImGuiService::OnAttach()
	{

		// TODO: Make Code API Agnostic
		// Right now it is all specific to Vulkan
		// Probably use a switch on the renderer api

		//Application& app = Application::Get();
		//GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
		//GraphicsContext* context = GraphicsContext::Instance();

		//s_Instance = std::any_cast<VkInstance>(context->GetContextInfo("Instance"));
		//s_Device = std::any_cast<VkDevice>(context->GetContextInfo("Device"));
		//s_PhysicalDevice = std::any_cast<VkPhysicalDevice>(context->GetContextInfo("PhysicalDevice"));
		//s_QueueFamily = std::any_cast<uint32_t>(context->GetContextInfo("QueueFamily"));
		//s_Queue = std::any_cast<VkQueue>(context->GetContextInfo("Queue"));
		//s_MinImageCount = std::any_cast<uint32_t>(context->GetContextInfo("ImageCount"));

		//
		//SetupVulkan();

		//ImGui_ImplVulkanH_Window* wd = &s_MainWindowData;
		//SetupImGuiVulkanPrimitives(wd, context);
		//
		//SetupImGuiVulkanFrames(wd, context);
		//CreateImageViews(wd);
		//CreateRenderPass(wd);
		//CreateFrameBuffers(wd);
	

		//// Setup Dear ImGui context
		//IMGUI_CHECKVERSION();
		//ImGui::CreateContext();
		//ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		////io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		////io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		////io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		////float fontSize = 18.0f;// *2.0f;
		////io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Bold.ttf", fontSize);
		////io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Regular.ttf", fontSize);


		//// Setup Dear ImGui style
		//ImGui::StyleColorsDark();

		//// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		//ImGuiStyle& style = ImGui::GetStyle();
		//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		//{
		//	style.WindowRounding = 0.0f;
		//	style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		//}

		//SetDarkThemeColors();

		//// Setup Platform/Renderer backends
		//ImGui_ImplGlfw_InitForVulkan(window, true); // TODO: Make this based on an API switch
		//ImGui_ImplVulkan_InitInfo init_info = {};
		//init_info.Instance = s_Instance;
		//init_info.PhysicalDevice = s_PhysicalDevice;
		//init_info.Device = s_Device;
		//init_info.QueueFamily = s_QueueFamily;
		//init_info.Queue = s_Queue;
		//init_info.PipelineCache = s_PipelineCache;
		//init_info.DescriptorPool = s_DescriptorPool;
		//init_info.Subpass = 0;
		//init_info.MinImageCount = s_MinImageCount;
		//init_info.ImageCount = wd->ImageCount;
		//init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		//init_info.Allocator = s_Allocator;
		//init_info.CheckVkResultFn = nullptr;

		//// ImGui's Vulkan Render Pipeline created here
		//ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

		//// Upload Fonts
		//{
		//	// Use any command queue
		//	VkCommandPool command_pool = ;
		//	VkCommandBuffer command_buffer = conte;

		//	vkResetCommandPool(s_Device, command_pool, 0);
		//	VkCommandBufferBeginInfo begin_info = {};
		//	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//	begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		//	vkBeginCommandBuffer(command_buffer, &begin_info);

		//	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

		//	VkSubmitInfo end_info = {};
		//	end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		//	end_info.commandBufferCount = 1;
		//	end_info.pCommandBuffers = &command_buffer;
		//	vkEndCommandBuffer(command_buffer);
		//	vkQueueSubmit(s_Queue, 1, &end_info, VK_NULL_HANDLE);

		//	vkDeviceWaitIdle(s_Device);
		//	ImGui_ImplVulkan_DestroyFontUploadObjects();
		//}
	}

	void ImGuiService::OnDetach()
	{
		//vkDeviceWaitIdle(s_Device); // already running in m_Window->OnFinish();
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		//CleanupVulkanWindow();
	}

	void ImGuiService::OnEvent(Event& e)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.m_Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			e.m_Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;

			//MF_CORE_DEBUG("{}", e.ToString());
		}
		
		

	}


	

	//void ImGuiService::EndFrame()
	//{
	//	ImGui::EndFrame();
	//	ImGui::Render();

	//	ImGuiIO& io = ImGui::GetIO();
	//	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	//	{
	//		GLFWwindow* contextBackup = glfwGetCurrentContext();
	//		ImGui::UpdatePlatformWindows();
	//		ImGui::RenderPlatformWindowsDefault();
	//		glfwMakeContextCurrent(contextBackup);
	//	}

	//}

	//void ImGuiService::RecreateImageResources()
	//{
	//	Application& app = Application::Get();
	//	VulkanContext* context = static_cast<VulkanContext*>(app.GetWindow().GetGraphicsContext());

	//	GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
	//	s_MinImageCount = std::any_cast<uint32_t>(context->GetContextInfo("ImageCount"));

	//	ImGui_ImplVulkanH_Window* wd = &s_MainWindowData;
	//	SetupImGuiVulkanPrimitives(wd, context);

	//	int width, height;
	//	glfwGetFramebufferSize(window, &width, &height);
	//	if (width > 0 && height > 0)
	//	{
	//		SetupImGuiVulkanFrames(wd, context);
	//		CreateImageViews(wd);
	//		CreateFrameBuffers(wd);
	//	}
	//}

	//void ImGuiService::RecordAndSubmitCommandBuffer(uint32_t imageIndex)
	//{
	//	Application& app = Application::Get();
	//	VulkanContext* context = static_cast<VulkanContext*>(app.GetWindow().GetGraphicsContext());

	//	ImDrawData* draw_data = ImGui::GetDrawData();
	//	const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
	//	ImGui_ImplVulkanH_Window* wd = &s_MainWindowData;

	//	wd->FrameIndex = imageIndex;  // NEEDS TO BE THE IMAGE INDEX, NOT CURRENT FRAME

	//	if (!is_minimized)
	//	{
	//		wd->ClearValue.color.float32[0] = s_ClearColor.x * s_ClearColor.w;
	//		wd->ClearValue.color.float32[1] = s_ClearColor.y * s_ClearColor.w;
	//		wd->ClearValue.color.float32[2] = s_ClearColor.z * s_ClearColor.w;
	//		wd->ClearValue.color.float32[3] = s_ClearColor.w;

	//		// NEED TO BE SURE TO PROVIDE THESE VALUES SPECIFICALLY AS THEY MATCH THE NUMBE OF FRAMES IN FLIGHT, NOT THE IMAGE COUNT
	//		// FRAMES IN FLIGHT -> 2
	//		// IMAGE COUNT -> 3
	//		wd->Frames[imageIndex].Fence = std::any_cast<VkFence>(context->GetContextInfo("ImGuiInFlightFence"));
	//		wd->Frames[imageIndex].CommandBuffer = std::any_cast<VkCommandBuffer>(context->GetContextInfo("ImGuiCommandBuffer"));

	//		// get wait and signal semaphores and send them to frame render
	//		VkSemaphore* wait = std::any_cast<VkSemaphore*>(context->GetContextInfo("ImGuiWaitSemaphores"));
	//		VkSemaphore* signal = std::any_cast<VkSemaphore*>(context->GetContextInfo("ImGuiSignalSemaphores"));
	//		

	//		FrameRender(wd, draw_data, wait, signal);
	//	}
	//}

	//void ImGuiService::OnRender()
	//{
	//	
	//}

	//void ImGuiService::SetDarkThemeColors()
	//{
	//	auto& colors = ImGui::GetStyle().Colors;
	//	colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

	//	// Headers
	//	colors[ImGuiCol_Header] = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);
	//	colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.7f, 1.0f, 1.0f);
	//	colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

	//	// Buttons
	//	colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	//	colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.6f, 1.0f };
	//	colors[ImGuiCol_ButtonActive] = ImVec4{ 0.3f, 0.305f, 1.f, 1.0f };

	//	// Frame BG
	//	colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	//	colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
	//	colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

	//	// Tabs
	//	colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	//	colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
	//	colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
	//	colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	//	colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

	//	// Title
	//	colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	//	colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	//	colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

	//	// Text
	//	colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White

	//}

	//uint32_t ImGuiService::GetActiveWidgetID() const
	//{
	//	//return GImGui->ActiveId;
	//	return 0;
	//}

}