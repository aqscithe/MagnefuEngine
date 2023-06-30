#pragma once


namespace Magnefu
{
	using String = std::string;

	struct PushConstants
	{
		alignas(16) Maths::vec3 Tint;
		alignas(16) Maths::vec3 CameraPos;
		alignas(16) Maths::vec3 LightPos;
		alignas(16) Maths::vec3 LightColor;
		alignas(16) Maths::vec3 Ka = Maths::vec3(0.0);
		alignas(16) Maths::vec3 Kd = Maths::vec3(1.0);
		alignas(16) Maths::vec3 Ks = Maths::vec3(1.0);
		float                   Opacity;
		float                   RadiantFlux;
		float                   Reflectance = 0.5; // fresnel reflectance for dielectrics [0.0, 1.0]
		int                     LightEnabled;
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
		virtual void DrawFrame() = 0;
		virtual void OnImGuiRender() = 0;
		virtual void OnFinish() = 0;
		virtual std::any GetContextInfo(const std::string& name) = 0;
		virtual void SetFramebufferResized(bool framebufferResized) = 0;
		virtual const RendererInfo& GetRendererInfo() const = 0;
		virtual void SetPushConstants(PushConstants& pushConstants) = 0;

		static GraphicsContext* Create(void* windowHandle);

	private:

	};
}