#include "xlpch.h"

#include "Runtime/Input/InputActionManager.h"
#include "Runtime/Input/Input.h"

#include <algorithm>

namespace XLEngine
{
	const InputAction* InputActionManager::FindAction(const std::string& name) const
	{
		auto it = m_Actions.find(name);
		return it == m_Actions.end() ? nullptr : &it->second;
	}

	void InputActionManager::AddAction(const InputAction& action)
	{
		m_Actions[action.Name] = action;
	}

	void InputActionManager::RemoveAction(const std::string& name)
	{
		m_Actions.erase(name);
	}

	void InputActionManager::ClearActions()
	{
		m_Actions.clear();
		ClearContexts();
	}

	void InputActionManager::PushContext(const std::string& context)
	{
		m_ContextStack.push_back(context);
	}

	void InputActionManager::PopContext()
	{
		if (!m_ContextStack.empty())
			m_ContextStack.pop_back();
	}

	void InputActionManager::ClearContexts()
	{
		m_ContextStack.clear();
	}

	void InputActionManager::SetBaseContext(const std::string& context)
	{
		m_BaseContext = context;
	}

	bool InputActionManager::IsContextActive(const std::string& context) const
	{
		if (context.empty())
			return true;
		if (context == m_BaseContext)
			return true;
		for (const auto& c : m_ContextStack)
			if (c == context)
				return true;
		return false;
	}

	bool InputActionManager::BindingEnabled(const InputKeyBinding& b) const
	{
		return b.Modifier == (KeyCode)0 || Input::IsKeyPressed(b.Modifier);
	}

	bool InputActionManager::PrimaryPressedEdge(const InputKeyBinding& b) const
	{
		auto it = m_KeyPressedEdge.find((int)b.Key);
		return it != m_KeyPressedEdge.end() && it->second;
	}

	void InputActionManager::UpdateFrame()
	{
		m_Frame++;

		// 收集所有动作涉及的键（含修饰键），去重后统一采样边沿。
		std::vector<int> keys;
		auto addKey = [&](KeyCode k)
		{
			if (k != (KeyCode)0)
				keys.push_back((int)k);
		};
		for (const auto& kv : m_Actions)
		{
			for (const auto& b : kv.second.Bindings)
			{
				addKey(b.Key);
				addKey(b.NegativeKey);
				addKey(b.PositiveY);
				addKey(b.NegativeY);
				addKey(b.Modifier);
			}
		}
		std::sort(keys.begin(), keys.end());
		keys.erase(std::unique(keys.begin(), keys.end()), keys.end());

		for (int key : keys)
		{
			const bool now = Input::IsKeyPressed((KeyCode)key);
			const bool prev = m_KeyHeld.count(key) ? m_KeyHeld[key] : false;

			const bool pressedEdge  = now && !prev;
			const bool releasedEdge = prev && !now;

			m_KeyPressedEdge[key]  = pressedEdge;
			m_KeyReleasedEdge[key] = releasedEdge;

			if (pressedEdge)
			{
				// DoubleTap 判定：本次按下距上一次按下是否落在帧窗口内。
				const uint64_t prevFrame = m_KeyLastPressFrame.count(key) ? m_KeyLastPressFrame[key] : 0;
				m_KeyDoubleTapEdge[key] = (prevFrame != 0 && m_Frame - prevFrame <= kTapRepeatFrames);
				m_KeyLastPressFrame[key] = m_Frame;
			}

			m_KeyHeld[key] = now;
		}
	}

	bool InputActionManager::IsHeld(const std::string& name) const
	{
		const InputAction* a = FindAction(name);
		if (!a || !ActionActive(*a))
			return false;
		for (const auto& b : a->Bindings)
		{
			if (BindingEnabled(b) && Input::IsKeyPressed(b.Key))
				return true;
		}
		return false;
	}

	bool InputActionManager::WasPressed(const std::string& name) const
	{
		const InputAction* a = FindAction(name);
		if (!a || !ActionActive(*a))
			return false;
		for (const auto& b : a->Bindings)
		{
			if (!BindingEnabled(b))
				continue;
			if (!PrimaryPressedEdge(b))
				continue;
			if (a->Trigger == InputTrigger::DoubleTap)
			{
				auto it = m_KeyDoubleTapEdge.find((int)b.Key);
				if (it == m_KeyDoubleTapEdge.end() || !it->second)
					continue;
			}
			return true;
		}
		return false;
	}

	bool InputActionManager::WasReleased(const std::string& name) const
	{
		const InputAction* a = FindAction(name);
		if (!a || !ActionActive(*a))
			return false;
		for (const auto& b : a->Bindings)
		{
			auto it = m_KeyReleasedEdge.find((int)b.Key);
			if (it != m_KeyReleasedEdge.end() && it->second)
				return true;
		}
		return false;
	}

	float InputActionManager::GetAxis(const std::string& name) const
	{
		const InputAction* a = FindAction(name);
		if (!a || a->Type != InputActionType::Axis1D || !ActionActive(*a))
			return 0.0f;

		float value = 0.0f;
		for (const auto& b : a->Bindings)
		{
			if (!BindingEnabled(b))
				continue;
			if (Input::IsKeyPressed(b.Key))        value += 1.0f;
			if (Input::IsKeyPressed(b.NegativeKey)) value -= 1.0f;
		}
		return glm::clamp(value, -1.0f, 1.0f);
	}

	glm::vec2 InputActionManager::GetAxis2D(const std::string& name) const
	{
		const InputAction* a = FindAction(name);
		if (!a || a->Type != InputActionType::Axis2D || !ActionActive(*a))
			return glm::vec2(0.0f);

		glm::vec2 value(0.0f);
		for (const auto& b : a->Bindings)
		{
			if (!BindingEnabled(b))
				continue;
			if (Input::IsKeyPressed(b.Key))        value.x += 1.0f;
			if (Input::IsKeyPressed(b.NegativeKey)) value.x -= 1.0f;
			if (Input::IsKeyPressed(b.PositiveY))  value.y += 1.0f;
			if (Input::IsKeyPressed(b.NegativeY))  value.y -= 1.0f;
		}
		value.x = glm::clamp(value.x, -1.0f, 1.0f);
		value.y = glm::clamp(value.y, -1.0f, 1.0f);
		return value;
	}
}