#pragma once

#include "Mesh.hpp"
#include "Material.hpp"

#include <unordered_map>
#include <iostream>

namespace Felina
{
	using MeshID = uint32_t;
	using MaterialID = uint32_t;

	class Renderer;

	class ResourceManager
	{
		public:
			template<typename T>
			struct Resource {
				std::string name;
				std::unique_ptr<T> resource;
			};

		public:
			ResourceManager(const ResourceManager&) = delete; // Delete copy constructor
			ResourceManager& operator=(const ResourceManager&) = delete; // Delete assignment operator
			
			static ResourceManager& GetInstance()
			{
				static ResourceManager instance;
				return instance;
			}

			MeshID LoadMesh(std::unique_ptr<Mesh> mesh, const std::string& name, Renderer& renderer);
			MaterialID LoadMaterial(std::unique_ptr<Material> material, const std::string& name);
			void UnloadAll();

			const Mesh& GetMesh(MeshID id) const;
			const std::string& GetMeshName(MeshID id) const;
			const std::unordered_map<MeshID, Resource<Mesh>>& GetMeshes() const { return m_meshes; }
			const Material& GetMaterial(MaterialID id) const;
			const std::string& GetMaterialName(MaterialID id) const;
			const std::unordered_map<MaterialID, Resource<Material>>& GetMaterials() const { return m_materials; }
		private:
			ResourceManager(){} // Private constructor

			std::unordered_map<MeshID, Resource<Mesh>> m_meshes;
			std::unordered_map<MaterialID, Resource<Material>> m_materials;
			MeshID m_meshID{ 0 };
			MaterialID m_materialID{ 0 };
	};
}