#pragma once

// -- Application Includes ---------//
#include "Event.h"

// -- Graphics Includes --------------------- //


// -- Core Includes ---------------------------------- //




namespace Magnefu {
	
	class  MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y) :
			m_MouseX(x), m_MouseY(y) {}

		inline float GetMouseX() const { return m_MouseX; }
		inline float GetMouseY() const { return m_MouseY; }

		std::string  ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: X" << m_MouseX << " | Repeat Count: " << m_MouseY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_MouseX;
		float m_MouseY;
	};

	class  MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xOffset, float yOffset) :
			m_XOffset(xOffset), m_YOffset(yOffset) {}

		inline float GetXOffset() const { return m_XOffset; }
		inline float GetYOffset() const { return m_YOffset; }

		std::string  ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: X Offset " << GetXOffset() << " | Y Offset " << GetYOffset();
			return ss.str();

		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_XOffset;
		float m_YOffset;

	};

	class  MouseButtonEvent : public Event
	{
	public:
		MouseButtonEvent(int buttonCode) : 
			m_ButtonCode(buttonCode) {}

		inline int GetButtonCode() const { return m_ButtonCode; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		int m_ButtonCode;
	};

	class  MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int buttonCode, int repeatCount) :
			MouseButtonEvent(buttonCode), m_RepeatCount(repeatCount) {}

		inline int GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent:  Button Code " << GetButtonCode() << " | Repeat Count " << GetRepeatCount();
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	private:
		int m_RepeatCount;

	};

	class  MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(int buttonCode) :
			MouseButtonEvent(buttonCode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent:  Button Code " << GetButtonCode();
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	private:
	};
}