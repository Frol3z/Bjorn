#pragma once

#include <glm/glm.hpp>

namespace Felina
{
	class Material
	{
		public:
			Material(glm::vec3 albedo, glm::vec3 specular, glm::vec4 coefficients);

			const glm::vec3 GetAlbedo() const { return m_albedo; }
			const glm::vec3 GetSpecular() const { return m_specular; }
			const glm::vec4 GetCoefficients() const { return m_coefficients; }

		private:
			glm::vec3 m_albedo;
			glm::vec3 m_specular;
			glm::vec4 m_coefficients; // R:kI, G:kD, B:kS, A:shininess
	};
}