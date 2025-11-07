#include "Scene.hpp"

namespace Bjorn
{
	Scene::Scene(float viewportWidth, float viewportHeight)
		: m_camera(viewportWidth, viewportHeight)
	{
		m_objects.clear();
	}

	Scene::~Scene()
	{
		m_objects.clear();
	}

	void Scene::AddObject(std::unique_ptr<Object> object)
	{
		// Insert the objects in the scene and move its ownership
		m_objects.emplace(object->GetName(), std::move(object));
	}

	const Object& Scene::GetObject(const std::string& name) const
	{
		auto it = m_objects.find(name);
		if (it == m_objects.end())
		{
			throw std::runtime_error("Object not found: " + name);
		}
		return *(it->second);
	}
};