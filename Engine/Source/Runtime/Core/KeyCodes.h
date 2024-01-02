#pragma once

namespace XLEngine
{
	typedef enum class KeyCode : uint16_t
	{
		// From glfw3.h
		Space = 32,
		Apostrophe = 39, /* ' */
		Comma = 44, /* , */
		Minus = 45, /* - */
		Period = 46, /* . */
		Slash = 47, /* / */

		D0 = 48, /* 0 */
		D1 = 49, /* 1 */
		D2 = 50, /* 2 */
		D3 = 51, /* 3 */
		D4 = 52, /* 4 */
		D5 = 53, /* 5 */
		D6 = 54, /* 6 */
		D7 = 55, /* 7 */
		D8 = 56, /* 8 */
		D9 = 57, /* 9 */

		Semicolon = 59, /* ; */
		Equal = 61, /* = */

		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		LeftBracket = 91,  /* [ */
		Backslash = 92,  /* \ */
		RightBracket = 93,  /* ] */
		GraveAccent = 96,  /* ` */

		World1 = 161, /* non-US #1 */
		World2 = 162, /* non-US #2 */

		/* Function keys */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,

		/* Keypad */
		KP0 = 320,
		KP1 = 321,
		KP2 = 322,
		KP3 = 323,
		KP4 = 324,
		KP5 = 325,
		KP6 = 326,
		KP7 = 327,
		KP8 = 328,
		KP9 = 329,
		KPDecimal = 330,
		KPDivide = 331,
		KPMultiply = 332,
		KPSubtract = 333,
		KPAdd = 334,
		KPEnter = 335,
		KPEqual = 336,

		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348
	} Key;

	inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << static_cast<int32_t>(keyCode);
		return os;
	}
}

// From glfw3.h
#define XL_KEY_SPACE           ::XLEngine::Key::Space
#define XL_KEY_APOSTROPHE      ::XLEngine::Key::Apostrophe    /* ' */
#define XL_KEY_COMMA           ::XLEngine::Key::Comma         /* , */
#define XL_KEY_MINUS           ::XLEngine::Key::Minus         /* - */
#define XL_KEY_PERIOD          ::XLEngine::Key::Period        /* . */
#define XL_KEY_SLASH           ::XLEngine::Key::Slash         /* / */
#define XL_KEY_0               ::XLEngine::Key::D0
#define XL_KEY_1               ::XLEngine::Key::D1
#define XL_KEY_2               ::XLEngine::Key::D2
#define XL_KEY_3               ::XLEngine::Key::D3
#define XL_KEY_4               ::XLEngine::Key::D4
#define XL_KEY_5               ::XLEngine::Key::D5
#define XL_KEY_6               ::XLEngine::Key::D6
#define XL_KEY_7               ::XLEngine::Key::D7
#define XL_KEY_8               ::XLEngine::Key::D8
#define XL_KEY_9               ::XLEngine::Key::D9
#define XL_KEY_SEMICOLON       ::XLEngine::Key::Semicolon     /* ; */
#define XL_KEY_EQUAL           ::XLEngine::Key::Equal         /* = */
#define XL_KEY_A               ::XLEngine::Key::A
#define XL_KEY_B               ::XLEngine::Key::B
#define XL_KEY_C               ::XLEngine::Key::C
#define XL_KEY_D               ::XLEngine::Key::D
#define XL_KEY_E               ::XLEngine::Key::E
#define XL_KEY_F               ::XLEngine::Key::F
#define XL_KEY_G               ::XLEngine::Key::G
#define XL_KEY_H               ::XLEngine::Key::H
#define XL_KEY_I               ::XLEngine::Key::I
#define XL_KEY_J               ::XLEngine::Key::J
#define XL_KEY_K               ::XLEngine::Key::K
#define XL_KEY_L               ::XLEngine::Key::L
#define XL_KEY_M               ::XLEngine::Key::M
#define XL_KEY_N               ::XLEngine::Key::N
#define XL_KEY_O               ::XLEngine::Key::O
#define XL_KEY_P               ::XLEngine::Key::P
#define XL_KEY_Q               ::XLEngine::Key::Q
#define XL_KEY_R               ::XLEngine::Key::R
#define XL_KEY_S               ::XLEngine::Key::S
#define XL_KEY_T               ::XLEngine::Key::T
#define XL_KEY_U               ::XLEngine::Key::U
#define XL_KEY_V               ::XLEngine::Key::V
#define XL_KEY_W               ::XLEngine::Key::W
#define XL_KEY_X               ::XLEngine::Key::X
#define XL_KEY_Y               ::XLEngine::Key::Y
#define XL_KEY_Z               ::XLEngine::Key::Z
#define XL_KEY_LEFT_BRACKET    ::XLEngine::Key::LeftBracket   /* [ */
#define XL_KEY_BACKSLASH       ::XLEngine::Key::Backslash     /* \ */
#define XL_KEY_RIGHT_BRACKET   ::XLEngine::Key::RightBracket  /* ] */
#define XL_KEY_GRAVE_ACCENT    ::XLEngine::Key::GraveAccent   /* ` */
#define XL_KEY_WORLD_1         ::XLEngine::Key::World1        /* non-US #1 */
#define XL_KEY_WORLD_2         ::XLEngine::Key::World2        /* non-US #2 */

