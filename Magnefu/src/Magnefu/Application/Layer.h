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

		
		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void OnEvent(Event& event) = 0;

		virtual void Update(float deltaTime) = 0;
		virtual void DrawGUI() = 0;

		inline cstring GetName() const { return m_debug_name; }
	protected:
		cstring m_debug_name;
		bool m_enabled;

	};
}