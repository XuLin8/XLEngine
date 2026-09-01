#pragma once

#include "Runtime/Input/KeyCodes.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace XLEngine
{
	// 动作值维度。
	enum class InputActionType : uint32_t
	{
		Bool,   // 布尔：IsHeld / WasPressed / WasReleased
		Axis1D, // 一维轴值 [-1, 1]
		Axis2D, // 二维轴值 glm::vec2
	};

	// 触发方式。
	enum class InputTrigger : uint32_t
	{
		Hold,      // 按住即生效（移动/持续类）
		Tap,       // 按下边沿触发一次（防按住连发）
		DoubleTap, // 两次按下（限帧窗口内）才触发
	};

	// 单条按键绑定。不同动作类型取用不同槽位：
	//   Bool   -> Key                        (+Modifier)
	//   Axis1D -> Key(正) / NegativeKey(负)  (+Modifier)
	//   Axis2D -> Key(X+) / NegativeKey(X-) / PositiveY(Y+) / NegativeY(Y-)
	struct InputKeyBinding
	{
		KeyCode Key         = (KeyCode)0; // 主键（Bool/Axis1D 正/Axis2D X+）
		KeyCode NegativeKey = (KeyCode)0; // Axis1D 负 | Axis2D X-
		KeyCode PositiveY   = (KeyCode)0; // Axis2D Y+
		KeyCode NegativeY   = (KeyCode)0; // Axis2D Y-
		KeyCode Modifier    = (KeyCode)0; // 可选修饰键（按住才生效）
	};

	// 一个逻辑动作：名字 + 维度 + 触发 + 所属上下文 + 一组绑定（任一命中即激活）。
	struct InputAction
	{
		std::string Name;
		InputActionType Type    = InputActionType::Bool;
		InputTrigger  Trigger    = InputTrigger::Hold;
		std::string   Context    = "Gameplay";
		std::vector<InputKeyBinding> Bindings;
	};
}