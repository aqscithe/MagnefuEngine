#pragma once

// -- Application Includes ---------//
#include "Events/Event.h"

// -- Graphics Includes --------------------- //


// -- Core Includes ---------------------------------- //
#include "Magnefu/Core/Assertions.h"


namespace Magnefu
{
	struct  Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(double deltaTime) {}
		virtual void OnRender() {}
		virtual void OnGUIRender() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
		bool m_Enabled;

	};
}