#include "ResourceManager.hpp"

namespace Bjorn
{
	void ResourceManager::LoadMesh()
	{
		// Instance the mesh
		auto mesh = std::make_unique<Mesh>();
		// Request the renderer to load mesh data on the GPU
		m_renderer.LoadMesh(*mesh);
		// Store (and move ownership) of the mesh into the map
		m_meshes.emplace("QuadMesh", std::move(mesh));
	}

	void ResourceManager::UnloadAll()
	{
		m_meshes.clear();
	}

	const Mesh& ResourceManager::GetMesh(const std::string& name) const
	{
		auto it = m_meshes.find(name);
		if (it == m_meshes.end())
		{
			throw std::runtime_error("Mesh not found: " + name);
		}
		return *(it->second);
	}
}