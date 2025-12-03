#pragma once

#include <glm/glm.hpp>

namespace Felina
{
	class Material
	{
		public:
			Material(glm::vec3 albedo, glm::vec4 specular);

			const glm::vec3 GetAlbedo() const { return m_albedo; }
			const glm::vec4 GetSpecular() const { return m_specular; }

		private:
			glm::vec3 m_albedo;
			glm::vec4 m_specular;
	};
}