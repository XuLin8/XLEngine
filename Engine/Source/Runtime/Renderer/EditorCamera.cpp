#include "xlpch.h"
#include "EditorCamera.h"

#include "Runtime/Input/Input.h"
#include "Runtime/Input/KeyCodes.h"
#include "Runtime/Input/MouseCodes.h"

#include <glfw/glfw3.h>
#include <glm/gtx/quaternion.hpp>

namespace XLEngine
{
	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
		: m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip), Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip))
	{
		UpdateView();
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorCamera::UpdateView()
	{
		// m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
		if (!m_FlyMode)
			m_Position = CalculatePosition();

		glm::quat orientation = GetOrientation();
		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

	void EditorCamera::OnUpdate(Timestep ts)
	{
		if (m_FlyMode)
		{
			FlyUpdate(ts);
			UpdateView();
			return;
		}

		if (Input::IsKeyPressed(Key::LeftAlt))
		{
			const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
			glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
			m_InitialMousePosition = mouse;

			if (Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
				MousePan(delta);
			else if (Input::IsMouseButtonPressed(Mouse::ButtonLeft))
				MouseRotate(delta);
			else if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
				MouseZoom(delta.y);
		}

		UpdateView();
	}

	void EditorCamera::SetFlyMode(bool enabled)
	{
		if (m_FlyMode == enabled)
			return;

		m_FlyMode = enabled;
		if (m_FlyMode)
		{
			// Enter roam: take over the current orbit viewport and start walking from it
			m_Position = CalculatePosition();
			m_LastMousePosition = { Input::GetMouseX(), Input::GetMouseY() };
		}
		else
		{
			// Exit roam: keep the roaming viewpoint and re-derive an orbit focal point from it
			m_FocalPoint = m_Position + GetForwardDirection() * m_Distance;
		}
		UpdateView();
	}

	void EditorCamera::FlyUpdate(Timestep ts)
	{
		// Roam input is only consumed while the viewport is hovered/focused, so typing in a
		// panel (or using Ctrl+ shortcuts) never moves the camera. Last mouse is still tracked
		// every frame to avoid a look-jump when the pointer re-enters the viewport.
		const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
		if (m_ViewportActive)
		{
			// Mouse-look: hold Right Mouse Button inside the viewport to turn the camera
			if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
			{
				glm::vec2 delta = (mouse - m_LastMousePosition) * 0.003f;
				m_Yaw -= delta.x * RotationSpeed();
				m_Pitch -= delta.y * RotationSpeed();
				m_Pitch = std::clamp(m_Pitch, -89.0f, 89.0f);
			}

			// WASD move in the camera plane, Space/Q up/down, Shift sprints
			float speed = m_FlySpeed;
			if (Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift))
				speed *= 4.0f;

			const glm::vec3 forward = GetForwardDirection();
			const glm::vec3 right = GetRightDirection();
			glm::vec3 move{ 0.0f };
			if (Input::IsKeyPressed(Key::W)) move += forward;
			if (Input::IsKeyPressed(Key::S)) move -= forward;
			if (Input::IsKeyPressed(Key::A)) move -= right;
			if (Input::IsKeyPressed(Key::D)) move += right;
			if (Input::IsKeyPressed(Key::Space)) move += glm::vec3(0.0f, 1.0f, 0.0f);
			if (Input::IsKeyPressed(Key::Q)) move -= glm::vec3(0.0f, 1.0f, 0.0f);

			if (glm::length(move) > 0.0f)
				m_Position += glm::normalize(move) * (speed * ts);
		}
		m_LastMousePosition = mouse;
	}

	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(XL_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		float delta = e.GetYOffset() * 0.1f;
		if (m_FlyMode)
		{
			// In roam mode the wheel glides along the view direction instead of zooming orbit distance
			m_Position += GetForwardDirection() * delta * m_FlySpeed * 0.1f;
			UpdateView();
			return false;
		}
		MouseZoom(delta);
		UpdateView();
		return false;
	}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const glm::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
	}

	glm::vec3 EditorCamera::GetUpDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetRightDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetForwardDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}
}