#pragma once

// -- Application Includes ---------//
#include "Event.h"

// -- Graphics Includes --------------------- //


// -- Core Includes ---------------------------------- //


// -- Other Includes --------------------------- //
#include <sstream>


namespace Magnefu
{
	class  WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowCloseEvent";
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApp)

	private:
	};

	class  WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(uint16_t width, uint16_t height) :
			m_Width(width), m_Height(height) {}

		inline uint16_t GetWidth() const { return m_Width; }
		inline uint16_t GetHeight() const { return m_Height; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent:  Width " << GetWidth() << " | Height " << GetHeight();
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApp)

	private:
		uint16_t m_Width, m_Height;
	};

	class  WindowMovedEvent : public Event
	{

	};


	class  WindowFocusEvent : public Event
	{

	};

	class  WindowLostFocusEvent : public Event
	{

	};
}