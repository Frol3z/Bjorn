#pragma once

#include "ResourceManager.hpp"
#include "Transform.hpp"

#include <string>

namespace Felina
{
	class Object
	{
		public:
			Object(const std::string& name, MeshID mesh, MaterialID material)
				: m_name(name), m_mesh(mesh), m_material(material)
			{}
			
			const std::string& GetName() const { return m_name; }
			MeshID GetMesh() const { return m_mesh; }
			MaterialID GetMaterial() const { return m_material; }
			glm::mat4 GetModelMatrix() const { return m_transform.GetMatrix(); }
			glm::mat3 GetNormalMatrix() const { return glm::transpose(glm::inverse(glm::mat3(m_transform.GetMatrix()))); }
			Transform& GetTransform() { return m_transform; }

		private:
			std::string m_name;
			MeshID m_mesh;
			MaterialID m_material;
			Transform m_transform;
	};
}