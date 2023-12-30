#pragma once

// -- Application Includes ---------//
#include "Layer.h"

// -- Graphics Includes --------------------- //


// -- Core Includes ---------------------------------- //
#include "Magnefu/Core/Assertions.h"

#include <vector>

namespace Magnefu
{
	class  LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		Layer** begin() { return m_Layers.begin(); }
		Layer** end() { return m_Layers.end(); }

	private:
		Array<Layer*> m_Layers;
		uint32_t m_LayerInsertIndex = 0;
	};
}