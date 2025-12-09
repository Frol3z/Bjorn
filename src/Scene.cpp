#include "Scene.hpp"

namespace Felina
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
		m_objects.push_back(std::move(object));
	}
};