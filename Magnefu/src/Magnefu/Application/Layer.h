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
		Layer(cstring name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(double deltaTime) {}
		virtual void OnRender() {}
		virtual void OnGUIRender() {}
		virtual void OnEvent(Event& event) {}

		inline cstring GetName() const { return m_DebugName; }
	protected:
		cstring m_DebugName;
		bool m_Enabled;

	};
}