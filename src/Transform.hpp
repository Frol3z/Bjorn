#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Felina
{
	class Transform
	{
	public:
		Transform();

		void Translate(const glm::vec3& translation);
		void Rotate(const float angle, const glm::vec3& axis);
		void Scale(const glm::vec3& scale);

		const glm::vec3& GetPosition() const { return m_position; }
		const glm::quat& GetRotation() const { return m_rotation; }
		const glm::vec3& GetScale() const { return m_scale; }
		const glm::mat4& GetMatrix() const { return m_transform; };

	private:
		void UpdateMatrix();

		glm::vec3 m_position;
		glm::quat m_rotation;
		glm::vec3 m_scale;

		glm::mat4 m_transform;
	};
}