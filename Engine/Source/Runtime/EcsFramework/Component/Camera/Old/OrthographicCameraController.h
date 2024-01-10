#pragma once

#include "Runtime/Core/Timestep.h"
#include "OrthographicCamera.h"
#include "Runtime/Events/MouseEvent.h"
#include "Runtime/Events/ApplicationEvent.h"

namespace XLEngine
{
	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);
		void OnResize(float width, float height);

		[[nodiscard]] OrthographicCamera& GetCamera() { return m_Camera; }
		[[nodiscard]] const OrthographicCamera& GetCamera() const { return m_Camera; }

		[[nodiscard]] float GetZoomLevel() const { return m_ZoomLevel; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; }

	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);

	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;//must define before m_Camera,m_Camera needs it to initialize
		
		bool m_Rotation = false;
		
		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f;
		float m_CameraTranslationSpeed = 5.0f, m_CameraRotationSpeed = 180.0f;

		OrthographicCamera m_Camera;
	};
}

