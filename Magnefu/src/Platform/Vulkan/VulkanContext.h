#pragma once

#include "Magnefu/Renderer/GraphicsContext.h"

#include "VulkanCommon.h"

#include "VulkanBuffer.h"


struct GLFWwindow;

namespace Magnefu
{
	using String = std::string;

	enum class ShaderType
	{
		None = -1,
		Vertex,
		Fragment,
		Compute
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
		ShaderSource Compute;
	};

	struct QueueFamilyIndices 
	{
		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> PresentFamily;
		std::optional<uint32_t> ComputeFamily;

		bool IsComplete() {
			return (GraphicsFamily.has_value() && PresentFamily.has_value() && ComputeFamily.has_value());
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

	

	struct ParticleUniformBufferObject 
	{
		float deltaTime = 1.0f;
	};

	struct Particle {
		Maths::vec2 position;
		Maths::vec2 velocity;
		Maths::vec4 color;

		static VkVertexInputBindingDescription GetBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Particle);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Particle, position);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Particle, color);

			return attributeDescriptions;
		}
	};

	
	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);
		~VulkanContext();

		inline static VulkanContext& Get() { return *s_Instance; }

		// -- Inherited -- //
		void Init() override;
		void TempSecondaryInit() override;
		void DrawFrame() override;
		void OnImGuiRender() override;
		void OnFinish() override; // main loop completed
		std::any GetContextInfo(const std::string& name) override;
		void SetFramebufferResized(bool framebufferResized) override { m_FramebufferResized = framebufferResized; }
		const RendererInfo& GetRendererInfo() const override { return m_RendererInfo; }
		void SetPushConstants(PushConstants& pushConstants) override { m_PushConstants = pushConstants; }

		// -- Getter Methods -- //
		inline const VkDevice& GetDevice() const { return m_VkDevice; }
		inline const VkPhysicalDevice& GetPhysicalDevice() const { return m_VkPhysicalDevice; }
		inline const VkCommandPool& GetCommandPool() const { return m_CommandPool; }
		inline const VkQueue& GetGraphicsQueue() const { return m_GraphicsQueue; }
		inline const VkExtent2D& GetSwapChainExtent() const { return m_SwapChainExtent; }
		inline const uint32_t GetCurrentFrame() const { return m_CurrentFrame; }
		inline const VkPhysicalDeviceProperties GetDeviceProperties() const { return m_Properties; }
		inline const VkPhysicalDeviceFeatures GetSupportedFeatures() const { return m_SupportedFeatures; }


	private:

		

		// -- Initialization -- //
		void CreateVkInstance();
		void SetupValidationLayers(bool& allLayersAvailable);
		void SetupDebugMessenger();
		void CreateWindowSurface();
		void SelectPhysicalDevice();
		void CreateLogicalDevice();

		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateComputeDescriptorSetLayout();
		void CreateGraphicsPipeline();
		void CreateParticleGraphicsPipeline();
		void CreateFrameBuffers();
		void CreateCommandPool();
		void CreateShaderStorageBuffers();
		void CreateComputePipeline();
		void CreateColorResources();
		void CreateDepthResources();
		void LoadModel();

		void CreateComputeUniformBuffers();
		void CreateComputeDescriptorPool();
		void CreateComputeDescriptorSets();
		void CreateCommandBuffers();
		void CreateComputeCommandBuffers();
		void CreateSyncObjects();


		// -- Device -- //
		std::vector<const char*> GetRequiredExtensions();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		bool IsDeviceSuitable(VkPhysicalDevice);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice);


		// -- Swap Chain -- //
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		
		VkFormat FindDepthFormat();
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);


		// Place in VKShader
		ShaderList ParseShader(const String& filepath);
		VkShaderModule CreateShaderModule(const ShaderSource& source);
		

		// Uniforms
		//void UpdateUniformBuffer();
		void UpdateComputeUniformBuffer();

		// Depth 
		bool HasStencilComponent(VkFormat format);


		VkSampleCountFlagBits GetMaxUsableSampleCount();
		

		// Part of Render Loop
		void RecordComputeCommandBuffer(VkCommandBuffer commandBuffer);
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, VulkanBuffer& vertexBuffer, VulkanBuffer& indexBuffer, uint32_t indexCount, VkDescriptorSet& descriptorSet);
		void RecreateSwapChain();
		void PerformComputeOps();
		void PerformGraphicsOps();
		void PresentImage();


		// -- Clean Up -- //
		void CleanupSwapChain();

	private:

		static VulkanContext* s_Instance;

#ifdef MF_DEBUG
		const bool                   m_EnableValidationLayers = true;
