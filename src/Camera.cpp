#include "Camera.hpp"

#include <iostream>

namespace Felina
{
	Camera::Camera(float width, float height, float nearPlane, float farPlane)
		: m_right(width), m_bottom(height), m_near(nearPlane), m_far(farPlane),
		m_projectionMatrix(1.0f), m_viewMatrix(1.0f)
	{
		ComputeProjectionMatrix();
		ComputeViewMatrix();
	}

	void Camera::UpdateProjectionMatrix(float newWidth, float newHeight)
	{
		m_right = newWidth;
		m_bottom = newHeight;
		ComputeProjectionMatrix();
	}

	void Camera::SetPosition(glm::vec3 newPosition)
	{
		m_transform.Translate(newPosition);
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

	void Camera::ComputeProjectionMatrix()
	{
		m_projectionMatrix = glm::perspective(glm::radians(45.0f), m_right / m_bottom, m_near, m_far);
		m_projectionMatrix[1][1] *= -1; // Flips Y-axis..
		ComputeInvViewProj();
	}

	void Camera::ComputeViewMatrix()
	{
		m_viewMatrix = glm::lookAt(m_transform.GetPosition(), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ComputeInvViewProj();
	}

	void Camera::ComputeInvViewProj()
	{
		// TODO: avoid recomputing the inverse twice when both view and projection change
		m_invViewProj = glm::inverse(m_projectionMatrix * m_viewMatrix);
	}
}