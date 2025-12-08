#include "Object.hpp"

namespace Felina
{
	void Object::AddChild(std::unique_ptr<Object> child)
	{
		m_children.push_back(std::move(child));
	}

	glm::mat4 Object::GetModelMatrix() const
	{
		if (m_parent)
			return m_parent->GetModelMatrix() * m_transform.GetMatrix();
		else
			return m_transform.GetMatrix();
	}
}