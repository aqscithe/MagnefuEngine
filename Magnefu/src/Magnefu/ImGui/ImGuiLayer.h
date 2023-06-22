#pragma once

#include "Magnefu/Layer.h"
#include "Magnefu/Core/Events/KeyEvent.h"
#include "Magnefu/Core/Events/MouseEvent.h"
#include "Magnefu/Renderer/Renderer.h"


namespace Magnefu
{
	class  ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach() override;
		
		void OnDetach() override;
		void OnEvent(Event&) override;
		void OnRender() override;

		void BeginFrame();
		void EndFrame();
		void RecreateImageResources();
		void RecordAndSubmitCommandBuffer();

		void BlockEvents(bool block) { m_BlockEvents = block; }

		void SetDarkThemeColors();

		uint32_t GetActiveWidgetID() const;


	private:
		Renderer m_Renderer;
		bool m_BlockEvents = true;
		
	};
}