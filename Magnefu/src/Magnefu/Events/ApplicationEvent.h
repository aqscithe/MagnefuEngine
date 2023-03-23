#pragma once

#include "Event.h"

namespace Magnefu
{
	class MAGNEFU_API WindowCloseEvent : public Event
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

	class MAGNEFU_API WindowResizeEvent : public Event
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

	class MAGNEFU_API WindowMovedEvent : public Event
	{

	};


	class MAGNEFU_API WindowFocusEvent : public Event
	{

	};

	class MAGNEFU_API WindowLostFocusEvent : public Event
	{

	};
}