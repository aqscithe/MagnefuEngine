#pragma once

// -- Application Includes ---------//
#include "Event.h"

// -- Graphics Includes --------------------- //


// -- Core Includes ---------------------------------- //



namespace Magnefu
{
	class  KeyEvent : public Event
	{
	public:
		inline int GetKeyCode() const { return m_KeyCode; }
		inline int GetScancode() const { return m_ScanCode; }
		inline int GetAction() const { return m_Action; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

	protected:
		KeyEvent(int keyCode, int scancode, int action) :
			m_KeyCode(keyCode), m_ScanCode(scancode), m_Action(action) {}

		int m_KeyCode;
		int m_ScanCode;
		int m_Action;
		
	};

	class  KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(int keyCode, int scancode, int action, bool hasRepeated_) :
			KeyEvent(keyCode, scancode, action), hasRepeated(hasRepeated_) {} //m_RepeatCount(repeatCount) {}

		inline bool HasRepeated() { return hasRepeated; }

		std::string  ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " | Repeated? " << hasRepeated;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)

	private:
		bool hasRepeated = false;
		//int m_RepeatCount;
	};

	class  KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(int keyCode, int scancode, int action) :
			KeyEvent(keyCode, scancode, action) {}

		std::string  ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	};
}