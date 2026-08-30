#include "xlpch.h"
#include "CameraController.h"

#include "Runtime/Input/Input.h"
#include "Runtime/Input/KeyCodes.h"
#include "Runtime/Input/MouseCodes.h"

namespace XLEngine
{
	CameraController::CameraController(float fov, float aspectRatio, float nearClip, float farClip)
		: m_Camera(fov, aspectRatio, nearClip, farClip)
	{
	}

	void CameraController::OnUpdate(Timestep ts)
	{
		if (m_Camera.IsFlyMode())
			UpdateFly(ts);
		else
			UpdateOrbit(ts);
	}

	void CameraController::UpdateOrbit(Timestep ts)
	{
		// Orbit: Alt + 中键拖动平移，Alt + 左键拖动旋转，Alt + 右键拖动缩放
		if (Input::IsKeyPressed(Key::LeftAlt))
		{
			const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
			glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
			m_InitialMousePosition = mouse;

			if (Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
				m_Camera.OrbitPan(delta);
			else if (Input::IsMouseButtonPressed(Mouse::ButtonLeft))
				m_Camera.OrbitRotate(delta);
			else if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
				m_Camera.OrbitZoom(delta.y);
		}
	}

	void CameraController::UpdateFly(Timestep ts)
	{
		// Roam input is only consumed while the viewport is hovered/focused, so typing in a
		// panel (or using Ctrl+ shortcuts) never moves the camera. Last mouse is still tracked
		// every frame to avoid a look-jump when the pointer re-enters the viewport.
		const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
		if (m_Camera.GetViewportActive())
		{
			// Mouse-look: hold Right Mouse Button inside the viewport to turn the camera
			if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
			{
				glm::vec2 delta = (mouse - m_LastMousePosition) * 0.003f;
				m_Camera.FlyLook(delta);
			}

			// WASD move in the camera plane, Space/Q up/down, Shift sprints
			float speed = m_Camera.GetFlySpeed();
			if (Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift))
				speed *= 4.0f;

			const glm::vec3 forward = m_Camera.GetForwardDirection();
			const glm::vec3 right = m_Camera.GetRightDirection();
			glm::vec3 move{ 0.0f };
			if (Input::IsKeyPressed(Key::W)) move += forward;
			if (Input::IsKeyPressed(Key::S)) move -= forward;
			if (Input::IsKeyPressed(Key::A)) move -= right;
			if (Input::IsKeyPressed(Key::D)) move += right;
			if (Input::IsKeyPressed(Key::Space)) move += glm::vec3(0.0f, 1.0f, 0.0f);
			if (Input::IsKeyPressed(Key::Q)) move -= glm::vec3(0.0f, 1.0f, 0.0f);

			if (glm::length(move) > 0.0f)
				m_Camera.FlyMove(glm::normalize(move), speed, ts);
		}
		m_LastMousePosition = mouse;
	}

	void CameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(XL_BIND_EVENT_FN(CameraController::OnMouseScroll));
	}

	bool CameraController::OnMouseScroll(MouseScrolledEvent& e)
	{
		float delta = e.GetYOffset() * 0.1f;
		if (m_Camera.IsFlyMode())
		{
			// In roam mode the wheel glides along the view direction instead of zooming orbit distance
			m_Camera.FlyScroll(delta);
			return false;
		}
		m_Camera.OrbitZoom(delta);
		return false;
	}

	void CameraController::SetFlyMode(bool enabled)
	{
		const bool wasFly = m_Camera.IsFlyMode();
		m_Camera.SetFlyMode(enabled);
		// 进入漫游时记录当前鼠标位置，避免回归视口时视角发生跳变
		if (enabled && !wasFly)
			m_LastMousePosition = { Input::GetMouseX(), Input::GetMouseY() };
	}
}