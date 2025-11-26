#pragma once

#include "Mesh.hpp"
#include "Transform.hpp"

#include <string>

namespace Felina
{
	class Object
	{
		public:
			Object(const std::string& name, const Mesh& mesh)
				: m_name(name), m_mesh(mesh)
			{}
			
			const std::string& GetName() const { return m_name; }
			const Mesh& GetMesh() const { return m_mesh; }
			glm::mat4 GetModelMatrix() const { return m_transform.GetMatrix(); }
			glm::mat3 GetNormalMatrix() const { return glm::transpose(glm::inverse(glm::mat3(m_transform.GetMatrix()))); }
			Transform& GetTransform() { return m_transform; }

		private:
			std::string m_name;
			const Mesh& m_mesh;
			Transform m_transform;
	};
}