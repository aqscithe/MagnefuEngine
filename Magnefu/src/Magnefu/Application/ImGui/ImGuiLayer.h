#pragma once

// -- Application Includes ---------//
#include "ImGuiDefs.h"
#include "Magnefu/Application/Layer.h"
#include "Magnefu/Application/Events/KeyEvent.h"
#include "Magnefu/Application/Events/MouseEvent.h"

// -- Graphics Includes --------------------- //


// -- Core Includes ---------------------------------- //



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
		void RecordAndSubmitCommandBuffer(uint32_t imageIndex);

		void BlockEvents(bool block) { m_BlockEvents = block; }

		void SetDarkThemeColors();

		uint32_t GetActiveWidgetID() const;


	private:
		
		bool m_BlockEvents = true;
		
	};
}