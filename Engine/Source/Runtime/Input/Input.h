#pragma once 

#include <glm/glm.hpp>
#include "Runtime/Input/KeyCodes.h"
#include "Runtime/Input/MouseCodes.h"

namespace XLEngine
{
	class Window;

	//这个类提供抽象的接口，用于处理输入设备的输入事件，在应用程序中获取按键状态和鼠标位置等信息
	//具体的输入处理逻辑位于平台层（WindowsInput.cpp），并且不依赖 Application 单例，
	//窗口指针由应用框架层在初始化时通过 SetWindow 注入（下方依赖方向约束，避免功能层反向依赖上层单例）
	class XLENGINE_API Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);

		static bool IsMouseButtonPressed(MouseCode button);
		static glm::vec2 GetMousePosition();
		[[nodiscard]] static float GetMouseX();
		[[nodiscard]] static float GetMouseY();

		// 由应用框架层在创建窗口后注入窗口指针
		static void SetWindow(Window* window);
	private:
		static Window* s_Window;
	};
}