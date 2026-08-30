#pragma once

#include "Runtime/Renderer/EditorCamera.h"
#include "Runtime/Core/Timestep.h"
#include "Runtime/Events/Event.h"
#include "Runtime/Events/MouseEvent.h"

namespace XLEngine
{
	// 编辑器相机控制器（功能层）：负责采集 Orbit/Fly 的用户输入并驱动 EditorCamera，
	// 使相机本体只保留状态与数学。输入读取在此处集中，不依赖 Application 单例。
	class CameraController
	{
	public:
		CameraController() = default;
		CameraController(float fov, float aspectRatio, float nearClip, float farClip);

		// 每帧采集输入并更新相机
		void OnUpdate(Timestep ts);
		// 分发与相机相关的窗口事件（如鼠标滚轮）
		void OnEvent(Event& e);

		[[nodiscard]] EditorCamera& GetCamera() { return m_Camera; }
		[[nodiscard]] const EditorCamera& GetCamera() const { return m_Camera; }

		void SetViewportSize(float width, float height) { m_Camera.SetViewportSize(width, height); }
		void SetFlyMode(bool enabled);
		[[nodiscard]] inline bool IsFlyMode() const { return m_Camera.IsFlyMode(); }
		void SetViewportActive(bool active) { m_Camera.SetViewportActive(active); }
	private:
		void UpdateOrbit(Timestep ts);
		void UpdateFly(Timestep ts);

		bool OnMouseScroll(MouseScrolledEvent& e);
	private:
		EditorCamera m_Camera;

		glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };
		glm::vec2 m_LastMousePosition = { 0.0f, 0.0f };
	};
}