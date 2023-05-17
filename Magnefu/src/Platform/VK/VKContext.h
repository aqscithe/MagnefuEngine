#pragma once

#include "Magnefu/Renderer/GraphicsContext.h"
#include "vulkan/vulkan.h"

struct GLFWwindow;

namespace Magnefu
{
	struct QueueFamilyIndices {
		std::optional<uint32_t> GraphicsFamily;

		bool IsComplete() {
			return GraphicsFamily.has_value();
		}
	};

	class VKContext : public GraphicsContext
	{
	public:
		VKContext(GLFWwindow* windowHandle);
		~VKContext();

		void Init() override;
		void SwapBuffers() override;
		void OnImGuiRender() override;

	private:
		bool IsDeviceSuitable(VkPhysicalDevice);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice);

	private:
		GLFWwindow*      m_WindowHandle;
		RendererInfo     m_RendererInfo;
		VkInstance       m_VkInstance;
		VkPhysicalDevice m_VkPhysicalDevice;
		VkDevice         m_VkDevice;
		VkQueue          m_GraphicsQueue;
		VkSurfaceKHR     m_WindowSurface;
	};
}