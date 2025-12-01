#include "ResourceManager.hpp"

#include "Renderer.hpp"

namespace Felina
{
	MeshID ResourceManager::LoadMesh(std::unique_ptr<Mesh> mesh, const std::string& name, Renderer& renderer)
	{
		// Request the renderer to load mesh data on the GPU
		renderer.LoadMesh(*mesh);

		// Store (and move ownership) of the mesh into the map
		auto ID = m_meshID++;
		m_meshes.emplace(ID, Resource<Mesh>{ name, std::move(mesh) });
		return ID;
	}

	MaterialID ResourceManager::LoadMaterial(std::unique_ptr<Material> material, const std::string& name)
	{
		auto ID = m_materialID++;
		m_materials.emplace(ID, Resource<Material>{ name, std::move(material) });
		return ID;
	}

	void ResourceManager::UnloadAll()
	{
		m_meshes.clear();
		m_materials.clear();
	}

	const Mesh& ResourceManager::GetMesh(MeshID id) const
	{
		auto it = m_meshes.find(id);
		if (it == m_meshes.end())
			throw std::runtime_error("[RESOURCE MANAGER] Mesh with ID " + std::to_string(id) + "not found!");
		return *(it->second.resource);
	}

	const std::string& ResourceManager::GetMeshName(MeshID id) const
	{
		auto it = m_meshes.find(id);
		if(it == m_meshes.end())
			throw std::runtime_error("[RESOURCE MANAGER] Mesh with ID " + std::to_string(id) + "not found!");
		return (it->second.name);
	}

	const Material& ResourceManager::GetMaterial(MaterialID id) const
	{
		auto it = m_materials.find(id);
		if (it == m_materials.end())
			throw std::runtime_error("[RESOURCE MANAGER] Material with ID " + std::to_string(id) + "not found!");
		return *(it->second.resource);
	}

	const std::string& ResourceManager::GetMaterialName(MaterialID id) const
	{
		auto it = m_materials.find(id);
		if (it == m_materials.end())
			throw std::runtime_error("[RESOURCE MANAGER] Material with ID " + std::to_string(id) + "not found!");
		return (it->second.name);
	}
}