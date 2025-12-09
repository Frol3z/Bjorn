#pragma once

#include "Camera.hpp"
#include "Mesh.hpp"
#include "Object.hpp"

#include <unordered_map>
#include <memory>
#include <string>

namespace Felina 
{
	class Scene
	{
		public:
			Scene(float viewportWidth, float viewportHeight);
			~Scene();

			void RotateCamera(double azimuth, double elevation) { m_camera.Rotate(azimuth, elevation); }
			Camera& GetCamera() { return m_camera; }
			const Camera& GetCamera() const { return m_camera; }

			// TODO: update
			void AddObject(std::unique_ptr<Object> object);
			const std::vector<std::unique_ptr<Object>>& GetObjects() const { return m_objects; }

		private:
			Camera m_camera;
			std::vector<std::unique_ptr<Object>> m_objects; // Top-level objects
	};
}