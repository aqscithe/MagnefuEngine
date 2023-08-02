#pragma once


namespace Magnefu
{
	class RendererAPI
	{
	public:
		enum class API
		{
			NONE = 0,
			VULKAN = 1,
		};

	public:

		inline static API GetAPI() { return s_API; }


	private:
		static API s_API;
	};
}