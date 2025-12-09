#pragma once

#include "ResourceManager.hpp"
#include "Transform.hpp"

#include <string>

namespace Felina
{
	class Object
	{
		// TODO:
		// Figure out a method to cache the matrices without the need to recompute
		// them each frame, especially the normal matrix which requires an inverse computation

		public:
			Object(const std::string& name, MeshID mesh, MaterialID material, Object* parent = nullptr)
				: m_name(name), m_mesh(mesh), m_material(material), m_parent(parent)
			{}
			
			// Resources
			void SetMaterial(MaterialID id) { m_material = id; }
			void SetMesh(MeshID id) { m_mesh = id; }

			// Children
			void AddChild(std::unique_ptr<Object> child);
			const std::vector<std::unique_ptr<Object>>& GetChildren() const { return m_children; }

			// Getters
			const std::string& GetName() const { return m_name; }
			MeshID GetMesh() const { return m_mesh; }
			MaterialID GetMaterial() const { return m_material; }
			glm::mat4 GetModelMatrix() const;
			glm::mat3 GetNormalMatrix() const { return glm::transpose(glm::inverse(glm::mat3(GetModelMatrix()))); }

			// Transform
			void Translate(const glm::vec3& translation) { m_transform.Translate(translation);}
			void Rotate(const float angle, const glm::vec3& axis) { m_transform.Rotate(angle, axis); }
			void Scale(const glm::vec3& scale) { m_transform.Scale(scale); }
			void SetPosition(const glm::vec3& position) { m_transform.SetPosition(position); }
			void SetRotation(const glm::quat& rotation) { m_transform.SetRotation(rotation); }
			void SetScale(const glm::vec3& scale) { m_transform.SetScale(scale); }
			void SetModelMatrix(const glm::mat4& matrix) { m_transform.SetMatrix(matrix); }
			const glm::vec3& GetPosition() const { return m_transform.GetPosition(); }
			const glm::quat& GetRotation() const { return m_transform.GetRotation(); }
			const glm::vec3& GetScale() const { return m_transform.GetScale(); }

		private:
			std::string m_name;

			MeshID m_mesh;
			MaterialID m_material;
			Transform m_transform;

			Object* m_parent;
			std::vector<std::unique_ptr<Object>> m_children;
	};
}