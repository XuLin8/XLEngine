#pragma once

#include "Runtime/Input/Input.h"
#include "Runtime/Input/KeyCodes.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace XLEngine
{
	// P1-2 输入动作映射：把具体按键抽象为「动作名」。
	// 脚本/玩法只需查询动作是否激活，不直接依赖物理按键，便于后续改键与跨设备。
	// 一个动作可绑定多个按键（如 WASD 与方向键），任一按下即视为激活。
	class InputActionMapper
	{
	public:
		using ActionName = std::string;

		static InputActionMapper& Get()
		{
			static InputActionMapper instance;
			return instance;
		}

		// 绑定：动作名 + 一个按键（追加）
		void Map(const ActionName& action, KeyCode key)
		{
			m_Bindings[action].push_back(key);
		}

		// 解除某动作的全部绑定
		void Unmap(const ActionName& action)
		{
			m_Bindings.erase(action);
		}

		void ClearAll()
		{
			m_Bindings.clear();
		}

		// 动作是否为激活态（任一绑定键按下）
		[[nodiscard]] bool IsPressed(const ActionName& action) const
		{
			auto it = m_Bindings.find(action);
			if (it == m_Bindings.end())
				return false;
			for (KeyCode key : it->second)
			{
				if (Input::IsKeyPressed(key))
					return true;
			}
			return false;
		}

		// 极轴量：按住 positive 得 +1，按住 negative 得 -1，同时按住为 0。
		// 供「左右/前后」这类二值轴使用。
		[[nodiscard]] float GetAxisPolar(const ActionName& positive, const ActionName& negative) const
		{
			bool p = IsPressed(positive);
			bool n = IsPressed(negative);
			return (p ? 1.0f : 0.0f) - (n ? 1.0f : 0.0f);
		}

		// 默认绑定：移动/交互（WASD + 方向键 + E/空格）
		void LoadDefaultBindings()
		{
			Unmap("MoveForward");
			Unmap("MoveBackward");
			Unmap("MoveLeft");
			Unmap("MoveRight");
			Unmap("Interact");
			Unmap("Restart");

			Map("MoveForward",  Key::W);
			Map("MoveForward",  Key::Up);
			Map("MoveBackward", Key::S);
			Map("MoveBackward", Key::Down);
			Map("MoveLeft",     Key::A);
			Map("MoveLeft",     Key::Left);
			Map("MoveRight",    Key::D);
			Map("MoveRight",    Key::Right);
			Map("Interact",     Key::E);
			Map("Interact",     Key::Space);
			Map("Restart",      Key::R);
		}

	private:
		InputActionMapper() = default;
		std::unordered_map<ActionName, std::vector<KeyCode>> m_Bindings;
	};
}