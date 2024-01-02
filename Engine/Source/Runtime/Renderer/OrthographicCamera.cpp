#include "xlpch.h"

#include "OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace XLEngine
{
	OrthographicCamera::OrthographicCamera(float l, float r, float b, float t)
		:m_ProjectionMatrix(glm::ortho(l, r, b, t, -1.0f, 1.0f)), m_ViewMatrix(1.0f) 
	{
		XL_PROFILE_FUNCTION();

		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::SetProjection(float l, float r, float b, float t)
	{
		XL_PROFILE_FUNCTION();

		m_ProjectionMatrix = glm::ortho(l, r, b, t, -1.0f, 1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::RecalculateViewMatrix()
	{
		XL_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}