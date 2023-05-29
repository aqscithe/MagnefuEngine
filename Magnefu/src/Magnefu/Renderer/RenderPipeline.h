#pragma once


namespace Magnefu
{
	class RenderPipeline
	{
	public:
		virtual ~RenderPipeline() = default;

		static Ref<RenderPipeline> Create(int temp = 1);
	};
}