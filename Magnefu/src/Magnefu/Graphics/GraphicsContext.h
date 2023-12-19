#pragma once

#include "Light.h"


namespace Magnefu
{
	using String = std::string;

	struct PushConstants
	{
		float Roughness;
	};

	struct RendererInfo
	{
		String Version;
		String Vendor;
		String Renderer;
	};

	struct MemoryStats 
	{ 
		uint32_t blockCount; 
		uint32_t allocationCount; 
		uint64_t blockBytes; 
		uint64_t allocationBytes; 
		uint64_t usage;
		uint64_t budget;
	};


	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;
		virtual void Init() = 0;
		virtual void TempSecondaryInit() = 0;
		virtual void DrawFrame() = 0;
		virtual void OnImGuiRender() = 0;
		virtual void OnFinish() = 0;
		virtual std::any GetContextInfo(const std::string& name) = 0;
		virtual void SetFramebufferResized(bool framebufferResized) = 0;
		virtual const RendererInfo& GetRendererInfo() const = 0;
		virtual void SetPushConstants(PushConstants& pushConstants) = 0;

		virtual void CalculateMemoryStats() = 0;
		virtual MemoryStats GetMemoryStats() = 0;

		virtual uint64_t GetVBufferOffset(uint32_t index) = 0;
		virtual uint64_t GetIBufferOffset(uint32_t index) = 0;

		static GraphicsContext* Create(void* windowHandle);

	private:

	};
}