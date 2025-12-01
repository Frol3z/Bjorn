#pragma once

#include "Transform.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace Felina
{
	class Camera
	{
		public:
			Camera(float width, float height, float nearPlane = 0.1f, float farPlane = 1000.0f);
		
			void UpdateProjectionMatrix(float newWidth, float newHeight);
			void SetPosition(glm::vec3 newPosition);
			glm::vec3 GetPosition() const { return m_transform.GetPosition(); }
			glm::mat4 GetProjectionMatrix() const;
			glm::mat4 GetViewMatrix() const;
			glm::mat4 GetInvViewProj() const;
	
		private:
			void ComputeProjectionMatrix();
			void ComputeViewMatrix();
			void ComputeInvViewProj();

			glm::mat4 m_projectionMatrix;
			glm::mat4 m_viewMatrix;
			glm::mat4 m_invViewProj;

			Transform m_transform;

			float m_left = 0.0f;
			float m_right = 0.0f;
			float m_bottom = 0.0f;
			float m_top = 0.0f;
			float m_near = 0.0f;
			float m_far = 0.0f;
	};
}