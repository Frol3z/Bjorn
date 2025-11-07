#pragma once

// NOTES
// This class should be a Singleton
// It will be responsible for loading and unloading resources
// The term resource comprehend meshes, materials, textures, etc...
// For each type there will be a list of the currently loaded resources

#include "Mesh.hpp"

#include <unordered_map>
#include <iostream>

namespace Bjorn
{
	class ResourceManager
	{
		public:
			ResourceManager(const ResourceManager&) = delete; // Delete copy constructor
			ResourceManager& operator=(const ResourceManager&) = delete; // Delete assignment operator
			
			static ResourceManager& GetInstance(Renderer& renderer)
			{
				static ResourceManager instance(renderer);
				return instance;
			}

			// Two possibilities
			// - vertex data and, optionally, index data may be passed
			// - filepath to be read may be passed

			// Mesh object will be created
			// Mesh will be explicitly loaded onto GPU memory (renderer.LoadMesh())
			// So the ResourceManager doesn't need to know about the allocator, just a reference to the Renderer to call the method
			// Once loaded the mesh object will be added to the unordered map
			void LoadMesh();
			void UnloadAll();

			const Mesh& GetMesh(const std::string& name) const;

		private:
			ResourceManager(Renderer& renderer) : m_renderer(renderer) {} // Private constructor

			Renderer& m_renderer;
			std::unordered_map<std::string, std::unique_ptr<Mesh>> m_meshes;
	};
}