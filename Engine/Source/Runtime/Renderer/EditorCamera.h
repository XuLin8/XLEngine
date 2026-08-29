#pragma once

#include "Camera.h"
#include "Runtime/Core/Timestep.h"
#include "Runtime/Events/Event.h"
#include "Runtime/Events/MouseEvent.h"

namespace XLEngine
{
	class EditorCamera :public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		[[nodiscard]] inline float GetDistance() const { return m_Distance; }
		inline void SetDistance(float distance) { m_Distance = distance; }

		inline void SetPitch(float pitch) { m_Pitch = pitch; }
		inline void SetYaw(float yaw) { m_Yaw = yaw; }

		inline void SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

		// FPS free-roam mode (WASD + RMB look), linked with the gizmo for object manipulation
		void SetFlyMode(bool enabled);
		[[nodiscard]] bool IsFlyMode() const { return m_FlyMode; }
		inline void SetFlySpeed(float speed) { m_FlySpeed = speed; }
		[[nodiscard]] inline float GetFlySpeed() const { return m_FlySpeed; }
		// Roam input is only consumed while the viewport is hovered/focused
		inline void SetViewportActive(bool active) { m_ViewportActive = active; }

		[[nodiscard]] const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		[[nodiscard]] glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }

		[[nodiscard]] glm::vec3 GetUpDirection() const;
		[[nodiscard]] glm::vec3 GetRightDirection() const;
		[[nodiscard]] glm::vec3 GetForwardDirection() const;
		const glm::vec3& GetPosition() const { return m_Position; }
		[[nodiscard]] glm::quat GetOrientation() const;

		[[nodiscard]] float GetPitch() const { return m_Pitch; }
		[[nodiscard]] float GetYaw() const { return m_Yaw; }
	private:
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		void FlyUpdate(Timestep ts);

		glm::vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;
	private:
		float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

		glm::mat4 m_ViewMatrix;
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

		glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };

		float m_Distance = 10.0f;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;

		// FPS free-roam state
		bool m_FlyMode = false;
		float m_FlySpeed = 25.0f;
		bool m_ViewportActive = true;
		glm::vec2 m_LastMousePosition = { 0.0f, 0.0f };

	};
}