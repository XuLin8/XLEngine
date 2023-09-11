#pragma once

#include "Event.h"

namespace XLEngine
{
	class XLENGINE_API KeyEvent :public Event
	{
	public:
		inline int GetKeyCode() const { return m_KeyCode; }
		EVENT_CLASS_CATEGORY(EventCategoryKeyboard|EventCategoryInput)
	protected:
		KeyEvent(int key)
			:m_KeyCode(key) {}
		int m_KeyCode;
	};

	class XLENGINE_API KeyPressedEvent :public KeyEvent
	{
	public:
		KeyPressedEvent(int key,int repeatCount)
			:KeyEvent(key) ,m_RepeatCount(repeatCount) {}
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << "(" << m_RepeatCount << " repeats)";
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyPressed)
	private:
		int m_RepeatCount;
	};

	class XLENGINE_API KeyReleasedEvent :public KeyEvent
	{
	public:
		KeyReleasedEvent(int key)
			:KeyEvent(key) {}
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyReleased)
	};
}