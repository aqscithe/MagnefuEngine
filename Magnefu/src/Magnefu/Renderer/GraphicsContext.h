#pragma once


namespace Magnefu
{
	using String = std::string;

	struct PushConstants
	{
		
	};

	struct RendererInfo
	{
		String Version;
		String Vendor;
		String Renderer;
	};

	class GraphicsContext
	{
	public:
		virtual void Init() = 0;
		virtual void TempSecondaryInit() = 0;
		virtual void DrawFrame() = 0;
		virtual void OnImGuiRender() = 0;
		virtual void OnFinish() = 0;
		virtual std::any GetContextInfo(const std::string& name) = 0;
		virtual void SetFramebufferResized(bool framebufferResized) = 0;
		virtual const RendererInfo& GetRendererInfo() const = 0;
		virtual void SetPushConstants(PushConstants& pushConstants) = 0;

		virtual uint64_t GetVBufferOffset(uint32_t index) = 0;
		virtual uint64_t GetIBufferOffset(uint32_t index) = 0;

		static GraphicsContext* Create(void* windowHandle);

	private:

	};
}