#pragma once

#include "Event.h"

#include <sstream>


namespace Magnefu
{
	class  GamepadConnectedEvent : public Event
	{
	public:
		GamepadConnectedEvent(int index) : m_Index(index) {}

		inline int GetIndex() { return m_Index; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "GamepadConnectedEvent";
			return ss.str();
		}

		EVENT_CLASS_TYPE(GamepadConnected)
		EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryGamepad)

	private:
		int m_Index;
	};

	class  GamepadDisconnectedEvent : public Event
	{
	public:
		GamepadDisconnectedEvent(int index) : m_Index(index) {}

		inline int GetIndex() { return m_Index; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "GamepadDisconnectedEvent";
			return ss.str();
		}

		EVENT_CLASS_TYPE(GamepadDisconnected)
		EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryGamepad)

	private:
		int m_Index;
	};
}