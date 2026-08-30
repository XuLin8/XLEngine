#pragma once

#include "Camera.h"
#include "Runtime/Core/Timestep.h"

namespace XLEngine
{
	// 编辑器相机：只负责相机自身的状态与数学(视图/投影/轨道/漫游导航)，
	// 不读取任何输入。Orbit/Fly 的输入采集与事件分发由独立 CameraController 负责，
	// 从而保持功能层内部职责单一（相机不依赖 Input 单例）。
	class EditorCamera :public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

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
		[[nodiscard]] inline bool GetViewportActive() const { return m_ViewportActive; }

		[[nodiscard]] const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		[[nodiscard]] glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }

		[[nodiscard]] glm::vec3 GetUpDirection() const;
		[[nodiscard]] glm::vec3 GetRightDirection() const;
		[[nodiscard]] glm::vec3 GetForwardDirection() const;
		const glm::vec3& GetPosition() const { return m_Position; }
		[[nodiscard]] glm::quat GetOrientation() const;

		[[nodiscard]] float GetPitch() const { return m_Pitch; }
		[[nodiscard]] float GetYaw() const { return m_Yaw; }

		// ---- 由 CameraController 调用，仅驱动相机状态，不含输入采集 ----

		// 轨道模式：平移 / 旋转 / 缩放（围绕焦点）
		void OrbitPan(const glm::vec2& delta);
		void OrbitRotate(const glm::vec2& delta);
		void OrbitZoom(float delta);

		// 漫游模式：视角 / 位移 / 滚轮前移
		void FlyLook(const glm::vec2& delta);
		void FlyMove(const glm::vec3& moveDir, float speed, Timestep ts);
		void FlyScroll(float delta);
	private:
		void UpdateProjection();
		void UpdateView();

		glm::vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;
	private:
		float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

		glm::mat4 m_ViewMatrix;
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

		float m_Distance = 10.0f;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;

		// FPS free-roam state
		bool m_FlyMode = false;
		float m_FlySpeed = 25.0f;
		bool m_ViewportActive = true;
	};
}