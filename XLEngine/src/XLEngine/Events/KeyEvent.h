#pragma once

#include "Event.h"
#include "XLEngine/Core/KeyCodes.h"

namespace XLEngine
{
	class XLENGINE_API KeyEvent :public Event
	{
	public:
		KeyCode GetKeyCode() const { return m_KeyCode; }
		EVENT_CLASS_CATEGORY(EventCategoryKeyboard|EventCategoryInput)
	protected:
		KeyEvent(KeyCode key)
			:m_KeyCode(key) {}
		KeyCode m_KeyCode;
	};

	class XLENGINE_API KeyPressedEvent :public KeyEvent
	{
	public:
		KeyPressedEvent(const KeyCode key, const uint16_t repeatCount)
			:KeyEvent(key) ,m_RepeatCount(repeatCount) {}

		uint16_t GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << "(" << m_RepeatCount << " repeats)";
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyPressed)
	private:
		uint16_t m_RepeatCount;
	};

	class XLENGINE_API KeyReleasedEvent :public KeyEvent
	{
	public:
		KeyReleasedEvent(const KeyCode key)
			:KeyEvent(key) {}
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyReleased)
	};

	class XLENGINE_API KeyTypedEvent :public KeyEvent
	{
	public:
		KeyTypedEvent(const KeyCode keycode)
			: KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent" << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}