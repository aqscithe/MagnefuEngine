#pragma once

#include "Magnefu/Renderer/GraphicsContext.h"
#include "vulkan/vulkan.h"


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

	struct Vertex 
	{
		Maths::vec3 pos;
		Maths::vec3 color;
		Maths::vec2 texCoord;

		static VkVertexInputBindingDescription GetBindingDescription() 
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

			
			return attributeDescriptions;
		}

	};


	struct UniformBufferObject 
	{
		alignas(16) Maths::mat4 model;
		alignas(16) Maths::mat4 view;
		alignas(16) Maths::mat4 proj;
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

	class VKContext : public GraphicsContext
	{
	public:
		VKContext(GLFWwindow* windowHandle);
		~VKContext();

		void Init() override;
		void SwapBuffers() override;
		void DrawFrame() override;
		void OnImGuiRender() override;
		void OnFinish() override; // main loop completed
		std::any GetContextInfo(const std::string& name) override;
		void SetFramebufferResized(bool framebufferResized) override { m_FramebufferResized = framebufferResized; }

		

	private:
		void CreateVkInstance();
		void SetupValidationLayers(bool& allLayersAvailable);
		void SetupDebugMessenger();
		void CreateWindowSurface();
		void SelectPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateDescriptorSetLayout();
		void CreateComputeDescriptorSetLayout();
		void CreateGraphicsPipeline();
		void CreateParticleGraphicsPipeline();
		void CreateFrameBuffers();
		void CreateCommandPool();
		void CreateShaderStorageBuffers();
		void CreateComputePipeline();
		void CreateColorResources();
		void CreateDepthResources();
		void CreateTextureImage();
		void CreateTextureImageView();
		void CreateTextureSampler();
		void LoadModel();
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateUniformBuffers();
		void CreateComputeUniformBuffers();
		void CreateDescriptorPool();
		void CreateComputeDescriptorPool();
		void CreateDescriptorSets();
		void CreateComputeDescriptorSets();
		void CreateCommandBuffers();
		void CreateComputeCommandBuffers();
		void CreateSyncObjects();

		std::vector<const char*> GetRequiredExtensions();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		bool IsDeviceSuitable(VkPhysicalDevice);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void RecreateSwapChain();
		void CleanupSwapChain();
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void UpdateUniformBuffer();
		void UpdateComputeUniformBuffer();
		void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageType imageType, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat FindDepthFormat();
		bool HasStencilComponent(VkFormat format);
		void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		VkSampleCountFlagBits GetMaxUsableSampleCount();
		void RecordComputeCommandBuffer(VkCommandBuffer commandBuffer);


		// Place in VKShader
		ShaderList ParseShader(const String& filepath);
		VkShaderModule CreateShaderModule(const ShaderSource& source);

	private:
		GLFWwindow* m_WindowHandle;
		RendererInfo                 m_RendererInfo;
		VkInstance                   m_VkInstance;
		VkDebugUtilsMessengerEXT     m_DebugMessenger;
		VkPhysicalDevice             m_VkPhysicalDevice;
		VkDevice                     m_VkDevice;
		VkQueue                      m_GraphicsQueue;
		VkSurfaceKHR                 m_WindowSurface;
		VkQueue			             m_PresentQueue;
		VkQueue                      m_ComputeQueue;
		QueueFamilyIndices           m_QueueFamilyIndices;
		VkSurfaceFormatKHR           m_SurfaceFormat;
		uint32_t                     m_ImageCount;
		bool                         m_SwapChainRebuild = false;
		VkSwapchainKHR               m_SwapChain;
		std::vector<VkImage>         m_SwapChainImages;
		VkFormat                     m_SwapChainImageFormat;
		VkExtent2D                   m_SwapChainExtent;
		std::vector<VkImageView>     m_SwapChainImageViews;
		VkRenderPass                 m_RenderPass;
		VkDescriptorSetLayout        m_DescriptorSetLayout;
		ShaderList                   m_ParticleShaderList;
		VkPipelineLayout             m_PipelineLayout;
		VkPipeline                   m_GraphicsPipeline;
		std::vector<VkFramebuffer>   m_SwapChainFramebuffers;
		VkCommandPool                m_CommandPool;
		std::vector<VkCommandBuffer> m_CommandBuffers;
		std::vector<VkSemaphore>     m_ImageAvailableSemaphores;
		std::vector<VkSemaphore>     m_RenderFinishedSemaphores;
		std::vector<VkFence>         m_InFlightFences;
		uint32_t                     m_CurrentFrame;
		bool						 m_FramebufferResized;
		std::vector<Vertex>          m_Vertices;
		std::vector<uint32_t>        m_Indices;
		VkBuffer                     m_VertexBuffer;
		VkDeviceMemory               m_VertexBufferMemory;
		VkBuffer                     m_IndexBuffer;
		VkDeviceMemory               m_IndexBufferMemory;
		std::vector<VkBuffer>        m_UniformBuffers;
		std::vector<VkDeviceMemory>  m_UniformBuffersMemory;
		std::vector<void*>           m_UniformBuffersMapped;
		VkDescriptorPool             m_DescriptorPool;
		std::vector<VkDescriptorSet> m_DescriptorSets;
		uint32_t                     m_MipLevels;
		VkImage                      m_TextureImage;
		VkDeviceMemory               m_TextureImageMemory;
		VkImageView                  m_TextureImageView;
		VkSampler                    m_TextureSampler;
		VkImage                      m_DepthImage;
		VkDeviceMemory               m_DepthImageMemory;
		VkImageView                  m_DepthImageView;
		VkImage                      m_ColorImage;
		VkDeviceMemory               m_ColorImageMemory;
		VkImageView                  m_ColorImageView;
		VkSampleCountFlagBits        m_MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		//------------------- Particles     ---------------------- //
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

		VkPhysicalDeviceProperties   m_Properties{};
		VkPhysicalDeviceFeatures     m_SupportedFeatures;

#ifdef MF_DEBUG
		const bool                   m_EnableValidationLayers = true;
#else			                   
		const bool                   m_EnableValidationLayers = false;
#endif
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