/* Function keys */
#define XL_KEY_ESCAPE          ::XLEngine::Key::Escape
#define XL_KEY_ENTER           ::XLEngine::Key::Enter
#define XL_KEY_TAB             ::XLEngine::Key::Tab
#define XL_KEY_BACKSPACE       ::XLEngine::Key::Backspace
#define XL_KEY_INSERT          ::XLEngine::Key::Insert
#define XL_KEY_DELETE          ::XLEngine::Key::Delete
#define XL_KEY_RIGHT           ::XLEngine::Key::Right
#define XL_KEY_LEFT            ::XLEngine::Key::Left
#define XL_KEY_DOWN            ::XLEngine::Key::Down
#define XL_KEY_UP              ::XLEngine::Key::Up
#define XL_KEY_PAGE_UP         ::XLEngine::Key::PageUp
#define XL_KEY_PAGE_DOWN       ::XLEngine::Key::PageDown
#define XL_KEY_HOME            ::XLEngine::Key::Home
#define XL_KEY_END             ::XLEngine::Key::End
#define XL_KEY_CAPS_LOCK       ::XLEngine::Key::CapsLock
#define XL_KEY_SCROLL_LOCK     ::XLEngine::Key::ScrollLock
#define XL_KEY_NUM_LOCK        ::XLEngine::Key::NumLock
#define XL_KEY_PRINT_SCREEN    ::XLEngine::Key::PrintScreen
#define XL_KEY_PAUSE           ::XLEngine::Key::Pause
#define XL_KEY_F1              ::XLEngine::Key::F1
#define XL_KEY_F2              ::XLEngine::Key::F2
#define XL_KEY_F3              ::XLEngine::Key::F3
#define XL_KEY_F4              ::XLEngine::Key::F4
#define XL_KEY_F5              ::XLEngine::Key::F5
#define XL_KEY_F6              ::XLEngine::Key::F6
#define XL_KEY_F7              ::XLEngine::Key::F7
#define XL_KEY_F8              ::XLEngine::Key::F8
#define XL_KEY_F9              ::XLEngine::Key::F9
#define XL_KEY_F10             ::XLEngine::Key::F10
#define XL_KEY_F11             ::XLEngine::Key::F11
#define XL_KEY_F12             ::XLEngine::Key::F12
#define XL_KEY_F13             ::XLEngine::Key::F13
#define XL_KEY_F14             ::XLEngine::Key::F14
#define XL_KEY_F15             ::XLEngine::Key::F15
#define XL_KEY_F16             ::XLEngine::Key::F16
#define XL_KEY_F17             ::XLEngine::Key::F17
#define XL_KEY_F18             ::XLEngine::Key::F18
#define XL_KEY_F19             ::XLEngine::Key::F19
#define XL_KEY_F20             ::XLEngine::Key::F20
#define XL_KEY_F21             ::XLEngine::Key::F21
#define XL_KEY_F22             ::XLEngine::Key::F22
#define XL_KEY_F23             ::XLEngine::Key::F23
#define XL_KEY_F24             ::XLEngine::Key::F24
#define XL_KEY_F25             ::XLEngine::Key::F25

/* Keypad */
#define XL_KEY_KP_0            ::XLEngine::Key::KP0
#define XL_KEY_KP_1            ::XLEngine::Key::KP1
#define XL_KEY_KP_2            ::XLEngine::Key::KP2
#define XL_KEY_KP_3            ::XLEngine::Key::KP3
#define XL_KEY_KP_4            ::XLEngine::Key::KP4
#define XL_KEY_KP_5            ::XLEngine::Key::KP5
#define XL_KEY_KP_6            ::XLEngine::Key::KP6
#define XL_KEY_KP_7            ::XLEngine::Key::KP7
#define XL_KEY_KP_8            ::XLEngine::Key::KP8
#define XL_KEY_KP_9            ::XLEngine::Key::KP9
#define XL_KEY_KP_DECIMAL      ::XLEngine::Key::KPDecimal
#define XL_KEY_KP_DIVIDE       ::XLEngine::Key::KPDivide
#define XL_KEY_KP_MULTIPLY     ::XLEngine::Key::KPMultiply
#define XL_KEY_KP_SUBTRACT     ::XLEngine::Key::KPSubtract
#define XL_KEY_KP_ADD          ::XLEngine::Key::KPAdd
#define XL_KEY_KP_ENTER        ::XLEngine::Key::KPEnter
#define XL_KEY_KP_EQUAL        ::XLEngine::Key::KPEqual

#define XL_KEY_LEFT_SHIFT      ::XLEngine::Key::LeftShift
#define XL_KEY_LEFT_CONTROL    ::XLEngine::Key::LeftControl
#define XL_KEY_LEFT_ALT        ::XLEngine::Key::LeftAlt
#define XL_KEY_LEFT_SUPER      ::XLEngine::Key::LeftSuper
#define XL_KEY_RIGHT_SHIFT     ::XLEngine::Key::RightShift
#define XL_KEY_RIGHT_CONTROL   ::XLEngine::Key::RightControl
#define XL_KEY_RIGHT_ALT       ::XLEngine::Key::RightAlt
#define XL_KEY_RIGHT_SUPER     ::XLEngine::Key::RightSuper
#define XL_KEY_MENU            ::XLEngine::Key::Menu