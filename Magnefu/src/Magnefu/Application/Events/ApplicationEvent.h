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
	public:
		WindowMovedEvent(int xpos, int ypos) :
			m_xpos(xpos), m_ypos(ypos) {}

		inline i32 GetXPosition() const { return m_xpos; }
		inline i32 GetYPosition() const { return m_ypos; }

		std::string ToString() const override
		{
			std::stringstream ss;

			ss << "WindowMovedEvent | ( x, y ) : ( " << GetXPosition() << ", " << GetYPosition() << " )";
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowMoved)
		EVENT_CLASS_CATEGORY(EventCategoryApp)

	private:
		i32 m_xpos, m_ypos;
	};


	class  WindowFocusEvent : public Event
	{
	public:
		WindowFocusEvent() {}


		std::string ToString() const override
		{
			std::stringstream ss;

			ss << "WindowFocusEvent";
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowFocus)
		EVENT_CLASS_CATEGORY(EventCategoryApp)

	};

	class  WindowLostFocusEvent : public Event
	{
	public:
		WindowLostFocusEvent() {}

		std::string ToString() const override
		{
			std::stringstream ss;

			ss << "WindowLostFocusEvent";
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowLostFocus)
		EVENT_CLASS_CATEGORY(EventCategoryApp)
	};
}