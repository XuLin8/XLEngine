#pragma once 

#include "Event.h"
#include "Runtime/Input/MouseCodes.h"

namespace XLEngine {
	class XLENGINE_API MouseMovedEvent :public Event
	{
	public:
		MouseMovedEvent(const float x, const float y)
			:m_MouseX(x), m_MouseY(y){}
		[[nodiscard]] inline float GetX() const { return m_MouseX; }
		[[nodiscard]] inline float GetY() const { return m_MouseY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}
		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_MouseX, m_MouseY;
	};

	class XLENGINE_API MouseScrolledEvent :public Event
	{
	public:
		MouseScrolledEvent(const float xOffset, const float yOffset)
			:m_XOffset(xOffset),m_YOffset(yOffset){}
		[[nodiscard]] inline float GetXOffset() const { return m_XOffset; }
		[[nodiscard]] inline float GetYOffset() const { return m_YOffset; }
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << m_XOffset << ", " << m_YOffset;
			return ss.str();
		}
		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput);

	private:
		float m_XOffset, m_YOffset;

	};

	class XLENGINE_API MouseButtonEvent :public Event
	{
	public:
		MouseCode GetMouseButton() const { return m_Button; }
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput);
	protected:
		MouseButtonEvent(const MouseCode button)
			:m_Button(button){}
		MouseCode m_Button;
		
	};

	class XLENGINE_API MouseButtonPressedEvent :public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(const MouseCode button)
			:MouseButtonEvent(button) {}
		std::string ToString() const override {
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}
		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class XLENGINE_API MouseButtonReleasedEvent :public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(const MouseCode button)
			:MouseButtonEvent(button) {}
		std::string ToString() const override {
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}
		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
}