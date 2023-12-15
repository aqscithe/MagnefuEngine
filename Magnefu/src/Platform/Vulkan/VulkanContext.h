#pragma once

#include "Magnefu/Renderer/GraphicsContext.h"
#include "Magnefu/Renderer/SceneObject.h"
#include "Magnefu/ResourceManagement/ResourcePaths.h"

#include "VulkanCommon.h"

#include "VulkanBuffer.h"



struct GLFWwindow;

namespace Magnefu
{
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
		Array<VkSurfaceFormatKHR> Formats;
		Array<VkPresentModeKHR>   PresentModes;
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

	struct VulkanMemory
	{
		// Stats
		VmaStatistics Stats;
		VmaTotalStatistics TotalStats;
		VmaBudget Budgets;
		
		// Vertex Buffer
		Array<VkDeviceSize> VBufferOffsets;
		VkBuffer                  VBuffer;
		VmaAllocation             VBufferAllocation;
		VmaAllocationInfo         VBufferAllocInfo;

		// Index Buffer
		Array<VkDeviceSize> IBufferOffsets;
		VkBuffer                  IBuffer;
		VmaAllocation             IBufferAllocation;
		VmaAllocationInfo         IBufferAllocInfo;


		// Uniforms
		VkDeviceSize                   UniformOffset = 0;
		VkDeviceSize                   UniformAlignment;
		Array<VkBuffer>          UniformBuffers;
		Array<void*>             UniformBuffersMapped;
		Array<VmaAllocation>     UniformAllocations;
		Array<VmaAllocationInfo> UniformAllocInfo;

		// Framebuffer Resources
		VkImage DepthImage;
		VkImage ColorImage;
		VmaAllocation DepthResAllocation;
		VmaAllocation ColorResAllocation;
		VmaAllocationInfo DepthResAllocInfo;
		VmaAllocationInfo ColorResAllocInfo;
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

		// Memory
		void CalculateMemoryStats() override;
		MemoryStats GetMemoryStats() override;

		uint64_t GetVBufferOffset(uint32_t index) override { return static_cast<uint64_t>(m_VulkanMemory.VBufferOffsets[index]); }
		uint64_t GetIBufferOffset(uint32_t index) override { return static_cast<uint64_t>(m_VulkanMemory.IBufferOffsets[index]); }

		// -- Getter Methods -- //
		inline const VkDevice& GetDevice() const { return m_VkDevice; }
		inline const VkPhysicalDevice& GetPhysicalDevice() const { return m_VkPhysicalDevice; }
		inline const VkCommandPool& GetCommandPool() const { return m_CommandPool; }
		inline const VkQueue& GetGraphicsQueue() const { return m_GraphicsQueue; }
		inline const VkExtent2D& GetSwapChainExtent() const { return m_SwapChainExtent; }
		inline const uint32_t GetCurrentFrame() const { return m_CurrentFrame; }
		inline const VkPhysicalDeviceProperties GetDeviceProperties() const { return m_Properties; }
		inline const VkPhysicalDeviceFeatures GetSupportedFeatures() const { return m_SupportedFeatures; }
		inline const VkPhysicalDeviceFeatures GetEnabledFeatures() const { return m_EnabledFeatures; }
		inline const VkSampleCountFlagBits GetMSAASamples() const { return m_MSAASamples; }
		inline const VkRenderPass& GetRenderPass() const { return m_RenderPass; }
		inline const VmaAllocator& GetVmaAllocator() const { return m_VmaAllocator; }

		inline const Array<VkBuffer>& GetUniformBuffers() { return m_VulkanMemory.UniformBuffers; }
		inline VulkanMemory& GetVulkanMemory() { return m_VulkanMemory; }

		


	private:

		

		// -- Initialization -- //
		void CreateVkInstance();
		void SetupValidationLayers(bool& allLayersAvailable);
		void SetupDebugMessenger();
		void CreateWindowSurface();
		void SelectPhysicalDevice();
		void CreateLogicalDevice();
		void CreateVmaAllocator();
		void AllocateBufferMemory();


		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateFrameBuffers();
		void CreateCommandPool();
		void CreateColorResources();
		void CreateDepthResources();
		void LoadModels();
		void LoadTextures();

