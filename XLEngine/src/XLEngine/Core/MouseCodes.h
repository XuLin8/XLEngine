#pragma once

namespace XLEngine
{
	typedef enum class MouseCode : uint16_t
	{
		// From glfw3.h
		Button0 = 0,
		Button1 = 1,
		Button2 = 2,
		Button3 = 3,
		Button4 = 4,
		Button5 = 5,
		Button6 = 6,
		Button7 = 7,

		ButtonLast = Button7,
		ButtonLeft = Button0,
		ButtonRight = Button1,
		ButtonMiddle = Button2
	} Mouse;

	inline std::ostream& operator<<(std::ostream& os, MouseCode mouseCode)
	{
		os << static_cast<int32_t>(mouseCode);
		return os;
	}
}

#define XL_MOUSE_BUTTON_0      ::XLEngine::Mouse::Button0
#define XL_MOUSE_BUTTON_1      ::XLEngine::Mouse::Button1
#define XL_MOUSE_BUTTON_2      ::XLEngine::Mouse::Button2
#define XL_MOUSE_BUTTON_3      ::XLEngine::Mouse::Button3
#define XL_MOUSE_BUTTON_4      ::XLEngine::Mouse::Button4
#define XL_MOUSE_BUTTON_5      ::XLEngine::Mouse::Button5
#define XL_MOUSE_BUTTON_6      ::XLEngine::Mouse::Button6
#define XL_MOUSE_BUTTON_7      ::XLEngine::Mouse::Button7
#define XL_MOUSE_BUTTON_LAST   ::XLEngine::Mouse::ButtonLast
#define XL_MOUSE_BUTTON_LEFT   ::XLEngine::Mouse::ButtonLeft
#define XL_MOUSE_BUTTON_RIGHT  ::XLEngine::Mouse::ButtonRight
#define XL_MOUSE_BUTTON_MIDDLE ::XLEngine::Mouse::ButtonMiddle