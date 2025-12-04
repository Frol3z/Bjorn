#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

namespace Felina
{
	Camera::Camera(float width, float height, float nearPlane, float farPlane)
		: m_right(width), m_bottom(height), m_near(nearPlane), m_far(farPlane),
		m_projectionMatrix(1.0f), m_viewMatrix(1.0f),
		m_position(WORLD_ORIGIN), m_localUp(WORLD_UP), m_target(WORLD_ORIGIN)
	{
		ComputeLocalCoordinateSystem();
		ComputeViewMatrix();
		ComputeProjectionMatrix();
	}

	void Camera::Rotate(double azimuth, double elevation)
	{		
		glm::mat4 horizontalRotation = glm::rotate(glm::mat4(1.0f), static_cast<float>(glm::radians(azimuth)), m_localUp);
		glm::mat4 totalRotation = glm::rotate(horizontalRotation, static_cast<float>(glm::radians(elevation)), m_localRight);
		m_position = totalRotation * glm::vec4(m_position, 1.0f);

		ComputeLocalCoordinateSystem();
		ComputeViewMatrix();
	}

	void Camera::UpdateProjectionMatrix(float newWidth, float newHeight)
	{
		m_right = newWidth;
		m_bottom = newHeight;
		ComputeProjectionMatrix();
	}

	void Camera::SetPosition(glm::vec3 position)
	{
		m_position = position;
		ComputeLocalCoordinateSystem();
		ComputeViewMatrix();
	}

	glm::mat4 Camera::GetProjectionMatrix() const
	{
		return m_projectionMatrix;
	}

	glm::mat4 Camera::GetViewMatrix() const
	{
		// Camera position, center of projection, up axis (Z-axis)
		return m_viewMatrix;
	}

	glm::mat4 Camera::GetInvViewProj() const
	{
		return m_invViewProj;
	}

	void Camera::ComputeLocalCoordinateSystem()
	{
		m_localForward = glm::normalize(m_target - m_position);
		if (m_localForward == WORLD_ORIGIN) 
		{
			// It may happen that if the camera is positioned at the
			// world origin, then the resulting forward vector would be
			// the null vector.
			// If that's the case, the world +Y is assumed
			// as the forward direction.
			m_localForward = WORLD_FORWARD;
		}
		m_localRight = glm::normalize(glm::cross(m_localForward, WORLD_UP));
		m_localUp = glm::normalize(glm::cross(m_localRight, m_localForward));
	}

	void Camera::ComputeViewMatrix()
	{
		m_viewMatrix = glm::lookAt(m_position, glm::vec3(0.0f), m_localUp);
		ComputeInvViewProj();
	}

	void Camera::ComputeProjectionMatrix()
	{
		m_projectionMatrix = glm::perspective(glm::radians(45.0f), m_right / m_bottom, m_near, m_far);
		m_projectionMatrix[1][1] *= -1; // Flips Y-axis..
		ComputeInvViewProj();
	}

	void Camera::ComputeInvViewProj()
	{
		// TODO: avoid recomputing the inverse twice when both view and projection change
		m_invViewProj = glm::inverse(m_projectionMatrix * m_viewMatrix);
	}
}