#pragma once

#include "Common.hpp"

#include <glm/glm.hpp>

namespace Felina
{
	class Material
	{
		public:
			Material(glm::vec3 albedo, glm::vec3 specular, glm::vec4 coefficients, TextureID albedoTex = -1, TextureID specularTex = -1);

			const glm::vec3 GetAlbedo() const { return m_albedo; }
			const glm::vec3 GetSpecular() const { return m_specular; }
			const glm::vec4 GetCoefficients() const { return m_coefficients; }
			const TextureID GetAlbedoTexture() const { return m_albedoTex; }
			const TextureID GetSpecularTexture() const { return m_specularTex; }

		private:
			glm::vec3 m_albedo;
			glm::vec3 m_specular;
			glm::vec4 m_coefficients; // R:kA, G:kD, B:kS, A:shininess
			
			// Invalid if equal to -1 unsigned int (424967...)
			TextureID m_albedoTex;
			TextureID m_specularTex;
	};
}