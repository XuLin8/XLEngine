#pragma once

#include "XLEngine/Renderer/Camera.h"

namespace XLEngine
{
	class SceneCamera :public Camera
	{
	public:
		enum class ProjectionType { 
			Perspective = 0, 
			Orthographic = 1
		};

	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		void SetViewportSize(uint32_t width, uint32_t height);

		void SetOrthographic(float size, float nearClip, float farClip);
		void SetOrthographicNearClip(float nearClip) { m_OrthographicNear = nearClip; RecalculateProjection(); }
		void SetOrthographicFarClip(float farClip) { m_OrthographicFar = farClip; RecalculateProjection(); }
		float GetOrthographicNearClip() const { return m_OrthographicNear; }
		float GetOrthographicFarClip() const { return m_OrthographicFar; }
		
		void SetPerspective(float verticalFOV, float nearClip, float farClip);
		void SetPerspectiveVerticalFOV(float verticalFOV) { m_PerspectiveFOV = verticalFOV; RecalculateProjection(); }
		void SetPerspectiveNearClip(float nearClip) { m_PerspectiveNear = nearClip; RecalculateProjection(); }
		void SetPerspectiveFarClip(float farClip) { m_PerspectiveFar = farClip; RecalculateProjection(); }
		float GetPerspectiveVerticalFOV() const { return m_PerspectiveFOV; }
		float GetPerspectiveNearClip() const { return m_PerspectiveNear; }
		float GetPerspectiveFarClip() const { return m_PerspectiveFar; }

		void SetOrthographicSize(float size) { m_OrthographicSize = size; RecalculateProjection(); }
		float GetOrthographicSize() const { return m_OrthographicSize; }

		void SetProjectionType(ProjectionType type) { m_ProjectionType = type; }
		ProjectionType GetProjectionType() const { return m_ProjectionType; }

	private:
		void RecalculateProjection();
	private:
		ProjectionType m_ProjectionType = ProjectionType::Orthographic;

		float m_PerspectiveFOV = glm::radians(45.0f);
		float m_PerspectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;

		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;

		float m_AspectRatio = 0.0f;
	};
}