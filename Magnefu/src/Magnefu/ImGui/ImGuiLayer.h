#pragma once

#include "Magnefu/Layer.h"
#include "Renderer.h"


namespace Magnefu
{
	class MAGNEFU_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(Event& event) override;

		void BeginFrame();
		void EndFrame();

	private:
		Renderer m_Renderer;
	};
}