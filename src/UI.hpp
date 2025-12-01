#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Felina
{
	class Object;
	class Scene;

	class UI
	{
		public:
			UI();
			void Update(Scene& scene);

		private:
			void DrawHierarchy(Scene& scene);
			void DrawInspector();

			Object* m_hierarchySelection = nullptr;
			glm::vec3 m_displayedPosition;
			glm::vec3 m_displayedRotation;
			glm::vec3 m_displayedScale;
	};
}