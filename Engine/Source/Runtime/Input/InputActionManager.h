#pragma once

#include "Runtime/Input/InputActionTypes.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <unordered_map>

namespace XLEngine
{
	// 统一输入动作管理（阶段 D：输入绑定迁移）。
	//
	// 特性与设计约束：
	//  - 每帧由 App 层调用 UpdateFrame() 对绑定键做“边沿采样”，
	//    提供 WasPressed / WasReleased / IsHeld 三态，取代脚本里手写边沿。
	//  - 动作维度 Bool / Axis1D / Axis2D，触发 Hold / Tap / DoubleTap，支持修饰键（如 Shift 组合）。
	//  - 上下文基于栈（栈顶优先级最高），区分 Gameplay / Editor / UI；
	//    动作仅在所属上下文处于激活栈中才被求值，天然分开玩法与编辑器输入。
	//  - 不依赖 Application 单例：按键状态经功能层 Input 读取（窗口指针由 App 层注入）。
	class InputActionManager
	{
	public:
		static InputActionManager& Get()
		{
			static InputActionManager instance;
			return instance;
		}

		// ---- 动作 ----
		void AddAction(const InputAction& action);
		void RemoveAction(const std::string& name);
		void ClearActions();

		// ---- 上下文栈 ----
		void PushContext(const std::string& context);
		void PopContext();
		void ClearContexts();
		void SetBaseContext(const std::string& context);
		[[nodiscard]] bool IsContextActive(const std::string& context) const;
		[[nodiscard]] const std::string& TopContext() const { return m_ContextStack.empty() ? m_BaseContext : m_ContextStack.back(); }

		// 每帧调用一次（挂在 Application::Run 帧循环首部）
		void UpdateFrame();

		// ---- 查询 ----
		[[nodiscard]] bool HasAction(const std::string& name) const { return m_Actions.find(name) != m_Actions.end(); }
		[[nodiscard]] bool IsHeld(const std::string& name) const;
		[[nodiscard]] bool WasPressed(const std::string& name) const;
		[[nodiscard]] bool WasReleased(const std::string& name) const;
		[[nodiscard]] float GetAxis(const std::string& name) const;       // Axis1D: [-1,1]
		[[nodiscard]] glm::vec2 GetAxis2D(const std::string& name) const; // Axis2D

	private:
		const InputAction* FindAction(const std::string& name) const;
		bool ActionActive(const InputAction& a) const { return IsContextActive(a.Context); }
		// 绑定外键满足 + 修饰键满足
		bool BindingEnabled(const InputKeyBinding& b) const;
		// 主键边沿（简化：作用于 primary Key 槽）
		bool PrimaryPressedEdge(const InputKeyBinding& b) const;

		std::unordered_map<std::string, InputAction> m_Actions;

		std::vector<std::string> m_ContextStack;
		std::string m_BaseContext = "Gameplay";

		// 每帧键状态缓存
		std::unordered_map<int, bool> m_KeyHeld;        // 当前是否按住
		std::unordered_map<int, bool> m_KeyPressedEdge; // 本帧新按下
		std::unordered_map<int, bool> m_KeyReleasedEdge;// 本帧新释放
		// DoubleTap：每位按键的上一次按下帧 + 本轮判定
		std::unordered_map<int, uint64_t> m_KeyLastPressFrame;
		std::unordered_map<int, bool> m_KeyDoubleTapEdge;

		uint64_t m_Frame = 0;
		static constexpr uint64_t kTapRepeatFrames = 8;
	};
}