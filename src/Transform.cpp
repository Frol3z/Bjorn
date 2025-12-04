#include "Transform.hpp"

#include <iostream>

namespace Felina
{
	Transform::Transform()
		: m_position(0.0f), m_rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)), m_scale(1.0f), m_transform(1.0f)
	{ }

	void Transform::Translate(const glm::vec3& translation)
	{
		m_position += translation;
		UpdateMatrix();
	}

	void Transform::Rotate(const float angle, const glm::vec3& axis)
	{
		m_rotation = glm::rotate(m_rotation, angle, axis);
		UpdateMatrix();
	}

	void Transform::Scale(const glm::vec3& scale)
	{
		m_scale *= scale;
		UpdateMatrix();
	}

	void Transform::SetPosition(const glm::vec3& position)
	{
		m_position = position;
		UpdateMatrix();
	}

	void Transform::SetRotation(const glm::quat& rotation)
	{
		m_rotation = rotation;
		UpdateMatrix();
	}

	void Transform::SetScale(const glm::vec3& scale)
	{
		m_scale = scale;
		UpdateMatrix();
	}

	void Transform::UpdateMatrix()
	{
		m_transform = glm::translate(glm::mat4(1.0f), m_position) * glm::mat4_cast(m_rotation) * glm::scale(glm::mat4(1.0f), m_scale);
	}
}