#pragma once 

#include <glm/glm.hpp>
#include "Runtime/Input/KeyCodes.h"
#include "Runtime/Input/MouseCodes.h"

namespace XLEngine
{
	//这个类提供抽象的接口，用于处理输入设备的输入事件，在应用程序中获取按键状态和鼠标位置等信息
	//具体的输入处理逻辑需要在派生类中实现
	class XLENGINE_API Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);

		static bool IsMouseButtonPressed(MouseCode button);
		static glm::vec2 GetMousePosition();
		[[nodiscard]] static float GetMouseX();
		[[nodiscard]] static float GetMouseY();
	};
}