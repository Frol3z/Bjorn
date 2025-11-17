#pragma once

#include "Mesh.hpp"

#include <unordered_map>
#include <iostream>

namespace Felina
{
	class Renderer;

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

			void LoadMesh();
			void UnloadAll();

			const Mesh& GetMesh(const std::string& name) const;

		private:
			ResourceManager(Renderer& renderer) : m_renderer(renderer) {} // Private constructor

			Renderer& m_renderer;
			std::unordered_map<std::string, std::unique_ptr<Mesh>> m_meshes;
	};
}