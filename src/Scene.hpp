#pragma once

#include "Camera.hpp"
#include "Mesh.hpp"
#include "Object.hpp"

#include <unordered_map>

namespace Bjorn 
{
	class Scene
	{
	public:
		Scene(float viewportWidth, float viewportHeight);
		~Scene();

		Camera& GetCamera() { return m_camera; }
		const Camera& GetCamera() const { return m_camera; }

		void AddObject(std::unique_ptr<Object> object);
		const Object& GetObject(const std::string& name) const;
	private:
		Camera m_camera;
		std::unordered_map<std::string, std::unique_ptr<Object>> m_objects;
	};
}