#else			                   
		const bool                   m_EnableValidationLayers = false;
#endif


		// -- Device Properties and Features -- //
		VkPhysicalDeviceProperties   m_Properties{};
		VkPhysicalDeviceFeatures     m_SupportedFeatures;

		// -- Device Primitives -- //
		RendererInfo                 m_RendererInfo;
		VkDebugUtilsMessengerEXT     m_DebugMessenger;
		VkInstance                   m_VkInstance;
		VkPhysicalDevice             m_VkPhysicalDevice;
		VkDevice                     m_VkDevice;

		// -- Window Primitives -- //
		GLFWwindow*                  m_WindowHandle;
		VkSurfaceKHR                 m_WindowSurface;

		// -- Queues -- //
		VkQueue                      m_GraphicsQueue;
		VkQueue			             m_PresentQueue;
		VkQueue                      m_ComputeQueue;
		QueueFamilyIndices           m_QueueFamilyIndices;
		VkSurfaceFormatKHR           m_SurfaceFormat;

		// -- Swap Chain -- //
		uint32_t                     m_ImageCount;
		uint32_t                     m_ImageIndex;
		bool                         m_SwapChainRebuild = false;
		VkPresentModeKHR             m_PresentMode;
		VkSwapchainKHR               m_SwapChain;
		std::vector<VkImage>         m_SwapChainImages;
		VkFormat                     m_SwapChainImageFormat;
		VkExtent2D                   m_SwapChainExtent;
		std::vector<VkImageView>     m_SwapChainImageViews;

		// -- Render Pass and Pipeline Primitives -- //
		VkRenderPass                 m_RenderPass;
		ShaderList                   m_ParticleShaderList;
		VkPipelineLayout             m_PipelineLayout;
		VkPipeline                   m_GraphicsPipeline;
		std::vector<VkFramebuffer>   m_SwapChainFramebuffers;
		VkCommandPool                m_CommandPool;
		std::vector<VkCommandBuffer> m_CommandBuffers;
		uint32_t                     m_CurrentFrame;
		bool						 m_FramebufferResized;

		// -- Synchronization Primitives -- //
		std::vector<VkSemaphore>     m_ImageAvailableSemaphores;
		std::vector<VkSemaphore>     m_ImGuiRenderFinishedSemaphores;
		std::vector<VkSemaphore>     m_RenderFinishedSemaphores;
		std::vector<VkFence>         m_InFlightFences;


		// -- Mip Map Info -- //
		VkSampleCountFlagBits        m_MSAASamples = VK_SAMPLE_COUNT_1_BIT;


		// -- Image Buffers -- //

		VkImage                      m_DepthImage;
		VkDeviceMemory               m_DepthImageMemory;
		VkImageView                  m_DepthImageView;
		VkImage                      m_ColorImage;
		VkDeviceMemory               m_ColorImageMemory;
		VkImageView                  m_ColorImageView;
		

		//------------------- Compute Shader ---------------------- //
		std::vector<VkBuffer>        m_ShaderStorageBuffers;
		std::vector<VkDeviceMemory>  m_ShaderStorageBuffersMemory;
		VkDescriptorSetLayout        m_ComputeDescriptorSetLayout;
		VkDescriptorPool             m_ComputeDescriptorPool;
		std::vector<VkDescriptorSet> m_ComputeDescriptorSets;
		std::vector<VkBuffer>        m_ComputeUniformBuffers;
		std::vector<VkDeviceMemory>  m_ComputeUniformBuffersMemory;
		std::vector<void*>           m_ComputeUniformBuffersMapped;
		VkPipeline                   m_ComputePipeline;
		VkPipelineLayout             m_ComputePipelineLayout;
		std::vector<VkFence>         m_ComputeInFlightFences;
		std::vector<VkSemaphore>     m_ComputeFinishedSemaphores;
		std::vector<VkCommandBuffer> m_ComputeCommandBuffers;

		VkPipeline                   m_ParticleGraphicsPipeline;
		VkPipelineLayout             m_ParticleGraphicsPipelineLayout;
		// -------------------------------------------------------- //

		PushConstants                m_PushConstants;


		//------------------- ImGui ---------------------- //

		std::vector<VkCommandBuffer> m_ImGuiCommandBuffers;
		std::vector<VkFence>         m_ImGuiInFlightFences;

		// ----------------------------------------------- //

	};
}

namespace std
{
	template<> struct hash<Magnefu::Vertex> {
		size_t operator()(Magnefu::Vertex const& vertex) const {
			return ((hash<Maths::vec3>()(vertex.pos) ^
				(hash<Maths::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<Maths::vec2>()(vertex.texCoord) << 1);
		}
	};
}