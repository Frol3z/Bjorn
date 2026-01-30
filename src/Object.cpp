#include "Object.hpp"

namespace Felina
{
	void Object::AddChild(std::unique_ptr<Object> child)
	{
		m_children.push_back(std::move(child));
	}
}