		void CreateCommandBuffers();
		void CreateSyncObjects();


		void LoadSingleModel(const ResourceInfo&, size_t objIndex, ModelType modelType);
		void LoadSingleTexture(int sceneObjIndex, const char* texturePath, int textureType);


		// -- Buffers -- //
		void AllocateIndexBuffers(const uint32_t& sceneObjCount, Array<Magnefu::SceneObject>& sceneObjs, VkCommandPool commandPool);
		void AllocateVertexBuffers(const uint32_t& sceneObjCount, Array<Magnefu::SceneObject>& sceneObjs, VkCommandPool commandPool);
		void AllocateUniformBuffers(const uint32_t& sceneObjCount);


		// -- Device -- //
		Array<const char*> GetRequiredExtensions();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		bool IsDeviceSuitable(VkPhysicalDevice);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice);


		// -- Swap Chain -- //
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const Array<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const Array<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		
		VkFormat FindDepthFormat();
		VkFormat FindSupportedFormat(const VkFormat* candidates, u32 candidateCount, VkImageTiling tiling, VkFormatFeatureFlags features);
		

		// Uniforms

		// Depth 
		bool HasStencilComponent(VkFormat format);

		VkSampleCountFlagBits GetMaxUsableSampleCount();
		

		// Part of Render Loop
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void RecreateSwapChain();
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
		
		uint32_t m_APIVersion = VK_API_VERSION_1_3;


		// -- Device Properties and Features -- //
		VkPhysicalDeviceProperties   m_Properties{};
		VkPhysicalDeviceFeatures     m_SupportedFeatures;
		VkPhysicalDeviceFeatures     m_EnabledFeatures;
		

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
		Array<VkImage>         m_SwapChainImages;
		VkFormat                     m_SwapChainImageFormat;
		VkExtent2D                   m_SwapChainExtent;
		Array<VkImageView>     m_SwapChainImageViews;

		// -- Render Pass and Pipeline Primitives -- //
		VkRenderPass                 m_RenderPass;
		Array<VkFramebuffer>   m_SwapChainFramebuffers;
		VkCommandPool                m_CommandPool;
		Array<VkCommandBuffer> m_CommandBuffers;
		uint32_t                     m_CurrentFrame;
		bool						 m_FramebufferResized;

		// -- Synchronization Primitives -- //
		Array<VkSemaphore>     m_ImageAvailableSemaphores;
		Array<VkSemaphore>     m_ImGuiRenderFinishedSemaphores;
		Array<VkSemaphore>     m_RenderFinishedSemaphores;
		Array<VkFence>         m_InFlightFences;


		// -- Mip Map Info -- //
		VkSampleCountFlagBits        m_MSAASamples = VK_SAMPLE_COUNT_1_BIT;


		// -- Image Buffers -- //

		VkImageView                  m_DepthImageView;
		VkImageView                  m_ColorImageView;

		// -- Memory Allocations -- //


		// -- Vma Primitives -- //
		VmaAllocator                m_VmaAllocator;
		VulkanMemory m_VulkanMemory;
		

		//------------------- Compute Shader ---------------------- //
		// -------------------------------------------------------- //

		PushConstants                m_PushConstants;


		//------------------- ImGui ---------------------- //

		Array<VkCommandBuffer> m_ImGuiCommandBuffers;
		Array<VkFence>         m_ImGuiInFlightFences;

		// ----------------------------------------------- //

	};
}

namespace std
{
	template<> struct hash<Magnefu::VulkanVertex> {
		size_t operator()(Magnefu::VulkanVertex const& vertex) const {
			return ((hash<Maths::vec3>()(vertex.pos) ^
				(hash<Maths::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<Maths::vec2>()(vertex.texCoord) << 1);
		}
	};
}