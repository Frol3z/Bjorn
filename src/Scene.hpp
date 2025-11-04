#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>

#include "Camera.hpp"
#include "Mesh.hpp"

#include <vector>

// TODO
// The Scene should expose to the user/application a functionality for doing something like scene.AddObject(object)
// where object is an instance of class Object and will reference globally loaded resources like meshes, materials, textures, etc...
// ...
// 
// 
// TODO
// Create an AssetManager/ResourceManager which will handle loading and unloading of meshes (and then material and others...)
// This class will hold a list of objects that will reference those resources
//

namespace Bjorn 
{
	class Scene
	{
	public:
		Scene(float viewportWidth, float viewportHeight);

		Camera& GetCamera() { return m_camera; }
		const Camera& GetCamera() const { return m_camera; }
	private:
		Camera m_camera;
	};
}