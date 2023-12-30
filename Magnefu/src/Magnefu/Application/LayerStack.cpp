// -- PCH -- //
#include "mfpch.h"

// -- .h -- //
#include "LayerStack.h"

// -- Application Includes ---------//


// -- Graphics Includes --------------------- //


// -- Core Includes ---------------------------------- //



namespace Magnefu
{
	LayerStack::LayerStack()
	{
		m_Layers.init(&MemoryService::Instance()->systemAllocator, 2);
	}

	LayerStack::~LayerStack()
	{
		m_Layers.shutdown();
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		m_Layers.push(layer);
		m_LayerInsertIndex++;
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		m_Layers.push(overlay);
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		u32 index = m_Layers.get_index(layer);
		m_Layers.delete_swap(index);
		m_LayerInsertIndex--;
		
	}

	void LayerStack::PopOverlay(Layer* overlay)
	{
		u32 index = m_Layers.get_index(overlay);
		m_Layers.delete_swap(index);
	}


}