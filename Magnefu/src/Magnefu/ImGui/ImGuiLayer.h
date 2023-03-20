#pragma once

#include "Magnefu/Layer.h"
#include "Magnefu/Events/KeyEvent.h"
#include "Magnefu/Events/MouseEvent.h"
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

		bool OnKeyPressed(KeyPressedEvent& event);
		bool OnKeyReleased(KeyReleasedEvent& event);
		void OnMouseButtonPressed(MouseButtonPressedEvent& event);
		bool OnMouseButtonReleased(MouseButtonReleasedEvent& event);

		void BeginFrame();
		void EndFrame();

	private:
		Renderer m_Renderer;
	};
}